using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace IntelBurnTestUsb
{
    // Separate QA executable. These tests never open physical disks or write to removable media.
    internal static class Checks
    {
        static int passed;
        static void Assert(bool condition, string name)
        { if (!condition) throw new Exception("FAILED: " + name); passed++; }
        static void Refuses(Action action, string name)
        { bool refused = false; try { action(); } catch (IOException) { refused = true; } Assert(refused, name); }
        internal static UsbDisk Example(string root)
        {
            UsbDisk disk = new UsbDisk { Number = 5, SectorSize = 512, Size = 64UL * 1024 * 1024, BusType = 7,
                UniqueId = "QA-ID", DevicePath = "QA-ONLY-NOT-A-DEVICE", Serial = "QA-USB", HardwareSerial = "QA-USB",
                Model = "Example USB", IsBoot = false, IsSystem = false, IsReadOnly = false, IsOffline = false, PartitionCount = 1 };
            disk.Volumes.Add(new UsbVolume { Root = root, Letter = "E:", Label = "MY FILES", FileSystem = "FAT32",
                Serial = 42, FreeBytes = 60UL * 1024 * 1024, IsBoot = false, IsSystem = false, IsReadOnly = false });
            return disk;
        }
        sealed class Probe : IUsbProbe
        {
            internal UsbDisk Disk; internal int Calls; internal Action<int> Before;
            public List<UsbDisk> List() { return Disk == null ? new List<UsbDisk>() : new List<UsbDisk> { Disk }; }
            public UsbDisk Refresh(uint n) { Calls++; if (Before != null) Before(Calls); return Disk != null && Disk.Number == n ? Disk : null; }
        }
        sealed class FileBlock : IBlockDevice
        {
            readonly FileStream stream; internal bool CorruptReads;
            public ulong Length { get { return (ulong)stream.Length; } }
            internal FileBlock(string path, long bytes)
            { stream = new FileStream(path, FileMode.CreateNew, FileAccess.ReadWrite, FileShare.Read); stream.SetLength(bytes); }
            public void Write(ulong offset, byte[] bytes) { stream.Position = checked((long)offset); stream.Write(bytes, 0, bytes.Length); }
            public byte[] Read(ulong offset, int size)
            {
                byte[] bytes = new byte[size]; stream.Position = checked((long)offset); int at = 0;
                while (at < size) { int n = stream.Read(bytes, at, size - at); if (n == 0) throw new IOException("Short file read"); at += n; }
                if (CorruptReads && size > 0) bytes[0] ^= 1; return bytes;
            }
            public void Flush() { stream.Flush(true); }
            public void Dispose() { stream.Dispose(); }
        }
        static void Guards(string directory)
        {
            Assert(!Safety.Eligible(null), "missing disk");
            foreach (Action<UsbDisk> alter in new Action<UsbDisk>[] {
                d => d.BusType = 11, d => d.IsBoot = true, d => d.IsSystem = true, d => d.IsReadOnly = true,
                d => d.IsOffline = true, d => d.IsBoot = null, d => d.IsSystem = null, d => d.IsReadOnly = null,
                d => d.IsOffline = null, d => d.HasProtectedPartition = true, d => d.DevicePath = "", d => d.Size = 1024 })
            { UsbDisk d = Example(directory); alter(d); Assert(!Safety.Eligible(d), "unsafe disks excluded"); }
            Assert(Safety.Eligible(Example(directory)), "USB allowed");
            foreach (string text in new[] { null, "", "erase", "YES", "ERASE USB", "ERAS" }) Assert(!Safety.EraseConsent(text), "explicit erase phrase");
            Assert(Safety.EraseConsent("ERASE"), "exact erase phrase");
            UsbDisk disk = Example(directory); UsbVolume volume = disk.Volumes[0];
            InstallPlan plan = new InstallPlan(disk, volume, false);
            Probe probe = new Probe { Disk = Example(directory) };
            plan.Revalidate(probe); Assert(true, "unchanged USB allowed");
            probe.Disk.Serial = "REPLACEMENT"; Refuses(() => plan.Revalidate(probe), "disk replacement refused");
            probe.Disk = Example(directory); probe.Disk.Volumes[0].Serial++; Refuses(() => plan.Revalidate(probe), "volume replacement refused");
            probe.Disk = Example(directory); probe.Disk.Volumes[0].Letter = "F:"; Refuses(() => plan.Revalidate(probe), "reassigned drive letter refused");
            probe.Disk = Example(directory); probe.Disk.IsSystem = true; Refuses(() => plan.Revalidate(probe), "new system drive refused");
            probe.Disk = null; Refuses(() => plan.Revalidate(probe), "unplugged disk refused");
            volume.FileSystem = "NTFS"; Refuses(() => new InstallPlan(disk, volume, false), "non-FAT32 copy refused");
            volume.FileSystem = "FAT32"; volume.IsReadOnly = null; Refuses(() => new InstallPlan(disk, volume, false), "unknown volume readonly state refused");
            volume.IsReadOnly = false; UsbVolume foreign = Example(directory + "-other").Volumes[0];
            Refuses(() => new InstallPlan(disk, foreign, false), "foreign volume refused");
            foreach (ulong size in new[] { 64UL << 20, 128UL << 20, 256UL << 20, 512UL << 20, 4UL << 30,
                8UL << 30, (8UL << 30) + 512, 16UL << 30, 32UL << 30, 128UL << 30, 512UL << 30, (2UL << 40) - 512 })
            {
                Fat32Layout layout = Fat32Layout.Create(size, 512);
                Assert(layout.Clusters >= 65525 && layout.Clusters < 0x0ffffff5, "FAT32 cluster count");
                Assert((layout.Clusters + 2UL) * 4 <= layout.FatSectors * 512UL, "FAT fits every cluster");
                Assert((layout.Sectors + (ulong)Fat32Layout.Start) * 512 == size, "USB full capacity retained");
                Assert(layout.DataOffset + layout.Clusters * layout.ClusterBytes <= size, "cluster bounds");
            }
            Refuses(() => Fat32Layout.Create(32UL << 20, 512), "small USB refused");
            Refuses(() => Fat32Layout.Create(64UL << 20, 4096), "unsupported logical sectors refused");
            Refuses(() => Fat32Layout.Create(3UL << 40, 512), "oversized format refused");
            Refuses(() => Fat32Layout.Create((64UL << 20) + 1, 512), "unaligned disk refused");
        }
        static void Copy(string output, byte[] efi, byte[] readme)
        {
            string root = Path.Combine(output, "copy-volume"); Directory.CreateDirectory(root);
            UsbDisk disk = Example(root); Probe probe = new Probe { Disk = disk };
            InstallPlan plan = new InstallPlan(disk, disk.Volumes[0], false);
            string keep = Path.Combine(root, "photo.txt"); File.WriteAllText(keep, "KEEP MY FILES");
            string boot = Path.Combine(root, "EFI", "BOOT"); Directory.CreateDirectory(boot);
            string target = Path.Combine(boot, "BOOTX64.EFI"); byte[] original = new byte[] { 1, 2, 3, 4, 5 };
            File.WriteAllBytes(target, original);
            string backup = FileInstall.Run(plan, probe, efi, readme, (p, s) => { });
            Assert(Safety.SameFile(target, efi), "copied payload verified");
            Assert(Safety.SameFile(backup, original), "original boot file backed up");
            Assert(File.ReadAllText(keep) == "KEEP MY FILES", "existing user file preserved");
            Assert(!Directory.GetFiles(boot, "*.tmp").Any(), "staged files cleaned up");
            Assert(FileInstall.Run(plan, probe, efi, readme, (p, s) => { }) == null, "identical rerun does not create backup");
            // Fail after a boot replacement to verify rollback.
            File.WriteAllBytes(target, original); probe.Calls = 0;
            probe.Before = n => { if (n == 5) throw new IOException("Injected probe failure after copying"); };
            Refuses(() => FileInstall.Run(plan, probe, efi, readme, (p, s) => { }), "post-copy failure reported");
            Assert(Safety.SameFile(target, original), "failed copy restores original boot file");
            probe.Before = null; probe.Calls = 0;
            probe.Before = n => { if (n == 1) disk.Serial = "SWAPPED"; };
            Refuses(() => FileInstall.Run(plan, probe, efi, readme, (p, s) => { }), "copy on swapped USB blocked");
            Assert(Safety.SameFile(target, original) && File.ReadAllText(keep) == "KEEP MY FILES", "swapped USB received no writes");
        }
        static void Screenshot(Form form, string path)
        {
            using (form)
            {
                // Offscreen native render. No visible application window or physical device interaction.
                form.Location = new Point(-20000, -20000); form.StartPosition = FormStartPosition.Manual;
                form.ShowInTaskbar = false; form.Opacity = 0;
                form.Show(); Application.DoEvents(); form.PerformLayout();
                using (Bitmap bitmap = new Bitmap(form.Width, form.Height))
                { form.DrawToBitmap(bitmap, new Rectangle(Point.Empty, bitmap.Size)); bitmap.Save(path); }
                form.Close();
            }
        }
        [STAThread]
        static int Main(string[] args)
        {
            try
            {
                AppContext.SetSwitch("Switch.System.IO.UseLegacyPathHandling", false);
                AppContext.SetSwitch("Switch.System.IO.BlockLongPaths", false);
                string allowed = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "../../out")) + Path.DirectorySeparatorChar;
                if (args.Length != 1) throw new Exception("Pass a new output directory under uefi/out.");
                string output = Path.GetFullPath(args[0]);
                if (!output.StartsWith(allowed, StringComparison.OrdinalIgnoreCase) || Directory.Exists(output))
                    throw new Exception("QA output must be a new directory under uefi/out.");
                Directory.CreateDirectory(output);
                byte[] efi = Payload.Efi(), readme = Payload.Readme(); Guards(output); Copy(output, efi, readme);
                Fat32Layout layout = Fat32Layout.Create(64UL << 20, 512);
                string image = Path.Combine(output, "installer-usb.img");
                using (FileBlock block = new FileBlock(image, (long)layout.DiskBytes))
                    Fat32Install.Run(block, layout, efi, readme, (p, s) => { });
                Assert(true, "formatted image verified before MBR publication");
                using (FileBlock block = new FileBlock(Path.Combine(output, "failed-verification.img"), (long)layout.DiskBytes))
                {
                    block.CorruptReads = true;
                    Refuses(() => Fat32Install.Run(block, layout, efi, readme, (p, s) => { }), "corrupt format verification refuses success");
                    block.CorruptReads = false;
                    Assert(block.Read(0, 512).All(b => b == 0), "failed verification does not publish MBR");
                }
                Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
                UsbDisk example = Example("E:\\"); Probe previewProbe = new Probe { Disk = example };
                SetupForm ready = new SetupForm(previewProbe, false); ready.SelectForPreview(example);
                Screenshot(ready, Path.Combine(output, "setup-ready.png"));
                ConfirmForm confirm = new ConfirmForm(new InstallPlan(example, null, true));
                Assert(!confirm.Install.Enabled, "erase disabled without phrase");
                confirm.Consent.Text = "erase"; Assert(!confirm.Install.Enabled, "lowercase erase refused in dialog");
                confirm.Consent.Text = "ERASE"; Assert(confirm.Install.Enabled, "erase enabled after phrase");
                confirm.Consent.Text = ""; Screenshot(confirm, Path.Combine(output, "setup-confirm.png"));
                SetupForm finished = new SetupForm(previewProbe, false); finished.SelectForPreview(example); finished.ShowReady(null);
                Screenshot(finished, Path.Combine(output, "setup-finished.png"));
                File.WriteAllText(Path.Combine(output, "checks.json"), "{\"passed\":true,\"checks\":" + passed + ",\"efi_sha256\":\"" + Safety.Hash(efi) + "\",\"physical_disks_written\":0}");
                Console.WriteLine("PASS: " + passed + " checks; no physical disks written.\nImage: " + image); return 0;
            }
            catch (Exception e) { Console.Error.WriteLine(e); return 1; }
        }
    }
}
