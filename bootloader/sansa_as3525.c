/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2008 by Rafaël Carré
 *
 * Based on Rockbox iriver bootloader by Linus Nielsen Feltzing
 * and the ipodlinux bootloader by Daniel Palffy and Bernard Leach
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <stdio.h>
#include <system.h>
#include <inttypes.h>
#include "config.h"
#include "lcd.h"
#include "backlight-target.h"
#include "button-target.h"
#include "common.h"
#include "storage.h"
#include "disk.h"
#include "panic.h"
#include "power.h"

#include "power.h"

#include "ascodec-target.h"

void morpion(void);
int show_logo(void);
void main(void) __attribute__((noreturn));
void main(void)
{
    unsigned char* loadbuffer;
    int buffer_size;
    void(*kernel_entry)(void);
    int ret;
    int delay;

    //system_init();
    kernel_init();
    ascodec_init();
    ascodec_write(AS3514_DCDC15, 0x10);
    GPIOA_DIR |= (1<<4);
    GPIOA_PIN(4) = (1<<4);
    delay=0x5000000;
    while(delay--);
    power_off();
    while(1) ;

#ifdef SANSA_C200V2
    /* stop here */
    while(1);
#endif

    lcd_init();
    show_logo();

    _backlight_on();

    button_init_device();
#if !(defined(SANSA_CLIPV2) || defined(SANSA_FUZEV2))
    int btn = button_read_device();

#if !defined(SANSA_FUZE) && !defined(SANSA_CLIP)
    if (button_hold())
    {
        verbose = true;
        lcd_clear_display();
        printf("Hold switch on");
        printf("Shutting down...");
        sleep(HZ);
        power_off();
    }
#endif

    /* Enable bootloader messages if any button is pressed */
    if (btn)
    {
        lcd_clear_display();
        verbose = true;
    }
#else
    verbose = true;

    enable_irq();

    lcd_puts(0, 0, "SELECT = tic tac toe");
    lcd_puts(0, 1, "OTHER  = continue");
    lcd_update();

    int btn;
    while(!(btn = button_read_device())) ;
    lcd_clear_display();
    if(btn & BUTTON_SELECT)
    {
        lcd_update();
        sleep(HZ/2);
        morpion();
        power_off();
    }
    else
        show_logo();

    while(button_read_device());
#endif

    ret = storage_init();
    if(ret < 0)
        error(EATA,ret);

    if(!disk_init(IF_MV(0)))
        panicf("disk_init() failed!");

    ret = disk_mount_all();

    if(ret <= 0)
        error(EDISK, ret);

    printf("Loading firmware");

    loadbuffer = (unsigned char*)DRAM_ORIG; /* DRAM */
    buffer_size = (int)(loadbuffer + (DRAM_SIZE) - TTB_SIZE);

    ret = load_firmware(loadbuffer, BOOTFILE, buffer_size);
    if(ret < 0)
        error(EBOOTFILE, ret);

    disable_irq(); /* disable irq until we have copied the new vectors */

    if (ret == EOK)
    {
        kernel_entry = (void*) loadbuffer;
        printf("Executing");
        kernel_entry();
        printf("ERR: Failed to boot");
    }

    /* never returns */
    while(1) ;
}
