# TinyUF2 for STM32F103CBT6 with 16 MHz HSE

This port targets the 128 KiB STM32F103CBT6 in an LQFP48 package. It reserves
the first 16 KiB for TinyUF2, so applications must link their vector table at
`0x08004000`. The remaining application region is 112 KiB.

The clock tree is fixed to the hardware requested by this board definition:

- HSE: 16 MHz crystal (not bypass clock)
- PLL input: HSE / 2 = 8 MHz
- SYSCLK: 8 MHz x 9 = 72 MHz
- USB clock: PLL / 1.5 = 48 MHz
- APB1: 36 MHz; APB2 and AHB: 72 MHz

PA11 is USB D- and PA12 is USB D+. STM32F103 requires a 1.5 kohm USB pull-up on
D+; most development boards already include it. The generic board definition
uses the BIOI CAP45 Caps Lock LED on PA3, active high, as its status LED.

## Build

Fetch the two STM32 dependencies once:

```sh
python tools/get_deps.py stm32f1
```

Then build with either Make or CMake:

```sh
cd ports/stm32f1
make BOARD=stm32f103cb_16mhz all
```

```sh
cmake -S ports/stm32f1 -B ports/stm32f1/_build/cmake \
  -DBOARD=stm32f103cb_16mhz -DTOOLCHAIN=gcc
cmake --build ports/stm32f1/_build/cmake
```

Flash `tinyuf2-stm32f103cb_16mhz.bin` at `0x08000000` using SWD. With a valid
application installed, double-tap NRST within 500 ms to enter TinyUF2. If the
application vector table is invalid, TinyUF2 starts automatically.

Create application UF2 files with the standard STM32F1 family ID:

```sh
python lib/uf2/utils/uf2conv.py -c -b 0x08004000 -f STM32F1 firmware.bin
```

The default `0xCAFE:0xF103` USB VID/PID is for development only and must be
replaced with an allocated pair before product distribution. Bootloader option
byte write protection and self-update are intentionally not enabled in this
initial port.
