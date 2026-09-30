using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace IntelBurnTestUsb
{
    internal sealed class UsbVolume
    {
        public string Root, Letter, Label, FileSystem;
        public uint Serial;
        public ulong FreeBytes;
        public bool? IsBoot, IsSystem, IsReadOnly;
        public override string ToString()
        {
            return (String.IsNullOrEmpty(Letter) ? "No drive letter" : Letter) + "  " +
                (String.IsNullOrEmpty(Label) ? "USB volume" : Label) + "  (" + FileSystem + ")";
        }
    }

    internal sealed class UsbDisk
    {
        public uint Number, SectorSize, PartitionCount;
        public ulong Size;
        public ushort BusType;
        public string UniqueId, DevicePath, Serial, HardwareSerial, Model;
        public bool? IsBoot, IsSystem, IsReadOnly, IsOffline;
        public bool HasProtectedPartition;
        public List<UsbVolume> Volumes = new List<UsbVolume>();
        public string Identity
        {
            get { return String.Join("|", Number, Size, SectorSize, UniqueId, DevicePath, Serial, HardwareSerial); }
        }
        public override string ToString()
        {
            string letters = String.Join(", ", Volumes.Select(v => v.Letter).Where(s => !String.IsNullOrEmpty(s)));
            return Model + "  —  " + (Size / (1024.0 * 1024 * 1024)).ToString("0.0") +
                " GiB" + (letters.Length == 0 ? "" : "  (" + letters + ")");
        }
    }

    internal interface IUsbProbe
    {
        List<UsbDisk> List();
        UsbDisk Refresh(uint number);
    }

    internal sealed class InstallPlan
    {
        public readonly UsbDisk Disk;
        public readonly UsbVolume Volume;
        public readonly bool Erase;
        public readonly string Identity, VolumeIdentity, LayoutIdentity;
        public InstallPlan(UsbDisk disk, UsbVolume volume, bool erase)
        {
            Safety.RequireEligible(disk);
            if (erase) Fat32Layout.Create(disk.Size, disk.SectorSize);
            else
            {
                Safety.RequireCopyVolume(volume);
                if (!disk.Volumes.Any(v => v.Root == volume.Root && v.Serial == volume.Serial))
                    throw new IOException("The selected volume does not belong to this USB.");
            }
            Disk = disk; Volume = volume; Erase = erase;
            Identity = disk.Identity;
            VolumeIdentity = volume == null ? "" : volume.Root + "|" + volume.Serial;
            LayoutIdentity = Layout(disk);
        }
        static string Layout(UsbDisk disk)
        {
            return disk.PartitionCount + "|" + String.Join("|", disk.Volumes.Select(v =>
                v.Root + ":" + v.Serial + ":" + v.Letter).OrderBy(s => s, StringComparer.OrdinalIgnoreCase));
        }
        public UsbDisk Revalidate(IUsbProbe probe)
        {
            UsbDisk fresh = probe.Refresh(Disk.Number);
            Safety.RequireEligible(fresh);
            if (!String.Equals(Identity, fresh.Identity, StringComparison.OrdinalIgnoreCase) ||
                !String.Equals(LayoutIdentity, Layout(fresh), StringComparison.OrdinalIgnoreCase))
                throw new IOException("The USB changed or was disconnected. Refresh the list and select it again.");
            if (!Erase)
            {
                UsbVolume volume = fresh.Volumes.FirstOrDefault(v => String.Equals(
                    v.Root + "|" + v.Serial, VolumeIdentity, StringComparison.OrdinalIgnoreCase));
                Safety.RequireCopyVolume(volume);
                if (!String.Equals(volume.Letter, Volume.Letter, StringComparison.OrdinalIgnoreCase))
                    throw new IOException("The USB drive letter changed. Select it again.");
            }
            return fresh;
        }
    }

    internal static class Safety
    {
        public static bool Eligible(UsbDisk disk)
        {
            return disk != null && disk.BusType == 7 && disk.IsBoot == false && disk.IsSystem == false &&
                disk.IsReadOnly == false && disk.IsOffline == false && !disk.HasProtectedPartition &&
                disk.Size >= 64UL * 1024 * 1024 && !String.IsNullOrWhiteSpace(disk.DevicePath);
        }
        public static void RequireEligible(UsbDisk disk)
        {
            if (!Eligible(disk)) throw new IOException("This drive is not an eligible USB. System and internal disks cannot be used.");
        }
        public static bool Copyable(UsbVolume v)
        {
            return v != null && v.IsBoot == false && v.IsSystem == false && v.IsReadOnly == false &&
                String.Equals(v.FileSystem, "FAT32", StringComparison.OrdinalIgnoreCase) &&
                !String.IsNullOrEmpty(v.Root);
        }
        public static void RequireCopyVolume(UsbVolume v)
        {
            if (!Copyable(v)) throw new IOException("Choose a writable FAT32 volume, or explicitly choose to format the USB.");
        }
        public static bool EraseConsent(string text) { return text != null && String.Equals(text.Trim(), "ERASE", StringComparison.Ordinal); }
        public static string Hash(byte[] bytes)
        {
            using (SHA256 sha = SHA256.Create()) return BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant();
        }
        public static bool SameFile(string path, byte[] bytes)
        {
            return File.Exists(path) && new FileInfo(path).Length == bytes.Length && Hash(File.ReadAllBytes(path)) == Hash(bytes);
        }
    }

    internal static class FileInstall
    {
        // Bind all writes to the selected volume GUID, not a reusable drive letter.
        public static string Run(InstallPlan plan, IUsbProbe probe, byte[] efi, byte[] readme, Action<int, string> progress)
        {
            if (plan.Erase) throw new InvalidOperationException("Copy installation cannot erase a disk.");
            plan.Revalidate(probe);
            string root = plan.Volume.Root;
            string boot = Path.Combine(root, "EFI", "BOOT");
            string notes = Path.Combine(root, "IntelBurnTest");
            ulong need = (ulong)efi.Length + (ulong)readme.Length + 65536;
            string target = Path.Combine(boot, "BOOTX64.EFI");
            if (File.Exists(target)) need += (ulong)new FileInfo(target).Length;
            if (plan.Volume.FreeBytes < need) throw new IOException("Not enough free space. Free some space on the USB and try again.");
            RejectReparse(root); RejectReparse(Path.Combine(root, "EFI")); RejectReparse(boot); RejectReparse(notes); RejectReparse(target);
            string unique = Guid.NewGuid().ToString("N");
            string staged = Path.Combine(boot, "IBT-" + unique + ".tmp");
            string backup = null;
            bool replaced = false, originallyPresent = File.Exists(target);
            try
            {
                plan.Revalidate(probe);
                Directory.CreateDirectory(boot);
                progress(20, "Copying test files...");
                WriteNew(staged, efi);
                if (!Safety.SameFile(staged, efi)) throw new IOException("The copied file did not verify. No boot file was replaced.");
                plan.Revalidate(probe);
                if (File.Exists(target) && !Safety.SameFile(target, efi))
                {
                    backup = Path.Combine(boot, "BOOTX64-IBT-BACKUP-" + unique + ".EFI");
                    File.Copy(target, backup, false);
                    if (Safety.Hash(File.ReadAllBytes(target)) != Safety.Hash(File.ReadAllBytes(backup)))
                        throw new IOException("The existing boot file could not be backed up. Installation was cancelled.");
                }
                progress(60, "Installing the boot file...");
                plan.Revalidate(probe);
                if (!Safety.SameFile(target, efi))
                {
                    replaced = true;
                    File.Copy(staged, target, true);
                    using (FileStream stream = new FileStream(target, FileMode.Open, FileAccess.ReadWrite, FileShare.Read)) stream.Flush(true);
                }
                if (!Safety.SameFile(target, efi)) throw new IOException("The boot file did not verify.");
                plan.Revalidate(probe);
                Directory.CreateDirectory(notes);
                string note = Path.Combine(notes, "README-" + unique + ".TXT");
                WriteNew(note, readme);
                if (!Safety.SameFile(note, readme)) throw new IOException("The instructions file did not verify.");
                progress(100, "USB ready");
                return backup;
            }
            catch
            {
                // A failed write is never reported as ready. Best-effort restore from a verified backup.
                if (replaced)
                {
                    try
                    {
                        plan.Revalidate(probe);
                        if (backup != null) File.Copy(backup, target, true);
                        else if (!originallyPresent && File.Exists(target)) File.Delete(target);
                    }
                    catch { /* Keep the backup for recovery if the USB is no longer accessible. */ }
                }
                throw;
            }
            finally
            {
                try { plan.Revalidate(probe); if (File.Exists(staged)) File.Delete(staged); } catch { }
            }
        }
        static void WriteNew(string path, byte[] bytes)
        {
            using (FileStream stream = new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.None, 4096, FileOptions.WriteThrough))
            { stream.Write(bytes, 0, bytes.Length); stream.Flush(true); }
        }
        static void RejectReparse(string path)
        {
            if ((File.Exists(path) || Directory.Exists(path)) && (File.GetAttributes(path) & FileAttributes.ReparsePoint) != 0)
                throw new IOException("A USB installation directory is a link. Installation was cancelled.");
        }
    }

    internal sealed class Fat32Layout
    {
        public const uint Start = 2048, Reserved = 32;
        public readonly ulong DiskBytes;
        public readonly uint Sectors, ClusterSectors, FatSectors, Clusters;
        public ulong ClusterBytes { get { return ClusterSectors * 512UL; } }
        public ulong Base { get { return Start * 512UL; } }
        public ulong FatOffset { get { return Base + Reserved * 512UL; } }
        public ulong DataOffset { get { return FatOffset + FatSectors * 1024UL; } }
        Fat32Layout(ulong bytes, uint sectors, uint spc, uint fat, uint clusters)
        { DiskBytes = bytes; Sectors = sectors; ClusterSectors = spc; FatSectors = fat; Clusters = clusters; }
        public static Fat32Layout Create(ulong bytes, uint sector)
        {
            if (sector != 512) throw new IOException("Formatting requires a USB with 512-byte logical sectors. Use an existing FAT32 USB instead.");
            if (bytes % 512 != 0 || bytes < 64UL * 1024 * 1024 || bytes / 512 > UInt32.MaxValue)
                throw new IOException("Formatting supports USB drives from 64 MiB to 2 TiB.");
            uint total = checked((uint)(bytes / 512) - Start);
            uint spc = bytes <= 260UL * 1024 * 1024 ? 1U : bytes <= 8UL * 1024 * 1024 * 1024 ? 8U : bytes <= 32UL * 1024 * 1024 * 1024 ? 32U : 64U;
            uint fat = 1, clusters;
            while (true)
            {
                clusters = (total - Reserved - 2 * fat) / spc;
                uint required = checked((uint)(((ulong)clusters + 2) * 4 + 511) / 512);
                if (required <= fat) break;
                fat = required;
            }
            if (clusters < 65525 || clusters >= 0x0ffffff5) throw new IOException("USB size does not describe a valid FAT32 volume.");
            return new Fat32Layout(bytes, total, spc, fat, clusters);
        }
        public ulong ClusterOffset(uint cluster) { return DataOffset + (cluster - 2) * ClusterBytes; }
    }

    internal interface IBlockDevice : IDisposable
    {
        ulong Length { get; }
        void Write(ulong offset, byte[] bytes);
        byte[] Read(ulong offset, int count);
        void Flush();
    }

    internal static class Fat32Install
    {
        const uint Eoc = 0x0fffffff;
        static void Put16(byte[] b, int o, ushort n) { b[o] = (byte)n; b[o + 1] = (byte)(n >> 8); }
        static void Put32(byte[] b, int o, uint n) { for (int i = 0; i < 4; i++) b[o + i] = (byte)(n >> (8 * i)); }
        static void Text(byte[] b, int o, string s) { Encoding.ASCII.GetBytes(s).CopyTo(b, o); }
        static byte[] Entry(string name, byte attr, uint cluster, uint length)
        {
            byte[] b = new byte[32]; Text(b, 0, name); b[11] = attr;
            Put16(b, 16, 0x21); Put16(b, 24, 0x21);
            Put16(b, 20, (ushort)(cluster >> 16)); Put16(b, 26, (ushort)cluster); Put32(b, 28, length); return b;
        }
        static byte[] Directory(int size, params byte[][] entries)
        { byte[] b = new byte[size]; for (int i = 0; i < entries.Length; i++) entries[i].CopyTo(b, i * 32); return b; }
        static byte[] Pad(byte[] b, int alignment)
        { byte[] padded = new byte[checked((b.Length + alignment - 1) / alignment * alignment)]; b.CopyTo(padded, 0); return padded; }
        static void Verify(IBlockDevice disk, ulong offset, byte[] expected)
        {
            byte[] read = disk.Read(offset, expected.Length);
            if (!read.SequenceEqual(expected)) throw new IOException("USB verification failed. Do not use this USB until installation succeeds.");
        }
        // The MBR is published last, after FATs, directories, and payload verify.
        // Only filesystem metadata and allocated clusters are initialized: this is not a secure wipe.
        public static void Run(IBlockDevice disk, Fat32Layout layout, byte[] efi, byte[] readme, Action<int, string> progress)
        {
            if (disk.Length != layout.DiskBytes) throw new IOException("The USB size changed. Installation was cancelled.");
            uint efiCount = checked((uint)(((ulong)efi.Length + layout.ClusterBytes - 1) / layout.ClusterBytes));
            uint notesCount = checked((uint)(((ulong)readme.Length + layout.ClusterBytes - 1) / layout.ClusterBytes));
            uint notesStart = 5 + efiCount, next = notesStart + notesCount;
            if (efiCount == 0 || notesCount == 0 || next >= layout.Clusters + 2) throw new IOException("The boot payload does not fit.");
            byte[] fat = new byte[checked((int)((next * 4UL + 511) / 512 * 512))];
            Put32(fat, 0, 0x0ffffff8); Put32(fat, 4, Eoc);
            for (uint n = 2; n <= 4; n++) Put32(fat, (int)n * 4, Eoc);
            for (uint n = 5; n < notesStart; n++) Put32(fat, (int)n * 4, n + 1 == notesStart ? Eoc : n + 1);
            for (uint n = notesStart; n < next; n++) Put32(fat, (int)n * 4, n + 1 == next ? Eoc : n + 1);
            uint serial = BitConverter.ToUInt32(Guid.NewGuid().ToByteArray(), 0) | 1;
            byte[] boot = new byte[512]; boot[0] = 0xeb; boot[1] = 0x58; boot[2] = 0x90;
            Text(boot, 3, "IBTUEFI "); Put16(boot, 11, 512); boot[13] = (byte)layout.ClusterSectors;
            Put16(boot, 14, (ushort)Fat32Layout.Reserved); boot[16] = 2; boot[21] = 0xf8;
            Put16(boot, 24, 63); Put16(boot, 26, 255); Put32(boot, 28, Fat32Layout.Start);
            Put32(boot, 32, layout.Sectors); Put32(boot, 36, layout.FatSectors); Put32(boot, 44, 2);
            Put16(boot, 48, 1); Put16(boot, 50, 6); boot[64] = 0x80; boot[66] = 0x29;
            Put32(boot, 67, serial); Text(boot, 71, "INTELBURN  "); Text(boot, 82, "FAT32   "); boot[510] = 0x55; boot[511] = 0xaa;
            byte[] info = new byte[512]; Put32(info, 0, 0x41615252); Put32(info, 484, 0x61417272);
            Put32(info, 488, layout.Clusters - (next - 2)); Put32(info, 492, next); Put32(info, 508, 0xaa550000);
            byte[] mbr = new byte[512]; Put32(mbr, 440, serial); mbr[446] = 0x80; mbr[450] = 0x0c;
            mbr[447] = mbr[451] = 0xfe; mbr[448] = mbr[449] = mbr[452] = mbr[453] = 0xff;
            Put32(mbr, 454, Fat32Layout.Start); Put32(mbr, 458, layout.Sectors); mbr[510] = 0x55; mbr[511] = 0xaa;
            progress(5, "Preparing FAT32...");
            byte[] zero = new byte[65536];
            ulong metadataEnd = layout.DataOffset;
            for (ulong offset = 512; offset < metadataEnd; )
            {
                int count = (int)Math.Min((ulong)zero.Length, metadataEnd - offset);
                disk.Write(offset, count == zero.Length ? zero : new byte[count]); offset += (ulong)count;
                progress(5 + (int)(offset * 65 / metadataEnd), "Preparing FAT32...");
            }
            // Remove a previous GPT backup so it cannot supersede the new MBR.
            disk.Write(layout.DiskBytes / 512 * 512 - 33 * 512, new byte[33 * 512]);
            disk.Write(layout.Base, boot); disk.Write(layout.Base + 6 * 512, boot);
            disk.Write(layout.Base + 512, info); disk.Write(layout.Base + 7 * 512, info);
            disk.Write(layout.FatOffset, fat); disk.Write(layout.FatOffset + layout.FatSectors * 512UL, fat);
            int clusterBytes = checked((int)layout.ClusterBytes);
            byte[] root = Directory(clusterBytes, Entry("INTELBURN  ", 8, 0, 0), Entry("EFI        ", 16, 3, 0), Entry("README  TXT", 32, notesStart, (uint)readme.Length));
            byte[] efiDir = Directory(clusterBytes, Entry(".          ", 16, 3, 0), Entry("..         ", 16, 0, 0), Entry("BOOT       ", 16, 4, 0));
            byte[] bootDir = Directory(clusterBytes, Entry(".          ", 16, 4, 0), Entry("..         ", 16, 3, 0), Entry("BOOTX64 EFI", 32, 5, (uint)efi.Length));
            progress(75, "Installing the test...");
            disk.Write(layout.ClusterOffset(2), root); disk.Write(layout.ClusterOffset(3), efiDir); disk.Write(layout.ClusterOffset(4), bootDir);
            byte[] paddedEfi = Pad(efi, clusterBytes), paddedNotes = Pad(readme, clusterBytes);
            disk.Write(layout.ClusterOffset(5), paddedEfi); disk.Write(layout.ClusterOffset(notesStart), paddedNotes);
            disk.Flush(); progress(90, "Verifying the USB...");
            Verify(disk, layout.Base, boot); Verify(disk, layout.Base + 512, info);
            Verify(disk, layout.FatOffset, fat); Verify(disk, layout.FatOffset + layout.FatSectors * 512UL, fat);
            Verify(disk, layout.ClusterOffset(2), root); Verify(disk, layout.ClusterOffset(3), efiDir); Verify(disk, layout.ClusterOffset(4), bootDir);
            Verify(disk, layout.ClusterOffset(5), paddedEfi); Verify(disk, layout.ClusterOffset(notesStart), paddedNotes);
            disk.Write(0, mbr); disk.Flush(); Verify(disk, 0, mbr);
            progress(100, "USB ready");
        }
    }
}
