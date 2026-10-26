# STM32F411 Blackpill with external W25Q32JV

This target copies `stm32f411ce_blackpill` and adds raw resource UF2 programming
for an external NOR connected to SPI2. The original board target is unchanged.
The copied CMake configuration also corrects the original target's F401 startup
selection to F411xE, matching its Make configuration.

## Wiring

| W25Q32JV SOIC-8 | STM32F411 |
| --- | --- |
| 1 /CS | PB8, external pull-up |
| 2 DO / IO1 | PB14, SPI2 MISO |
| 3 /WP / IO2 | Pull up to 3.3 V |
| 4 GND | GND |
| 5 DI / IO0 | PB15, SPI2 MOSI |
| 6 CLK | PB13, SPI2 SCK |
| 7 /HOLD / IO3 | Pull up to 3.3 V |
| 8 VCC | 3.3 V with local decoupling |

The Blackpill's official SPI1 Flash footprint is not used. SPI1 pins
PA4/PA5/PA7 remain available. SPI2 runs in mode 0 at 5.25 MHz with the copied
84 MHz system clock. The driver requires JEDEC ID `EF 40 16` and refuses
protected chips without changing their protection or QE settings.

Commands, identification, page sizes, erase sizes and timing are based on the
[Winbond W25Q32JV datasheet, Revision G](https://www.winbond.com/resource-files/w25q32jv%20revg%2003272018%20plus.pdf).
Erase timeout is 500 ms and page-program timeout is 10 ms. Cortex-M4 DWT timing
is used because this port's SysTick does not advance HAL ticks.

## Images and update behavior

| Destination | UF2 addresses | Family |
| --- | --- | --- |
| Internal application | `0x08010000` through `0x0807FFFF` | `0x57755A57` |
| External NOR | `0x90000000` through `0x903FFFFF` | `0x57755A57` |

The NOR range is a UF2 protocol address, not CPU mapped memory. NOR offset zero
is `0x90000000`. Internal Flash size, application start, bootloader write
protection and `CURRENT.UF2` retain their internal-Flash meanings; `CURRENT.UF2`
does not export the NOR. The bootloader fits the existing 31 KiB code region.

Create a resource-only image from a raw binary, from the repository root:

```powershell
python tools/nor_uf2.py animation.bin animation-nor.uf2
# Optional placement at a different 4 KiB boundary:
python tools/nor_uf2.py animation.bin animation-nor.uf2 --offset 0x1000
```

Enter TinyUF2 and copy the resulting file to `STMF411BOOT`. The generator pads
the last 256-byte page with `0xFF` and prints the original length and CRC32.
Only sectors touched by the image are erased, once per transfer. The last
sector's bytes beyond the image are also erased; do not put unrelated data in
that sector. Sectors outside the image remain untouched.

Each transfer accepts one contiguous image of 256-byte UF2 payloads with a
consistent count and address-to-block-number mapping. NOR starts must align to
4 KiB. Blocks may arrive out of order; identical duplicates do not erase or
count twice, and conflicting NOR duplicates fail. Mixed internal/NOR images,
address overflow, bootloader addresses, malformed lengths and failed writes
abort the transfer. Each programmed page is read back before it is counted.
On failure the NOR target reports a failed MSC write and suppresses completion.
Reset into the bootloader before retrying or uploading another image.

This is raw NOR programming, without an animation format, persistent validity
marker, atomic commit or A/B storage. Power loss can leave partial resources.
The application must validate its resource header, expected length/version and
CRC before use and fall back when invalid. The printed CRC is for inspection;
the bootloader does not store it as a manifest. NOR absence does not prevent an
internal application update. A combined firmware/resource UF2 is not supported.
Installing the bootloader does not itself migrate application resources.

## Windows source, robocopy, WSL build

Make source edits in the Windows repository and sync to a WSL build tree with
robocopy. A machine-specific helper may be kept in the ignored `tmp/` directory;
it is local tooling and is not distributed with the repository. If present,
run it from the repository root:

```powershell
.\tmp\build_stm32f4_wsl.ps1 -DryRun
.\tmp\build_stm32f4_wsl.ps1
```

Keep local paths and distro settings in that ignored script. Use `robocopy /E`,
exclude `.git`, editor metadata, `tmp`, `_build` and `_bin`, and preserve WSL-only
dependencies. Robocopy exit codes below 8 are successful. Following the root
README's Make build, enter `ports/stm32f4` within the WSL build tree and run:

```sh
make BOARD=stm32f411ce_blackpill_nor -j4 all
```

The ELF, map, BIN and HEX are copied back with robocopy to
`tmp/stm32f4/stm32f411ce_blackpill_nor/`. WSL needs the dependencies described by
the root README and `arm-none-eabi-gcc`, Make and Python 3 installed.
Install the bootloader BIN at `0x08000000` using SWD or ROM DFU; the resource UF2
belongs on the TinyUF2 volume after this bootloader has been installed.

Parser regression tests, from the WSL build-tree root:

```sh
mkdir -p tmp/nor-tests
gcc -std=gnu11 -Wall -Wextra -Werror -Wno-pointer-to-int-cast \
  -Iports/stm32f4/tests/uf2_nor -Isrc \
  src/ghostfat.c ports/stm32f4/tests/uf2_nor/main.c -o tmp/nor-tests/parser
tmp/nor-tests/parser
```

Hardware acceptance still requires JEDEC detection, a resource write/readback,
power-cycle retention, interrupted transfers, and an internal firmware update
with the actual board. Host parser tests cannot verify SPI wiring or timing.

Validation on 2026-10-26, using Ubuntu-22.04 and Arm GCC 10.3.1:

- Make NOR target: passed, 17,732-byte BIN, 11,352 bytes allocated RAM.
- CMake `tinyuf2` target: passed, 25,016 bytes Flash, within the 31 KiB region.
- Original `stm32f411ce_blackpill` Make target: passed.
- Host parser tests above: passed.
- Generator roundtrip: passed for 1/255/256/257/4096 bytes and the full 4 MiB;
  empty inputs, invalid offsets and capacity overflow were rejected.
- `git -c diff.ignoreSubmodules=all diff --check`: passed.

Make and CMake use different shared build flags, so their sizes differ. The
Windows artifact directory above contains the Make build. No board was flashed
and no hardware test was performed.
