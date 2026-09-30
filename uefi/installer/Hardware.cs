using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32.SafeHandles;

namespace IntelBurnTestUsb
{
    internal static class Native
    {
        const uint Read = 0x80000000, Write = 0x40000000;
        internal const uint Lock = 0x00090018, Dismount = 0x00090020;
        internal const uint StorageQuery = 0x002d1400, DeviceNumber = 0x002d1080;
        internal const uint LengthInfo = 0x0007405c, Geometry = 0x00070000;
        internal const uint UpdateProperties = 0x00070140, VolumeExtents = 0x00560000;
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern SafeFileHandle CreateFile(string path, uint access, uint share, IntPtr security, uint creation, uint flags, IntPtr template);
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool DeviceIoControl(SafeFileHandle handle, uint control, byte[] input, uint inSize, byte[] output, uint outSize, out uint returned, IntPtr overlapped);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern bool GetVolumeNameForVolumeMountPoint(string mount, StringBuilder name, uint length);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern bool GetVolumeInformation(string root, StringBuilder label, uint labelSize, out uint serial, out uint maxName, out uint flags, StringBuilder fs, uint fsSize);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern bool GetDiskFreeSpaceEx(string root, out ulong available, out ulong total, out ulong free);

        internal static SafeFileHandle Open(string path, bool writable)
        {
            SafeFileHandle handle = CreateFile(path, Read | (writable ? Write : 0), 3, IntPtr.Zero, 3,
                writable ? 0x80000000U : 0U, IntPtr.Zero); // OPEN_EXISTING; FILE_FLAG_WRITE_THROUGH
            if (handle.IsInvalid) { handle.Dispose(); throw Error("Cannot open the USB. Close apps using it, then refresh the list"); }
            return handle;
        }
        internal static IOException Error(string message)
        { return new IOException(message + ". " + new Win32Exception(Marshal.GetLastWin32Error()).Message); }
        internal static byte[] Control(SafeFileHandle handle, uint control, byte[] input, int size)
        {
            byte[] output = new byte[size]; uint returned;
            if (!DeviceIoControl(handle, control, input, input == null ? 0 : (uint)input.Length, output, (uint)size, out returned, IntPtr.Zero))
                throw Error("The USB could not be accessed safely");
            if (returned < size && size != 0) Array.Resize(ref output, checked((int)returned));
            return output;
        }
        internal static string Serial(SafeFileHandle handle)
        {
            byte[] query = new byte[12]; // StorageDeviceProperty, PropertyStandardQuery
            byte[] header = Control(handle, StorageQuery, query, 8);
            if (header.Length < 8) throw new IOException("USB identity is unavailable.");
            uint size = BitConverter.ToUInt32(header, 4);
            if (size < 36 || size > 65536) throw new IOException("USB identity is unavailable.");
            byte[] descriptor = Control(handle, StorageQuery, query, (int)size);
            if (descriptor.Length < 36 || BitConverter.ToUInt32(descriptor, 28) != 7)
                throw new IOException("This is not a USB disk. Installation was cancelled.");
            uint offset = BitConverter.ToUInt32(descriptor, 24);
            if (offset == 0) return "";
            if (offset >= descriptor.Length) throw new IOException("Invalid USB identity.");
            int end = (int)offset; while (end < descriptor.Length && descriptor[end] != 0) end++;
            return Encoding.ASCII.GetString(descriptor, (int)offset, end - (int)offset).Trim();
        }
        internal static ulong DiskLength(SafeFileHandle handle)
        {
            byte[] bytes = Control(handle, LengthInfo, null, 8);
            if (bytes.Length != 8 || BitConverter.ToInt64(bytes, 0) <= 0) throw new IOException("USB size is unavailable.");
            return (ulong)BitConverter.ToInt64(bytes, 0);
        }
        internal static void MatchDisk(SafeFileHandle handle, UsbDisk disk)
        {
            byte[] number = Control(handle, DeviceNumber, null, 12);
            byte[] geometry = Control(handle, Geometry, null, 24);
            if (number.Length != 12 || geometry.Length != 24 || BitConverter.ToUInt32(number, 0) != 7 ||
                BitConverter.ToUInt32(number, 4) != disk.Number || BitConverter.ToUInt32(geometry, 20) != disk.SectorSize ||
                DiskLength(handle) != disk.Size || !String.Equals(Serial(handle), disk.HardwareSerial, StringComparison.Ordinal))
                throw new IOException("The selected USB changed. Refresh the list and select it again.");
        }
        internal static void MatchVolume(SafeFileHandle handle, uint diskNumber)
        {
            byte[] extents = Control(handle, VolumeExtents, null, 1024);
            // DISK_EXTENT begins at offset 8 on x64, with eight-byte aligned LARGE_INTEGERs.
            if (extents.Length < 32 || BitConverter.ToUInt32(extents, 0) != 1 || BitConverter.ToUInt32(extents, 8) != diskNumber)
                throw new IOException("A USB volume spans another disk. Installation was cancelled.");
        }
    }

    internal sealed class WindowsUsbProbe : IUsbProbe
    {
        const string Namespace = @"root\Microsoft\Windows\Storage";
        static bool? Flag(ManagementBaseObject o, string name)
        { object value = o[name]; return value == null ? (bool?)null : Convert.ToBoolean(value); }
        static string Value(ManagementBaseObject o, string name) { return Convert.ToString(o[name]).Trim(); }
        public List<UsbDisk> List()
        {
            List<UsbDisk> disks = new List<UsbDisk>();
            using (ManagementObjectSearcher search = new ManagementObjectSearcher(Namespace, "SELECT * FROM MSFT_Disk WHERE BusType = 7"))
            using (ManagementObjectCollection results = search.Get())
                foreach (ManagementObject result in results)
                {
                    using (result)
                    {
                        UsbDisk disk = new UsbDisk {
                            Number = Convert.ToUInt32(result["Number"]), Size = Convert.ToUInt64(result["Size"]),
                            SectorSize = Convert.ToUInt32(result["LogicalSectorSize"]), BusType = Convert.ToUInt16(result["BusType"]),
                            UniqueId = Value(result, "UniqueId"), DevicePath = Value(result, "Path"),
                            Serial = Value(result, "SerialNumber"), Model = Value(result, "FriendlyName"),
                            IsBoot = Flag(result, "IsBoot"), IsSystem = Flag(result, "IsSystem"),
                            IsReadOnly = Flag(result, "IsReadOnly"), IsOffline = Flag(result, "IsOffline")
                        };
                        if (!Safety.Eligible(disk)) continue;
                        uint expectedPartitions = Convert.ToUInt32(result["NumberOfPartitions"]);
                        try
                        {
                            ReadPartitions(disk);
                            if (!Safety.Eligible(disk) || disk.PartitionCount != expectedPartitions) continue;
                            using (SafeFileHandle handle = Native.Open(disk.DevicePath, false))
                            { disk.HardwareSerial = Native.Serial(handle); Native.MatchDisk(handle, disk); }
                            disks.Add(disk);
                        }
                        catch (IOException) { /* Unknown, busy, or disappeared disks are excluded. */ }
                    }
                }
            return disks.OrderBy(d => d.Number).ToList();
        }
        public UsbDisk Refresh(uint number) { return List().FirstOrDefault(d => d.Number == number); }
        static void ReadPartitions(UsbDisk disk)
        {
            using (ManagementObjectSearcher search = new ManagementObjectSearcher(Namespace,
                "SELECT * FROM MSFT_Partition WHERE DiskNumber = " + disk.Number))
            using (ManagementObjectCollection results = search.Get())
                foreach (ManagementObject partition in results)
                {
                    using (partition)
                    {
                        disk.PartitionCount++;
                        bool? boot = Flag(partition, "IsBoot"), system = Flag(partition, "IsSystem");
                        bool? readOnly = Flag(partition, "IsReadOnly");
                        if (boot != false || system != false || readOnly != false || Flag(partition, "IsOffline") != false)
                            disk.HasProtectedPartition = true;
                        string[] paths = partition["AccessPaths"] as string[] ?? new string[0];
                        string root = paths.FirstOrDefault(p => p.StartsWith(@"\\?\Volume{", StringComparison.OrdinalIgnoreCase));
                        string letter = Value(partition, "DriveLetter").Trim('\0');
                        if (letter.Length > 0) letter += ":";
                        if (root == null && letter.Length != 0)
                        {
                            StringBuilder name = new StringBuilder(1024);
                            if (!Native.GetVolumeNameForVolumeMountPoint(letter + "\\", name, (uint)name.Capacity))
                                throw Native.Error("USB volume identity is unavailable");
                            root = name.ToString();
                        }
                        if (root == null && paths.Length > 0) throw new IOException("USB volume identity is unavailable.");
                        if (root == null) continue; // Unmounted/foreign/reserved partition: no Windows filesystem to lock.
                        if (!root.EndsWith("\\", StringComparison.Ordinal)) root += "\\";
                        using (SafeFileHandle handle = Native.Open(root.TrimEnd('\\'), false)) Native.MatchVolume(handle, disk.Number);
                        uint serial, maximum, flags;
                        StringBuilder label = new StringBuilder(261), fs = new StringBuilder(261);
                        ulong available = 0, total, free;
                        bool info = Native.GetVolumeInformation(root, label, 261, out serial, out maximum, out flags, fs, 261);
                        bool space = info && Native.GetDiskFreeSpaceEx(root, out available, out total, out free);
                        if (!space) available = 0;
                        disk.Volumes.Add(new UsbVolume { Root = root, Letter = letter, Label = info ? label.ToString() : "Unformatted",
                            FileSystem = info ? fs.ToString() : "Unknown", Serial = info ? serial : 0, FreeBytes = available,
                            IsBoot = boot, IsSystem = system, IsReadOnly = readOnly == true || (info && (flags & 0x80000) != 0) });
                    }
                }
        }
    }

    internal sealed class RawUsb : IBlockDevice
    {
        readonly List<SafeFileHandle> locks = new List<SafeFileHandle>();
        FileStream stream;
        public ulong Length { get; private set; }
        public RawUsb(InstallPlan plan, IUsbProbe probe)
        {
            if (!plan.Erase) throw new InvalidOperationException("Raw writes require an erase plan.");
            UsbDisk disk = plan.Revalidate(probe);
            try
            {
                foreach (UsbVolume volume in disk.Volumes)
                {
                    SafeFileHandle handle = Native.Open(volume.Root.TrimEnd('\\'), true);
                    locks.Add(handle); Native.MatchVolume(handle, disk.Number);
                    Native.Control(handle, Native.Lock, null, 0);
                    Native.Control(handle, Native.Dismount, null, 0);
                }
                // This exact device handle remains open throughout all writes.
                SafeFileHandle physical = Native.Open(disk.DevicePath, true);
                try { Native.MatchDisk(physical, disk); stream = new FileStream(physical, FileAccess.ReadWrite, 65536, false); }
                catch { physical.Dispose(); throw; }
                Length = disk.Size;
            }
            catch { Dispose(); throw; }
        }
        void Range(ulong offset, int size)
        {
            if (offset % 512 != 0 || size % 512 != 0 || offset > Length || (ulong)size > Length - offset)
                throw new IOException("Invalid USB write boundary.");
        }
        public void Write(ulong offset, byte[] bytes) { Range(offset, bytes.Length); stream.Seek(checked((long)offset), SeekOrigin.Begin); stream.Write(bytes, 0, bytes.Length); }
        public byte[] Read(ulong offset, int count)
        {
            Range(offset, count); stream.Seek(checked((long)offset), SeekOrigin.Begin);
            byte[] bytes = new byte[count]; int at = 0;
            while (at < count) { int n = stream.Read(bytes, at, count - at); if (n == 0) throw new IOException("The USB disconnected during verification."); at += n; }
            return bytes;
        }
        public void Flush() { stream.Flush(true); }
        public void Complete()
        {
            Flush();
            // Release old volumes before asking Windows to discover the new partition.
            foreach (SafeFileHandle handle in locks) handle.Dispose(); locks.Clear();
            Native.Control(stream.SafeFileHandle, Native.UpdateProperties, null, 0);
        }
        public void Dispose()
        {
            if (stream != null) { stream.Dispose(); stream = null; }
            foreach (SafeFileHandle handle in locks) handle.Dispose(); locks.Clear();
        }
    }
}
