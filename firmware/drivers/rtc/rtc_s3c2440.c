/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Linus Nielsen Feltzing, Uwe Freese, Laurent Baum
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
#include "config.h"
#include "i2c.h"
#include "rtc.h"
#include "kernel.h"
#include "system.h"

void rtc_init(void)
{
    /* RTC Enable */
    RTCCON |= 1;
}

int rtc_read_datetime(unsigned char* buf)
{
    buf[0] = BCDSEC;
    buf[1] = BCDMIN;
    buf[2] = BCDHOUR;
    buf[3] = BCDDAY-1;	/* timefuncs wants 0..6 for wday */
    buf[4] = BCDDATE;
    buf[5] = BCDMON;
    buf[6] = BCDYEAR;

    return 1;
}

int rtc_write_datetime(unsigned char* buf)
{
    BCDSEC  = buf[0];
    BCDMIN  = buf[1];
    BCDHOUR = buf[2];
    BCDDAY  = buf[3]+1; /* chip wants 1..7 for wday */
    BCDDATE = buf[4];
    BCDMON  = buf[5];
    BCDYEAR = buf[6];

    return 1;
}

#ifdef HAVE_RTC_ALARM
/* This alarm code works with a flashed bootloader.  This will not work with
 * the OF bootloader.
 */
 
/* Check whether the unit has been started by the RTC alarm function */
bool rtc_check_alarm_started(bool release_alarm)
{
    if (GSTATUS3) 
    {
        GSTATUS3 &= ~release_alarm;  
        return true;
    } 
    else 
    { 
        return false;
    }
}

/* Check to see if the alarm has flaged since the last it was checked */
bool rtc_check_alarm_flag(void) 
{
    bool ret=SRCPND & 0x40000000;
    
    SRCPND=RTC_MASK;
    
    return ret;
}

/* set alarm time registers to the given time (repeat once per day) */
void rtc_set_alarm(int h, int m)
{
    ALMMIN=(((m / 10) << 4) | (m % 10)) & 0x7f; /* minutes */
    ALMHOUR=(((h / 10) << 4) | (h % 10)) & 0x3f; /* hour */
}

/* read out the current alarm time */
void rtc_get_alarm(int *h, int *m)
{
    *m=((ALMMIN & 0x70) >> 4) * 10 + (ALMMIN & 0x0f);
    *h=((ALMHOUR & 0x30) >> 4) * 10 + (ALMHOUR & 0x0f);
}

/* turn alarm on or off by setting the alarm flag enable
 * returns false if alarm was set and alarm flag (output) is off 
 */
bool rtc_enable_alarm(bool enable)
{
    if (enable)
    {
        RTCALM=0x46;
    }
    else
    {
        RTCALM=0x00;
    }

    return false; /* all ok */
}
#endif
