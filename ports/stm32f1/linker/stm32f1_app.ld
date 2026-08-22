ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08004000, LENGTH = 112K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K - 4
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_board_dfu_dbl_tap = _estack;

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text : { *(.text .text*) *(.rodata .rodata*) KEEP(*(.init)) KEEP(*(.fini)) _etext = .; } > FLASH
  .ARM.extab : { *(.ARM.extab*) } > FLASH
  .ARM : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH
  .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } > FLASH
  .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); } > FLASH
  .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*)) PROVIDE_HIDDEN(__fini_array_end = .); } > FLASH
  _sidata = LOADADDR(.data);
  .data : { _sdata = .; *(.data .data*) _edata = .; } > RAM AT > FLASH
  .bss (NOLOAD) : { _sbss = .; __bss_start__ = .; *(.bss .bss*) *(COMMON) _ebss = .; __bss_end__ = .; } > RAM
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
