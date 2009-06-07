#include "config.h"
#include "cpu.h"

ENTRY(start)
OUTPUT_FORMAT(elf32-littlearm)
OUTPUT_ARCH(arm)
STARTUP(target/arm/crt0.o)

/*
No need for DRAM in our bootloader
#define DRAMSIZE (MEMORYSIZE * 0x100000) - TTB_SIZE
#define DRAMORIG 0x30000000
*/
#if defined(SANSA_CLIPV2) || defined(SANSA_FUZE)
#define IRAMORIG 0              /* in fact DRAM, but we don't care */
#define IRAMSIZE (MEM*0x100000)
#else
#define IRAMORIG 0x81000000
#define IRAMSIZE 0x50000
#endif

MEMORY
{
    /*DRAM : ORIGIN = DRAMORIG, LENGTH = DRAMSIZE*/
    IRAM : ORIGIN = IRAMORIG, LENGTH = IRAMSIZE
}

SECTIONS
{
  . = IRAMORIG;

  .text : {
    *(.init.text)
    *(.glue_7)
    *(.glue_7t)
    *(.text*)
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

  .stack :
  {
     *(.stack)
     _stackbegin = .;
     stackbegin = .;
     . += 0x2000;
     _stackend = .;
     stackend = .;
  } > IRAM

  .bss : {
     _edata = .;
     *(.bss*);
     *(.ibss);
     *(COMMON)
     *(.ncbss*);
     _end = .;
   } > IRAM
}
