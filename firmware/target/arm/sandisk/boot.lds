#include "config.h"

ENTRY(start)
OUTPUT_FORMAT(elf32-littlearm)
OUTPUT_ARCH(arm)
STARTUP(target/arm/crt0-pp-bl.o)

#define DRAMSIZE (MEMORYSIZE * 0x100000)

#define DRAMORIG 0x10000000
#ifndef IRAMORIG
#define IRAMORIG 0x40000000
#endif
#define IRAMSIZE 0x20000
#define FLASHORIG 0x001f0000
#define FLASHSIZE 2M

MEMORY
{
    DRAM : ORIGIN = DRAMORIG, LENGTH = DRAMSIZE
    IRAM : ORIGIN = IRAMORIG, LENGTH = IRAMSIZE
}

SECTIONS
{
#ifdef C200_ERASE
  . = IRAMORIG+0x4000;
#else
  . = IRAMORIG;
#endif

  .text : {
    *(.init.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
  } > IRAM

  .data : {
    *(.icode)
    *(.irodata)
    *(.idata)
    *(.data*)
    *(.ncdata*)
    *(.rodata*)
    _dataend = . ;
  } > IRAM

  .stack : {
     *(.stack)
     _stackbegin = .;
     stackbegin = .;
     . += 0x2000;
     _stackend = .;
     stackend = .;
  } > IRAM

  /* The bss section is too large for IRAM - we just move it 16MB into the
     DRAM */

  . = DRAMORIG;
  .bss  . + (16*1024*1024) : {
     _edata = .;
     *(.bss*);
     *(.ibss);
     *(COMMON)
     *(.ncbss*);
     _end = .;
  } > DRAM
}
