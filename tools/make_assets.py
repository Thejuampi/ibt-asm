from PIL import Image, ImageDraw
import os
import struct

ROOT = os.path.join(os.path.dirname(__file__), "..")
RES = os.path.join(ROOT, "res")
FLAME_OUTER = [
    (0, 90),
    (-70, 40),
    (-88, -10),
    (-50, -30),
    (-62, -90),
    (-18, -40),
    (0, -118),
    (22, -38),
    (58, -86),
    (48, -22),
    (86, -8),
    (68, 42),
]
FLAME_MID = [
    (0, 70),
    (-48, 28),
    (-58, -4),
    (-28, -16),
    (-34, -58),
    (-8, -22),
    (0, -78),
    (12, -20),
    (36, -52),
    (30, -10),
    (56, 0),
    (44, 30),
]
FLAME_INNER = [
    (0, 48),
    (-26, 18),
    (-22, -4),
    (0, -36),
    (22, -2),
    (24, 20),
]
COL_OUTER = (230, 72, 16)
COL_MID = (255, 168, 24)
COL_INNER = (255, 236, 140)
COL_KEY = (23, 27, 34)


def flame_icon(size):
    im = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = max(1, size // 16)
    bg = [m, m, size - 1 - m, size - 1 - m]
    d.rounded_rectangle(bg, radius=size // 5, fill=(28, 28, 32, 255))
    cx, cy = size / 2, size * 0.58
    sc = size / 256.0

    def P(pts):
        return [(cx + x * sc, cy + y * sc) for x, y in pts]

    d.polygon(P(FLAME_OUTER), fill=COL_OUTER + (255,))
    d.polygon(P(FLAME_MID), fill=COL_MID + (255,))
    d.polygon(P(FLAME_INNER), fill=COL_INNER + (255,))
    return im


def save_bmp4(image, path, transparent_key=None):
    """Write a compact, uncompressed 4-bpp BMP accepted by rc.exe/LoadImageW."""
    # The themed SS_BITMAP path treats palette index 0 as transparent. For
    # flames, put the panel-colored key there deliberately. An unused index 0
    # is not sufficient because rc.exe compacts it away.
    color_count = 16 if transparent_key is not None else 15
    pal = image.convert("RGB").quantize(
        colors=color_count,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    width, height = pal.size
    row_bytes = (width + 1) // 2
    stride = (row_bytes + 3) & ~3
    image_size = stride * height
    pixel_offset = 14 + 40 + 16 * 4
    file_size = pixel_offset + image_size
    source_palette = pal.getpalette()[: color_count * 3]
    source_palette += [0] * (color_count * 3 - len(source_palette))
    if transparent_key is not None:
        key_index = min(
            range(color_count),
            key=lambda index: sum(
                (source_palette[index * 3 + channel] - transparent_key[channel]) ** 2
                for channel in range(3)
            ),
        )
        order = [key_index] + [index for index in range(color_count) if index != key_index]
        remap = {old: new for new, old in enumerate(order)}
        palette = []
        for old in order:
            palette.extend(source_palette[old * 3 : old * 3 + 3])
    else:
        remap = {old: old + 1 for old in range(color_count)}
        palette = [0, 0, 0] + source_palette
    palette += [0] * (16 * 3 - len(palette))
    pixels = pal.load()

    with open(path, "wb") as handle:
        handle.write(struct.pack("<2sIHHI", b"BM", file_size, 0, 0, pixel_offset))
        handle.write(
            struct.pack(
                "<IiiHHIIiiII",
                40,
                width,
                height,
                1,
                4,
                0,
                image_size,
                2835,
                2835,
                16,
                16,
            )
        )
        for index in range(16):
            r, g, b = palette[index * 3 : index * 3 + 3]
            handle.write(bytes((b, g, r, 0)))
        padding = bytes(stride - row_bytes)
        for y in range(height - 1, -1, -1):
            row = bytearray()
            for x in range(0, width, 2):
                high = remap[pixels[x, y]] & 0x0F
                low = remap[pixels[x + 1, y]] & 0x0F if x + 1 < width else 0
                row.append((high << 4) | low)
            handle.write(row)
            handle.write(padding)


def save_bmp4_rle(image, path, transparent_key=None):
    """Write a 4-bpp BI_RLE4 BMP using solid runs plus absolute packets."""
    color_count = 16 if transparent_key is not None else 15
    pal = image.convert("RGB").quantize(
        colors=color_count,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    width, height = pal.size
    source_palette = pal.getpalette()[: color_count * 3]
    source_palette += [0] * (color_count * 3 - len(source_palette))
    if transparent_key is not None:
        key_index = min(
            range(color_count),
            key=lambda index: sum(
                (source_palette[index * 3 + channel] - transparent_key[channel]) ** 2
                for channel in range(3)
            ),
        )
        order = [key_index] + [index for index in range(color_count) if index != key_index]
        remap = {old: new for new, old in enumerate(order)}
        palette = []
        for old in order:
            palette.extend(source_palette[old * 3 : old * 3 + 3])
    else:
        remap = {old: old + 1 for old in range(color_count)}
        palette = [0, 0, 0] + source_palette
    palette += [0] * (16 * 3 - len(palette))

    pixels = pal.load()
    encoded = bytearray()
    for y in range(height - 1, -1, -1):
        row = [remap[pixels[x, y]] & 0x0F for x in range(width)]
        x = 0
        while x < width:
            run = 1
            while x + run < width and row[x + run] == row[x] and run < 255:
                run += 1
            if run >= 3:
                encoded.extend((run, row[x] * 0x11))
                x += run
                continue
            start = x
            x += run
            while x < width and x - start < 255:
                next_run = 1
                while (
                    x + next_run < width
                    and row[x + next_run] == row[x]
                    and next_run < 255
                ):
                    next_run += 1
                if next_run >= 3:
                    break
                x += min(next_run, 255 - (x - start))
            count = x - start
            if count <= 2:
                a = row[start]
                b = row[start + 1] if count == 2 else a
                encoded.extend((count, (a << 4) | b))
            else:
                encoded.extend((0, count))
                for pos in range(start, start + count, 2):
                    a = row[pos]
                    b = row[pos + 1] if pos + 1 < start + count else 0
                    encoded.append((a << 4) | b)
                if ((count + 1) // 2) & 1:
                    encoded.append(0)
        encoded.extend((0, 0))
    encoded.extend((0, 1))

    pixel_offset = 14 + 40 + 16 * 4
    file_size = pixel_offset + len(encoded)
    with open(path, "wb") as handle:
        handle.write(struct.pack("<2sIHHI", b"BM", file_size, 0, 0, pixel_offset))
        handle.write(
            struct.pack(
                "<IiiHHIIiiII",
                40,
                width,
                height,
                1,
                4,
                2,
                len(encoded),
                2835,
                2835,
                16,
                16,
            )
        )
        for index in range(16):
            r, g, b = palette[index * 3 : index * 3 + 3]
            handle.write(bytes((b, g, r, 0)))
        handle.write(encoded)


def flame_assets():
    """Regenerate only the two flame resources consumed by ibt.rc."""
    source_path = os.path.join(RES, "flame_source.png")
    source = Image.open(source_path).convert("RGB")
    if source.size != (53 * 8, 50):
        raise RuntimeError("flame_source.png must be 424x50")
    rle_path = os.path.join(RES, "flame4-rle.bmp")
    raw_path = os.path.join(RES, "flame4.bmp")
    save_bmp4_rle(source, rle_path, transparent_key=COL_KEY)
    save_bmp4(source, raw_path, transparent_key=COL_KEY)
    return rle_path, raw_path


def coffee_bmp4():
    """Keep the editable 24-bit source and emit a compact resource copy."""
    source = os.path.join(RES, "coffee.bmp")
    target = os.path.join(RES, "coffee4.bmp")
    with Image.open(source) as image:
        save_bmp4(image.convert("RGB"), target)
    return target


def app_ico():
    master = flame_icon(256)
    path = os.path.join(RES, "app.ico")
    master.save(path, format="ICO", sizes=[(256, 256)], optimize=True)
    return path


if __name__ == "__main__":
    print("icon", app_ico())
    print("flames", *flame_assets())
    print("coffee", coffee_bmp4())
