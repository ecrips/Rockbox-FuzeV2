/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2007 by Karl Kurbjun
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
 
#include "inttypes.h"
#include "string.h"
#include "cpu.h"
#include "system.h"
#include "lcd.h"
#include "kernel.h"
#include "thread.h"
#include "storage.h"
#include "fat.h"
#include "disk.h"
#include "font.h"
#include "adc.h"
#include "backlight.h"
#include "backlight-target.h"
#include "button.h"
#include "panic.h"
#include "power.h"
#include "file.h"
#include "common.h"
#include "rbunicode.h"
#include "usb.h"
#include "spi.h"
#include "uart-target.h"
#include "tsc2100.h"
#include "time.h"
#include "system-arm.h"

void main(void)
{
    unsigned char* loadbuffer;
    int buffer_size;
    int rc;
    int(*kernel_entry)(void);

    /* Make sure interrupts are disabled */
    set_irq_level(IRQ_DISABLED);
    set_fiq_status(FIQ_DISABLED);
    system_init();
    kernel_init();
    
    /* Now enable interrupts */
    set_irq_level(IRQ_ENABLED);
    set_fiq_status(FIQ_ENABLED);

    backlight_init();
    lcd_init();
    font_init();
    button_init();
    usb_init();
    
    
    power_init();
//    enable_irq();
//    enable_fiq();

    adc_init();

    lcd_setfont(FONT_SYSFIXED);

    /* Show debug messages if button is pressed */
//    if(button_read_device())
        verbose = true;

    printf("Rockbox boot loader");
    printf("Version %s", APPSVERSION);

    /* Enter USB mode without USB thread */
    if(usb_detect() == USB_INSERTED)
    {
        const char msg[] = "Bootloader USB mode";
        reset_screen();
        lcd_putsxy( (LCD_WIDTH - (SYSFONT_WIDTH * strlen(msg))) / 2,
                    (LCD_HEIGHT - SYSFONT_HEIGHT) / 2, msg);
        lcd_update();

        ide_power_enable(true);
        storage_enable(false);
        sleep(HZ/20);
        usb_enable(true);

        while (usb_detect() == USB_INSERTED)
        {
            storage_spin(); /* Prevent the drive from spinning down */
            sleep(HZ);
        }

        usb_enable(false);

        reset_screen();
        lcd_update();
    }
    
    sleep(50);

    printf("ATA");
    rc = storage_init();
    if(rc)
    {
        reset_screen();
        error(EATA, rc);
    }

    printf("disk");
    disk_init();

    printf("mount");
    rc = disk_mount_all();
    if (rc<=0)
    {
        error(EDISK,rc);
    }

    printf("Loading firmware");

    loadbuffer = (unsigned char*) 0x00900000;
    buffer_size = (unsigned char*)0x01900000 - loadbuffer;

    rc = load_firmware(loadbuffer, BOOTFILE, buffer_size);
    if(rc < 0)
        error(EBOOTFILE, rc);

    if (rc == EOK)
    {
        kernel_entry = (void*) loadbuffer;
        rc = kernel_entry();
    }
}
