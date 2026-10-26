#!/usr/bin/env python3
"""Package a raw binary for the Blackpill SPI2 W25Q32JV target."""
import argparse
from pathlib import Path
import struct
import zlib

NOR_BASE = 0x90000000
NOR_SIZE = 4 * 1024 * 1024
SECTOR_SIZE = 4096
PAGE_SIZE = 256
FAMILY_ID = 0x57755A57


def convert(data: bytes, offset: int = 0) -> bytes:
    if offset < 0 or offset >= NOR_SIZE or offset % SECTOR_SIZE:
        raise ValueError("offset must be a 4 KiB boundary within the 4 MiB NOR")
    if not data or len(data) > NOR_SIZE - offset:
        raise ValueError("input must be nonempty and fit within the NOR")
    count = (len(data) + PAGE_SIZE - 1) // PAGE_SIZE
    blocks = []
    for index in range(count):
        header = struct.pack(
            "<8I", 0x0A324655, 0x9E5D5157, 0x2000,
            NOR_BASE + offset + index * PAGE_SIZE, PAGE_SIZE,
            index, count, FAMILY_ID,
        )
        payload = data[index * PAGE_SIZE:(index + 1) * PAGE_SIZE]
        blocks.append(header + payload.ljust(PAGE_SIZE, b"\xff") +
                      bytes(476 - PAGE_SIZE) + struct.pack("<I", 0x0AB16F30))
    return b"".join(blocks)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--offset", type=lambda value: int(value, 0), default=0,
                        help="NOR byte offset, aligned to 4096 (default: 0)")
    args = parser.parse_args()
    if args.input.resolve() == args.output.resolve():
        parser.error("input and output paths must differ")
    data = args.input.read_bytes()
    try:
        uf2 = convert(data, args.offset)
    except ValueError as error:
        parser.error(str(error))
    args.output.write_bytes(uf2)
    print(f"{len(data)} bytes -> {len(uf2) // 512} UF2 blocks; "
          f"address=0x{NOR_BASE + args.offset:08X}; CRC32=0x{zlib.crc32(data):08X}")
    print("Each touched 4 KiB sector is erased. Reset into the bootloader for each image.")


if __name__ == "__main__":
    main()
