/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Linus Nielsen Feltzing
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

/* Arm bootloader and startup code based on startup.s from the iPodLinux loader
 *
 * Copyright (c) 2003, Daniel Palffy (dpalffy (at) rainstorm.org)
 * Copyright (c) 2005, Bernard Leach <leachbj@bouncycastle.org>
 *
 */

#include "config.h"
#include "cpu.h"

    .section .init.text,"ax",%progbits

    .global    start

/* Telechips firmware files start with a 32-byte header, as part of the code. */

start:
#ifdef TCCBOOT
    /* Add -DTCCBOOT to EXTRA_DEFINES in the bootloader Makefile to
       enable building the bootloader to be appended to the end of the
       original firmware, dual-booting based on a key-press.

       NB: On the D2 TCCBOOT currently only works in USB boot mode (via tcctool)
       When flashed to the device, the OF will boot as normal - but holding a
       key to boot Rockbox results in a blank screen and crashed player.

       The following two values are filled in by mktccboot.
     */
    .word  0              /* Saved entrypoint of original firmware*/
    .word  0              /* Location in RAM of the start of our bootloader */
#else
    ldr    pc, =start_loc    /* jump to the main entry point  */

    .word  0xffff0601        /* Unknown magic */
    .word  0x3a726556        /* "Ver:" */
    .word  0x31373030        /* "0071" */
    .word  0                 /* First CRC32 */
    .word  0                 /* Unknown - always 0 */
    .word  0                 /* Second CRC32 */
    .word  0                 /* length of firmware file */

#ifdef COWON_D2
    /* Some original firmwares have 0x40 bytes of zeroes here - we
       don't know why, but err on the side of caution and include it
       here. */
    .space 0x40
#endif
#endif

start_loc:

#ifdef BOOTLOADER
#ifdef TCCBOOT
#ifdef COWON_D2
    ldr    r0, =0xf005a000
    ldr    r0, [r0, #0x40] /* Read GPIO B */
    tst    r0, #0x4
    ldreq  pc, [pc, #-28]  /* Jump to original firmware if keypad not pressed */
#else
    #error No bootup key detection implemented for this target
#endif

    /* Copy bootloader to safe area - 0x21000000 (DRAM) */
    /* TODO: Adjust this for other targets - DRAM + DRAMSIZE - 0x100000 */
    ldr    r0, [pc, #-28]
    mov    r1, #0x22000000
    sub    r1, r1, #0x100000
    ldr    r2, =_dataend
1:
    cmp    r2, r1
    ldrhi  r3, [r0], #4
    strhi  r3, [r1], #4
    bhi    1b

    ldr    pc, =copied_start   /* jump to the relocated start_loc:  */

copied_start:
#endif
#else
    /* We don't use interrupts in the bootloader */

    /* Set up stack for IRQ mode */ 
    mov    r0,#0xd2
    msr    cpsr, r0
    ldr    sp, =irq_stack
    /* Set up stack for FIQ mode */ 
    mov    r0,#0xd1
    msr    cpsr, r0
    ldr    sp, =fiq_stack

    /* Let abort and undefined modes use IRQ stack */
    mov    r0,#0xd7
    msr    cpsr, r0
    ldr    sp, =irq_stack
    mov    r0,#0xdb
    msr    cpsr, r0
    ldr    sp, =irq_stack
#endif

    /* Switch to supervisor mode */
    mov    r0,#0xd3
    msr    cpsr, r0
    ldr    sp, =stackend


#if !defined(BOOTLOADER) && !defined(STUB)

    /* Copy exception handler code to address 0 */
    ldr    r2, =_vectorsstart
    ldr    r3, =_vectorsend
    ldr    r4, =_vectorscopy
1:
    cmp    r3, r2
    ldrhi  r5, [r4], #4
    strhi  r5, [r2], #4
    bhi    1b
    
    /* Zero out IBSS */
    ldr    r2, =_iedata
    ldr    r3, =_iend
    mov    r4, #0
1:
    cmp    r3, r2
    strhi  r4, [r2], #4
    bhi    1b

    /* Copy the ITCM */
    ldr    r2, =_itcmcopy
    ldr    r3, =_itcmstart
    ldr    r4, =_itcmend
1:
    cmp    r4, r3
    ldrhi  r5, [r2], #4
    strhi  r5, [r3], #4
    bhi    1b

    /* Copy the DTCM */
    ldr    r2, =_dtcmcopy
    ldr    r3, =_dtcmstart
    ldr    r4, =_dtcmend
1:
    cmp    r4, r3
    ldrhi  r5, [r2], #4
    strhi  r5, [r3], #4
    bhi    1b
#endif /* !BOOTLOADER,!STUB */

    /* Initialise bss section to zero */
    ldr    r2, =_edata
    ldr    r3, =_end
    mov    r4, #0
1:
    cmp    r3, r2
    strhi  r4, [r2], #4
    bhi    1b

    /* Set up some stack and munge it with 0xdeadbeef */
    ldr    sp, =stackend
    mov    r3, sp
    ldr    r2, =stackbegin
    ldr    r4, =0xdeadbeef
1:
    cmp    r3, r2
    strhi  r4, [r2], #4
    bhi    1b

    /*
       Enable cache & TCM regions
       TODO: This is just doing what the OF does at present. It needs to be
       better understood and moved out to a separate MMU functions package.
    */
    ldr     r1, =0x1fe0c
    mov     r0, #0xf7000000
    str     r1, [r0]
    ldr     r1, =0x2801ae24
    str     r1, [r0,#4]
    ldr     r1, =0x13e44
    str     r1, [r0,#8]
    ldr     r1, =0x4001ce60
    str     r1, [r0,#0xc]
    ldr     r1, =0x6001be80
    str     r1, [r0,#0x10]
    ldr     r1, =0x3801aea4
    str     r1, [r0,#0x14]
    ldr     r1, =0x8001eec0
    str     r1, [r0,#0x18]
    ldr     r1, =0x1001aee0
    str     r1, [r0,#0x1c]
    add     r1, r0, #0x8000  /* r1 now = 0xf7008000 */
    ldr     r0, =0xa0000011
    ldr     r2, =0x5507d
    mcr     p15, 0, r0,c9,c1 /* data tcm region (enabled; 8kb; 0xa0000000) */
    mov     r0, #0xd
    mcr     p15, 0, r0,c9,c1, 1 /* inst tcm region (enabled, 4kb, 0x00000000) */
    ldr     r0, =0x55555555
    mcr     p15, 0, r1,c2,c0 /* translation table base register = 0xf7008000 */
    mcr     p15, 0, r0,c3,c0 /* domain access d0-d15 = 'client' */
    mov     r0, #0
    mcr     p15, 0, r0,c7,c5 /* invalidate icache */
    mcr     p15, 0, r2,c1,c0 /* enable mmu, i & d caches */
    mcr     p15, 0, r0,c7,c6 /* invalidate dcache */
    mcr     p15, 0, r1,c8,c7 /* invalidate tlb */
    
    bl     main
    /* main() should never return */

#ifndef BOOTLOADER

/* Exception handlers. Will be copied to address 0 after memory remapping */
    .section .vectors,"aw"
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]
    ldr    pc, [pc, #24]

    /* Exception vectors */
    .global vectors
vectors:
    .word  start
    .word  undef_instr_handler
    .word  software_int_handler
    .word  prefetch_abort_handler
    .word  data_abort_handler
    .word  reserved_handler
    .word  irq_handler
    .word  fiq_handler

    .text

#if !defined(STUB)
    .global irq
    .global fiq
    .global UIE
#endif

/* All illegal exceptions call into UIE with exception address as first
   parameter. This is calculated differently depending on which exception
   we're in. Second parameter is exception number, used for a string lookup
   in UIE.
 */
undef_instr_handler:
    mov    r0, lr
    mov    r1, #0
    b      UIE

/* We run supervisor mode most of the time, and should never see a software
   exception being thrown. Perhaps make it illegal and call UIE?
 */
software_int_handler:
reserved_handler:
    movs   pc, lr

prefetch_abort_handler:
    sub    r0, lr, #4
    mov    r1, #1
    b      UIE

data_abort_handler:
    sub    r0, lr, #8
    mov    r1, #2
    b      UIE

#if defined(STUB)
UIE:
    b UIE
#endif

    /* We don't use interrupts in the bootloader */

/* Align stacks to cache line boundary */
    .balign 16
    
/* 256 words of IRQ stack */
    .space 256*4
irq_stack:

/* 256 words of FIQ stack */
    .space 256*4
fiq_stack:

#endif