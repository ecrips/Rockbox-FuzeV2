/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 by Bj�rn Stenberg <bjorn@haxx.se>
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#include "debug.h"

#include "screens.h"
#include "button.h"

#include "string.h"
#include "lcd.h"

#include "ata.h" /* for volume definitions */

extern char having_new_lcd;

#if CONFIG_CODEC != SWCODEC
void audio_set_buffer_margin(int seconds)
{
     (void)seconds;
}
#endif

int fat_startsector(void)
{
    return 63;
}

int ata_write_sectors(IF_MV2(int drive,)
                      unsigned long start,
                      int count,
                      const void* buf)
{
    int i;

    for (i=0; i<count; i++ ) {
        FILE* f;
        char name[32];

        sprintf(name,"sector%lX.bin",start+i);
        f=fopen(name,"wb");
        if (f) {
            fwrite(buf,512,1,f);
            fclose(f);
        }
    }
    return 1;
}

int ata_read_sectors(IF_MV2(int drive,)
                     unsigned long start,
                     int count,
                     void* buf)
{
    int i;

    for (i=0; i<count; i++ ) {
        FILE* f;
        char name[32];

        DEBUGF("Reading sector %lX\n",start+i);
        sprintf(name,"sector%lX.bin",start+i);
        f=fopen(name,"rb");
        if (f) {
            fread(buf,512,1,f);
            fclose(f);
        }
    }
    return 1;
}

void ata_delayed_write(unsigned long sector, const void* buf)
{
    ata_write_sectors(IF_MV2(0,) sector, 1, buf);
}

void ata_flush(void)
{
}

void ata_spin(void)
{
}

void ata_spindown(int s)
{
    (void)s;
}

bool simulate_usb(void)
{
    usb_screen();
    return false;
}

int rtc_read(int address)
{
    return address ^ 0x55;
}

int rtc_write(int address, int value)
{
    (void)address;
    (void)value;
    return 0;
}

int rtc_read_datetime(unsigned char* buf)
{
    time_t now = time(NULL);
    struct tm *teem = localtime(&now);

    buf[0] = (teem->tm_sec%10) | ((teem->tm_sec/10) << 4);
    buf[1] = (teem->tm_min%10) | ((teem->tm_min/10) << 4);
    buf[2] = (teem->tm_hour%10) | ((teem->tm_hour/10) << 4);
    buf[3] = (teem->tm_wday);
    buf[4] = (teem->tm_mday%10) | ((teem->tm_mday/10) << 4);
    buf[5] = ((teem->tm_year-100)%10) | (((teem->tm_year-100)/10) << 4);
    buf[6] = ((teem->tm_mon+1)%10) | (((teem->tm_mon+1)/10) << 4);

    return 0;
}

int rtc_write_datetime(unsigned char* buf)
{
    (void)buf;
    return 0;
}

bool is_new_player(void)
{
    return having_new_lcd;
}

void lcd_set_contrast( int x )
{
    (void)x;
}

void mpeg_set_pitch(int pitch)
{
    (void)pitch;
}

static int sleeptime;
void set_sleep_timer(int seconds)
{
    sleeptime = seconds;
}

int get_sleep_timer(void)
{
    return sleeptime;
}

#ifdef HAVE_LCD_CHARCELLS
void lcd_clearrect (int x, int y, int nx, int ny)
{
  /* Reprint char if you want to change anything */
  (void)x;
  (void)y;
  (void)nx;
  (void)ny;
}

void lcd_fillrect (int x, int y, int nx, int ny)
{
  /* Reprint char if you want to change display anything */
  (void)x;
  (void)y;
  (void)nx;
  (void)ny;
}
#endif

void cpu_sleep(bool enabled)
{
    (void)enabled;
}

void button_set_flip(bool yesno)
{
    (void)yesno;
}

#if CONFIG_CODEC != SWCODEC
void talk_init(void)
{
}

int talk_buffer_steal(void)
{
    return 0;
}

int talk_id(int id, bool enqueue)
{
    (void)id;
    (void)enqueue;
    return 0;
}

int talk_file(char* filename, bool enqueue)
{
    (void)filename;
    (void)enqueue;
    return 0;
}

int talk_value(int n, int unit, bool enqueue)
{
    (void)n;
    (void)unit;
    (void)enqueue;
    return 0;
}

int talk_number(int n, bool enqueue)
{
    (void)n;
    (void)enqueue;
    return 0;
}

int talk_spell(char* spell, bool enqueue)
{
    (void)spell;
    (void)enqueue;
    return 0;
}

const char* const dir_thumbnail_name = "_dirname.talk";
const char* const file_thumbnail_ext = ".talk";
#endif

/* FIXME: this shoudn't be a stub, rather the real thing.
   I'm afraid on Win32/X11 it'll be hard to kill a thread from outside. */
void remove_thread(int threadnum)
{
    (void)threadnum;
}

/* assure an unused place to direct virtual pointers to */
#define VIRT_SIZE 0xFFFF /* more than enough for our string ID range */
unsigned char vp_dummy[VIRT_SIZE];

