/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2008 by Maurus Cuelenaere
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

/*
 * This config file is for the Onda VX747(+)
 */

#define TARGET_TREE /* this target is using the target tree system */

#define CONFIG_SDRAM_START 0x80004000

#ifdef ONDA_VX747P
#define MODEL_NAME  "Onda VX747+"
#define MODEL_NUMBER 44
/* Define something for camera interface... */
#else
#define MODEL_NAME  "Onda VX747"
#define MODEL_NUMBER 45
#endif

//#define HAVE_ATA_SD
//#define HAVE_HOTSWAP

//#define CONFIG_STORAGE (STORAGE_NAND | STORAGE_SD)
#define CONFIG_STORAGE STORAGE_SD

/* ChinaChip NAND FTL */
#define CONFIG_NAND NAND_CC

/* define this if you have a bitmap LCD display */
#define HAVE_LCD_BITMAP

/* define this if you have a colour LCD */
#define HAVE_LCD_COLOR

/* define this if you want album art for this target */
#define HAVE_ALBUMART

/* define this to enable bitmap scaling */
#define HAVE_BMP_SCALING

/* define this to enable JPEG decoding */
#define HAVE_JPEG

/* define this if you have access to the quickscreen */
#define HAVE_QUICKSCREEN

/* define this if you have access to the pitchscreen */
#define HAVE_PITCHSCREEN

/* define this if you would like tagcache to build on this target */
#define HAVE_TAGCACHE

/* define this if the target has volume keys which can be used in the lists */
//#define HAVE_VOLUME_IN_LIST

/* LCD dimensions */
#define CONFIG_LCD LCD_ONDAVX747

#define LCD_WIDTH  240
#define LCD_HEIGHT 400

#define LCD_DEPTH  16   /* 16bit colours */
#define LCD_PIXELFORMAT RGB565 /* rgb565 */

/* Define this if your LCD can be enabled/disabled */
#define HAVE_LCD_ENABLE

#define CONFIG_KEYPAD ONDAVX747_PAD
#define HAVE_TOUCHSCREEN
#define HAVE_BUTTON_DATA

/* Define this if you do software codec */
#define CONFIG_CODEC SWCODEC

/* define this if you have a real-time clock */
#define CONFIG_RTC RTC_JZ47XX

/* Define this for LCD backlight available */
#define HAVE_BACKLIGHT
#define HAVE_BACKLIGHT_BRIGHTNESS

/* define this if the backlight can be set to a brightness */
#define __BACKLIGHT_INIT

/* Which backlight fading type? */
//#define CONFIG_BACKLIGHT_FADING BACKLIGHT_FADING_PWM

/* Main LCD backlight brightness range and defaults */
#define MIN_BRIGHTNESS_SETTING          100
#define MAX_BRIGHTNESS_SETTING          300
#define DEFAULT_BRIGHTNESS_SETTING      300 /* "full brightness" */
#define DEFAULT_DIMNESS_SETTING         100 /* "most dim" */

/* Define this if you have a software controlled poweroff */
#define HAVE_SW_POWEROFF

/* The number of bytes reserved for loadable codecs */
#define CODEC_SIZE 0x100000

/* The number of bytes reserved for loadable plugins */
#define PLUGIN_BUFFER_SIZE 0x100000

/* Define this if you have the Jz4740 internal codec */
#define HAVE_JZ4740_CODEC

/* has no tone controls, so we use the software ones */
#define HAVE_SW_TONE_CONTROLS

/* define the bitmask of hardware sample rates */
#define HW_SAMPR_CAPS   (SAMPR_CAP_48 | SAMPR_CAP_44 | SAMPR_CAP_32 | \
                         SAMPR_CAP_24 | SAMPR_CAP_22 | SAMPR_CAP_16 | \
                         SAMPR_CAP_12 | SAMPR_CAP_11 | SAMPR_CAP_8)

#define CONFIG_I2C I2C_JZ47XX

#define NEED_ADC_CLOSE 1

#define BATTERY_CAPACITY_DEFAULT 1250 /* default battery capacity */
#define BATTERY_CAPACITY_MIN 500      /* min. capacity selectable */
#define BATTERY_CAPACITY_MAX 2500     /* max. capacity selectable */
#define BATTERY_CAPACITY_INC 100      /* capacity increment */
#define BATTERY_TYPES_COUNT  1        /* only one type */

/* Hardware controlled charging with monitoring */
#define CONFIG_CHARGING CHARGING_MONITOR

#define CFG_EXTAL 12000000    /* EXT clock: 12 Mhz */

#ifndef SIMULATOR

/* Define this if you have a Ingenic JZ4732 */
#define CONFIG_CPU JZ4732

/* define this if the hardware can be powered off while charging */
#define HAVE_POWEROFF_WHILE_CHARGING

/* Define this if you have a speaker */
#define HAVE_SPEAKER

/* Define this to the CPU frequency */
#define CPU_FREQ 336000000    /* CPU clock: 336 MHz */

/* define this if you have a flash memory storage */
#define HAVE_FLASH_STORAGE

/* Virtual LED (icon) */
#define CONFIG_LED LED_VIRTUAL

/* Offset ( in the firmware file's header ) to the file CRC */
#define FIRMWARE_OFFSET_FILE_CRC  0

/* Offset ( in the firmware file's header ) to the real data */
#define FIRMWARE_OFFSET_FILE_DATA 8

/* Define this if you have adjustable CPU frequency */
/* #define HAVE_ADJUSTABLE_CPU_FREQ */
#define CPUFREQ_NORMAL 336000000    /* CPU clock: 336 MHz */
#define CPUFREQ_MAX 336000000       /* CPU clock: 336 MHz */

#ifdef ONDA_VX747P
#define BOOTFILE_EXT "vx747p"
#else
#define BOOTFILE_EXT "vx747"
#endif
#define BOOTFILE     "rockbox." BOOTFILE_EXT
#define BOOTDIR      "/.rockbox"

#define CONFIG_USBOTG     USBOTG_JZ4740
#define HAVE_USBSTACK
#define USE_ROCKBOX_USB
#define USB_VENDOR_ID     0x07C4
#define USB_PRODUCT_ID    0xA4A5

#endif
