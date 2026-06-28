from PIL import Image
import sys
import os

def convert(path, mode, w_out=None, h_out=None):
    img = Image.open(path).convert("RGBA")
    if w_out and h_out:
        img = img.resize((w_out, h_out))
    w, h = img.size
    pixels = list(img.get_flattened_data())
    name = os.path.splitext(os.path.basename(path))[0]

    if mode == "argb1555":
        values = []
        for r, g, b, a in pixels:
            if a < 128:
                values.append("0x0000")
            else:
                v = 0x8000 | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)
                values.append(f"0x{v:04X}")

        print(f"// {w}x{h} ARGB1555")
        print(f"static const uint16_t {name}[{len(values)}] = {{")
        print(", ".join(values))
        print("};")

    elif mode == "argb8565":
        values = []
        for r, g, b, a in pixels:
            hi = (r & 0xF8) | (g >> 5)
            lo = ((g & 0x1C) << 3) | (b >> 3)
            values.append(f"0x{a:02X}, 0x{hi:02X}, 0x{lo:02X}")

        print(f"// {w}x{h} ARGB8565")
        print(f"static const uint8_t {name}[{w * h * 3}] = {{")
        print(", ".join(values))
        print("};")

if __name__ == "__main__":
    w = int(sys.argv[3]) if len(sys.argv) > 3 else None
    h = int(sys.argv[4]) if len(sys.argv) > 4 else None
    convert(sys.argv[1], sys.argv[2], w, h)
