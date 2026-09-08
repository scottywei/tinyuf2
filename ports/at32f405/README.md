# TinyUF2 for AT32F405

TinyUF2 reserves 32 KB, therefore applications should start at `0x08008000`.
Applications must also set `SCB->VTOR` to `0x08008000` before enabling
interrupts and reserve the top 8 bytes of SRAM for double-reset detection.

To create a UF2 image, use family ID `0x6b1f405c` as follows:

From hex:

```sh
uf2conv.py -c -f 0x6b1f405c firmware.hex
```

From bin:

```sh
uf2conv.py -c -b 0x08008000 -f 0x6b1f405c firmware.bin
```

## Supported Boards

- `weact_at32f405`: WeAct Studio AT32F405CxTx Core Board V1.0
