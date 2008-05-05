/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2008 by Barry Wardell
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include "lcd.h"
#include "lcd-remote.h"
#include "font.h"
#include <stdio.h>
#include <string.h>

#include "rockboxlogo.h"

int show_logo( void )
{
    char boot_version[32];
    
    snprintf(boot_version, sizeof(boot_version), "Boot Ver. %s", APPSVERSION);

    lcd_clear_display();
    lcd_bitmap(rockboxlogo, 0, 10, BMPWIDTH_rockboxlogo, BMPHEIGHT_rockboxlogo);
    lcd_setfont(FONT_SYSFIXED);
    lcd_putsxy((LCD_WIDTH/2) - ((strlen(boot_version)*SYSFONT_WIDTH)/2),
               LCD_HEIGHT-SYSFONT_HEIGHT, (unsigned char *)boot_version);

    lcd_update();
    
    return 0;
}
