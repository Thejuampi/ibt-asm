# IntelBurnTest USB Setup

`IntelBurnTest-USB-Setup.exe` is a single Windows x64 executable containing the
release EFI application and boot instructions. It prepares a USB for an x64
UEFI PC; it does not install anything on the Windows system drive. Windows
10/11 with .NET Framework 4.8 is required. The installer requests administrator
access to inspect and, only when confirmed, prepare USB disks.

## User flow

1. Connect a USB, open the installer, and select the USB by model, capacity,
   drive letter, and volume label.
2. **FAT32:** click Prepare USB and confirm. Other files are preserved. An
   existing `EFI/BOOT/BOOTX64.EFI` with different contents is copied to a unique
   `BOOTX64-IBT-BACKUP-*.EFI` file before replacement. The success screen gives
   its name. Restore that backup to `BOOTX64.EFI` to restore the previous boot
   application.
3. **Other filesystems:** back up your files and select the format checkbox.
   The confirmation identifies the entire USB and all affected Windows volumes.
   The Erase and install button stays disabled until **ERASE** is typed.
   Cancel is the dialog's default action. Formatting deletes all partitions
   and files; this is a filesystem reset, not a secure data wipe.
4. Keep the USB connected until setup reports **USB ready**. Restart and select
   its UEFI boot entry. This release is unsigned: Secure Boot must be disabled
   or the EFI application must be signed with a trusted key.

Copy installation does not change the partition table. Format installation
creates one MBR FAT32 partition covering the USB's capacity, rather than
shrinking large USBs to the downloadable raw image's 64 MiB size. Formatting
supports 512-byte logical sectors and capacities from 64 MiB to below 2 TiB.
An existing FAT32 volume can still be used when raw formatting is unsupported.

The installer is unsigned; Windows may display a SmartScreen warning. Downloads
are supplied through the project's GitHub release, with a SHA-256 manifest.

## Write protections

The storage provider must explicitly identify USB bus type, an online writable
disk, and false system/boot flags. Missing protection information excludes the
disk. System, boot, internal, offline, and read-only disks cannot be selected.
All partitions must be enumerated and unprotected. Volumes spanning disks are
rejected.

The selected disk and volume identities are captured before confirmation and
rechecked before writes. Copy installation uses the volume GUID, not a reused
drive letter. It refuses linked installation paths, stages and hashes the
payload, verifies backups, and attempts to restore a previous boot file if a
later operation fails. A disconnected disk or failed verification never
reports success.

Raw formatting requires exclusive locks on every existing Windows volume. A
lock failure cancels the operation: setup does not forcibly dismount busy
volumes. An opened physical-device handle must match the USB bus, disk number,
sector size, byte length, and hardware serial. That handle remains open for the
whole write. The writer initializes FAT32 metadata and allocated clusters,
verifies both FAT heads, boot structures, directories, and payload, and writes
the new MBR last. A format interrupted by removal or power loss still needs to
be repeated; it cannot recover the files the user agreed to erase.

## Build and checks

From the repository root, on Windows with NASM, LLVM, Python, and .NET Framework
4.8 installed:

```console
python uefi/build.py --installer
uefi\build\installer\InstallerChecks.exe uefi/out/installer-checks
```

The second command requires a new directory under `uefi/out`. It runs a separate
QA executable: no tests open a physical disk for writing. Tests cover unsafe and
unknown devices, hot-plug identity changes, volume reassignment, explicit erase
confirmation, preservation and backup of existing files, rollback, full-capacity
FAT32 geometry, successful image verification, and refusal to publish an MBR
after corrupted reads. Native form renders are saved for visual inspection.

The formatter writes a regular 64 MiB file for independent `fsck.fat`, `mtools`,
and QEMU/OVMF verification. To exercise the real release UI from that image,
copy the release `EFI` directory alongside it and run `uefi/ui_check.py` as
described in the parent README. Physical USB writing and firmware compatibility
have not been verified on real hardware in this release.

The Windows installer uses C# and Windows storage APIs. The program that boots
and performs the CPU test remains the NASM EFI application and loads no OS.
No Rufus code or executable is redistributed. The ZIP and raw image remain
available for users who prefer their existing USB preparation tool.

## Storage references

- [Microsoft: MSFT_Disk](https://learn.microsoft.com/en-us/windows-hardware/drivers/storage/msft-disk)
- [Microsoft: MSFT_Partition](https://learn.microsoft.com/en-us/windows-hardware/drivers/storage/msft-partition)
- [Microsoft: FSCTL_LOCK_VOLUME](https://learn.microsoft.com/en-us/windows/win32/api/winioctl/ni-winioctl-fsctl_lock_volume)
- [Microsoft: FSCTL_DISMOUNT_VOLUME](https://learn.microsoft.com/en-us/windows/win32/api/winioctl/ni-winioctl-fsctl_dismount_volume)
- [Microsoft: STORAGE_DEVICE_DESCRIPTOR](https://learn.microsoft.com/en-us/windows/win32/api/winioctl/ns-winioctl-storage_device_descriptor)
