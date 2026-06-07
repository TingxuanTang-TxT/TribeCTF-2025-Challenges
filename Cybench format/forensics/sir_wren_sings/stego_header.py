#!/usr/bin/env python3
import argparse
import binascii
import os
import struct
import sys

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
PNG_IHDR = b'IHDR'
PNG_IEND = b'IEND'
CUSTOM_CHUNK = b'sTEG'  # ancillary (lowercase first letter)

def insert_png_chunk(in_bytes: bytes, chunk_type: bytes, data: bytes, place_after=b'IHDR') -> bytes:
    if not in_bytes.startswith(PNG_SIGNATURE):
        raise ValueError("Not a PNG file")
    # Iterate chunks: [length(4)][type(4)][data(N)][crc(4)]
    pos = 8
    chunks = []
    ihdr_inserted = False
    while pos < len(in_bytes):
        if pos + 8 > len(in_bytes):
            raise ValueError("Corrupt PNG: truncated chunk header")
        length = struct.unpack(">I", in_bytes[pos:pos+4])
        ctype = in_bytes[pos+4:pos+8]
        start = pos
        data_start = pos + 8
        data_end = data_start + length
        crc_end = data_end + 4
        if crc_end > len(in_bytes):
            raise ValueError("Corrupt PNG: truncated chunk data")
        chunk_bytes = in_bytes[start:crc_end]
        pos = crc_end
        chunks.append((ctype, chunk_bytes))

    # Build new chunk
    crc_input = chunk_type + data
    crc = binascii.crc32(crc_input) & 0xffffffff
    new_chunk = struct.pack(">I", len(data)) + chunk_type + data + struct.pack(">I", crc)

    # Reassemble, inserting after the specified chunk (default IHDR), or before IEND if not found
    out = bytearray(PNG_SIGNATURE)
    inserted = False
    for ctype, chunk_bytes in chunks:
        out += chunk_bytes
        if not inserted and ctype == place_after:
            out += new_chunk
            inserted = True
    if not inserted:
        # Insert before IEND as a fallback
        out = bytearray(PNG_SIGNATURE)
        for ctype, chunk_bytes in chunks:
            if ctype == PNG_IEND and not inserted:
                out += new_chunk
                inserted = True
            out += chunk_bytes
    return bytes(out)

def embed_png(in_path: str, out_path: str, message: bytes) -> None:
    with open(in_path, "rb") as f:
        data = f.read()
    out = insert_png_chunk(data, CUSTOM_CHUNK, message, place_after=PNG_IHDR)
    with open(out_path, "wb") as f:
        f.write(out)

# ---- JPEG COM insertion ----
# JPEG structure: SOI(FFD8), then a sequence of segments/markers.
# We insert a COM (FFFE) before SOS (FFDA).
def insert_jpeg_com(in_bytes: bytes, comment: bytes) -> bytes:
    if len(in_bytes) < 4 or in_bytes[0:2] != b'\xff\xd8':
        raise ValueError("Not a JPEG (missing SOI)")
    # Build COM segment: 0xFFFE + length(2 bytes, includes length itself) + data
    if len(comment) > 0xFFFD:  # 65533 max comment length (length field includes itself)
        raise ValueError("Comment too long for a single COM segment")
    seg = b'\xff\xfe' + struct.pack(">H", len(comment) + 2) + comment

    pos = 2  # after SOI
    out = bytearray()
    out += in_bytes[0:2]

    # Walk segments until SOS (FFDA), insert COM just before SOS
    while pos < len(in_bytes):
        if pos + 2 > len(in_bytes):
            raise ValueError("Truncated JPEG")
        if in_bytes[pos] != 0xFF:
            # We are likely in compressed scan data (shouldn't happen before SOS)
            raise ValueError("Unexpected data before SOS")
        marker = in_bytes[pos:pos+2]
        pos += 2
        if marker == b'\xff\xda':  # SOS
            # Insert COM before SOS
            out += seg
            # Append SOS and the rest of the file (scan data until EOI)
            out += b'\xff\xda'
            out += in_bytes[pos:]
            return bytes(out)
        elif marker in (b'\xff\xd9',):  # EOI encountered unexpectedly before SOS
            # Insert COM before EOI
            out += seg
            out += b'\xff\xd9'
            out += in_bytes[pos:]
            return bytes(out)
        else:
            # Most markers have a 2-byte length and payload; some standalones exist but are rare here
            if pos + 2 > len(in_bytes):
                raise ValueError("Truncated JPEG segment length")
            seg_len = struct.unpack(">H", in_bytes[pos:pos+2])[0]
            end = pos + seg_len
            if end > len(in_bytes):
                raise ValueError("Truncated JPEG segment data")
            # Copy this full segment (length includes the 2-byte length itself)
            out += marker + in_bytes[pos: end]
            pos = end

    # If we never found SOS, just insert before EOI if present, else append
    if in_bytes[-2:] == b'\xff\xd9':
        return in_bytes[:-2] + seg + b'\xff\xd9'
    return in_bytes + seg

def embed_jpeg(in_path: str, out_path: str, message: bytes) -> None:
    with open(in_path, "rb") as f:
        data = f.read()
    out = insert_jpeg_com(data, message)
    with open(out_path, "wb") as f:
        f.write(out)

def main():
    p = argparse.ArgumentParser(description="Embed a message in an image header region (PNG custom chunk or JPEG COM).")
    p.add_argument("input", help="Input image file (.png or .jpg/.jpeg)")
    p.add_argument("output", help="Output image file")
    p.add_argument("-m", "--message", help="Message text to embed")
    p.add_argument("-f", "--file", help="Read message bytes from file instead of -m")
    args = p.parse_args()

    if not os.path.isfile(args.input):
        print("Input file not found", file=sys.stderr)
        sys.exit(1)

    if args.file:
        with open(args.file, "rb") as f:
            msg = f.read()
    elif args.message is not None:
        msg = args.message.encode("utf-8")
    else:
        print("Provide -m MESSAGE or -f FILE", file=sys.stderr)
        sys.exit(1)

    with open(args.input, "rb") as f:
        header = f.read(8)

    ext = os.path.splitext(args.input)[1].lower()
    try:
        if header.startswith(PNG_SIGNATURE) or ext == ".png":
            embed_png(args.input, args.output, msg)
            print(f"Embedded {len(msg)} bytes into PNG custom chunk sTEG and wrote {args.output}")
        else:
            # Assume JPEG if it starts with SOI or extension matches
            with open(args.input, "rb") as f:
                soi = f.read(2)
            if soi == b'\xff\xd8' or ext in (".jpg", ".jpeg"):
                embed_jpeg(args.input, args.output, msg)
                print(f"Embedded {len(msg)} bytes into JPEG COM segment and wrote {args.output}")
            else:
                print("Unsupported format. Only PNG and JPEG are supported.", file=sys.stderr)
                sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
