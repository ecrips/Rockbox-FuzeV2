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

#include "config.h"
#include "cpu.h"
#include "system.h"
#include "kernel.h"
#include "ata.h"
#include "usb.h"
#include "usb-target.h"

#define USB_RST_ASSERT
#define USB_RST_DEASSERT

#define USB_VPLUS_PWR_ASSERT
#define USB_VPLUS_PWR_DEASSERT

#define USB_UNIT_IS_PRESENT USB_EXTRACTED

/* The usb detect is one pin to the cpu active low */
inline int usb_detect(void)
{
    return USB_UNIT_IS_PRESENT;
}

void usb_init_device(void)
{
//    ata_enable(true);
}

void usb_enable(bool on)
{
    if (on)
    {
        USB_VPLUS_PWR_ASSERT;
    }
    else
    {
        USB_VPLUS_PWR_DEASSERT;
    }
}
