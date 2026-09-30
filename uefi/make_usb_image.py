"""Write a 64 MiB MBR/FAT32 image containing the removable UEFI boot path.

No disk devices are opened: the only output is the explicitly named image file.
The partition has enough clusters to be FAT32 by count, even on strict firmware.
"""
from pathlib import Path
import struct

SECTOR = 512
IMAGE_SECTORS = 64*1024*1024//SECTOR
START = 2048
RESERVED = 32
EOC = 0x0FFFFFFF


def directory_entry(name, attr, cluster, size=0):
    if len(name) != 11:
        raise ValueError("directory names must use the 11-byte short-name format")
    entry = bytearray(32)
    entry[:11] = name
    entry[11] = attr
    struct.pack_into("<H", entry, 16, 0x0021)  # valid FAT date: 1980-01-01
    struct.pack_into("<H", entry, 24, 0x0021)
    struct.pack_into("<H", entry, 20, cluster >> 16)
    struct.pack_into("<H", entry, 26, cluster & 0xFFFF)
    struct.pack_into("<I", entry, 28, size)
    return entry


def make_image(efi, readme, output):
    total = IMAGE_SECTORS-START
    fat_sectors = 1
    while True:
        clusters = total-RESERVED-2*fat_sectors  # one sector per cluster
        required = ((clusters+2)*4+SECTOR-1)//SECTOR
        if required <= fat_sectors:
            break
        fat_sectors = required
    if not 65525 <= clusters < 0x0FFFFFF5:
        raise ValueError("partition cluster count does not describe FAT32")
    image = bytearray(IMAGE_SECTORS*SECTOR)
    image[446:462] = struct.pack("<B3sB3sII", 0x80, b"\xfe\xff\xff", 0x0C,
                                 b"\xfe\xff\xff", START, total)
    struct.pack_into("<I", image, 440, 0x49425433)
    image[510:512] = b"\x55\xaa"
    boot = bytearray(SECTOR)
    boot[:3] = b"\xeb\x58\x90"
    boot[3:11] = b"IBTUEFI "
    struct.pack_into("<HBHBHHBHHHII", boot, 11, SECTOR, 1, RESERVED, 2,
                     0, 0, 0xF8, 0, 63, 255, START, total)
    struct.pack_into("<IHHIHH", boot, 36, fat_sectors, 0, 0, 2, 1, 6)
    boot[64] = 0x80
    boot[66] = 0x29
    struct.pack_into("<I", boot, 67, 0x49425433)
    boot[71:82] = b"IBT UEFI   "
    boot[82:90] = b"FAT32   "
    boot[510:512] = b"\x55\xaa"
    base = START*SECTOR
    image[base:base+SECTOR] = boot
    image[base+6*SECTOR:base+7*SECTOR] = boot
    fat = bytearray(fat_sectors*SECTOR)
    struct.pack_into("<IIIII", fat, 0, 0x0FFFFFF8, EOC, EOC, EOC, EOC)
    data_start = base+(RESERVED+2*fat_sectors)*SECTOR

    def write_cluster(cluster, data):
        if len(data) > SECTOR:
            raise ValueError("cluster payload too large")
        offset = data_start+(cluster-2)*SECTOR
        image[offset:offset+len(data)] = data

    next_cluster = 5

    def write_file(contents):
        nonlocal next_cluster
        first = next_cluster
        count = max(1, (len(contents)+SECTOR-1)//SECTOR)
        if next_cluster+count > clusters+2:
            raise ValueError("payload does not fit the USB image")
        for i in range(count):
            cluster = next_cluster+i
            successor = EOC if i+1 == count else cluster+1
            struct.pack_into("<I", fat, cluster*4, successor)
            write_cluster(cluster, contents[i*SECTOR:(i+1)*SECTOR])
        next_cluster += count
        return first

    efi_first = write_file(efi)
    readme_first = write_file(readme)
    write_cluster(2, directory_entry(b"IBT UEFI   ", 8, 0) +
                  directory_entry(b"EFI        ", 16, 3) +
                  directory_entry(b"README  TXT", 32, readme_first, len(readme)))
    write_cluster(3, directory_entry(b".          ", 16, 3) +
                  directory_entry(b"..         ", 16, 0) +
                  directory_entry(b"BOOT       ", 16, 4))
    write_cluster(4, directory_entry(b".          ", 16, 4) +
                  directory_entry(b"..         ", 16, 3) +
                  directory_entry(b"BOOTX64 EFI", 32, efi_first, len(efi)))
    fsinfo = bytearray(SECTOR)
    struct.pack_into("<I", fsinfo, 0, 0x41615252)
    struct.pack_into("<III", fsinfo, 484, 0x61417272, clusters-(next_cluster-2), next_cluster)
    struct.pack_into("<I", fsinfo, 508, 0xAA550000)
    image[base+SECTOR:base+2*SECTOR] = fsinfo
    image[base+7*SECTOR:base+8*SECTOR] = fsinfo
    fat_start = base+RESERVED*SECTOR
    image[fat_start:fat_start+len(fat)] = fat
    image[fat_start+len(fat):fat_start+2*len(fat)] = fat
    Path(output).write_bytes(image)
