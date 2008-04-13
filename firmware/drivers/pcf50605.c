/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Driver for pcf50605 PMU and RTC
 *
 * Based on code from the ipodlinux project - http://ipodlinux.org/
 * Adapted for Rockbox in December 2005
 *
 * Original file: linux/arch/armnommu/mach-ipod/pcf50605.c
 *
 * Copyright (c) 2003-2005 Bernard Leach (leachbj@bouncycastle.org)
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include "system.h"
#include "config.h"
#if CONFIG_I2C == I2C_PP5020 || CONFIG_I2C == I2C_PP5002
#include "i2c-pp.h"
#endif
#include "rtc.h"
#include "pcf5060x.h"

unsigned char pcf50605_wakeup_flags = 0;

int pcf50605_read(int address)
{
    return i2c_readbyte(0x8,address);
}

int pcf50605_read_multiple(int address, unsigned char* buf, int count)
{
    int read = i2c_readbytes(0x08, address, count, buf);
    return read - count;
}

int pcf50605_write(int address, unsigned char val)
{
    pp_i2c_send(0x8, address, val);
    return 0;
}

int pcf50605_write_multiple(int address, const unsigned char* buf, int count)
{
    int i;

    i2c_lock();

    for (i = 0; i < count; i++)
        pp_i2c_send(0x8, address + i, buf[i]);

    i2c_unlock();

    return 0;
}

/* The following command puts the iPod into a deep sleep.  Warning
   from the good people of ipodlinux - never issue this command
   without setting CHGWAK or EXTONWAK if you ever want to be able to
   power on your iPod again. */
void pcf50605_standby_mode(void)
{
    pcf50605_write(PCF5060X_OOCC1,
                   GOSTDBY | CHGWAK | EXTONWAK | pcf50605_wakeup_flags);
}

void pcf50605_init(void)
{
#if defined (IPOD_VIDEO)    
    /* I/O and GPO voltage supply (default: 0xf8 = 3.3V ON) */
    /* ECO not allowed regarding data sheet */
    pcf50605_write(PCF5060X_IOREGC,  0xf8);  /* 3.3V ON */
    
    /* core voltage supply (default DCDC1/DCDC2: 0xec = 1.2V ON) */
    /* ECO not stable, assumed due to less precision of voltage in ECO mode */
    pcf50605_write(PCF5060X_DCDC1,   0xec);  /* 1.2V ON */
    pcf50605_write(PCF5060X_DCDC2,   0x0c);  /* OFF */
    
    /* unknown (default: 0xe3 = 1.8V ON) */
    pcf50605_write(PCF5060X_DCUDC1,  0xe3);  /* 1.8V ON */
    
    /* WM8758 voltage supply (default: 0xf5 = 3.0V ON) */
    /* ECO not allowed as max. current of 5mA is not sufficient */
    pcf50605_write(PCF5060X_D1REGC1, 0xf0);  /* 2.5V ON */
    
    /* LCD voltage supply (default: 0xf5 = 3.0V ON) */
    pcf50605_write(PCF5060X_D3REGC1, 0xf1);  /* 2.6V ON */
#else
    /* keep initialization from svn for other iPods */
    pcf50605_write(PCF5060X_D1REGC1, 0xf5); /* 3.0V ON */
    pcf50605_write(PCF5060X_D3REGC1, 0xf5); /* 3.0V ON */
    /* Dock Connector pin 17 (default: OFF) */
    pcf50605_write(PCF5060X_D2REGC1, 0xf8); /* 3.3V ON */
#endif
}
