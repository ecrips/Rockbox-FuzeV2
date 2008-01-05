/***************************************************************************
*             __________               __   ___.
*   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
*   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
*   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
*   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
*                     \/            \/     \/    \/            \/
* $Id$
*
* Greyscale demo plugin
*
* Copyright (C) 2004-2008 Jens Arnold
*
* All files in this archive are subject to the GNU General Public License.
* See the file COPYING in the source tree root for full license agreement.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
****************************************************************************/

#include "plugin.h"
#include "helper.h" 

#if defined(HAVE_LCD_BITMAP) && (LCD_DEPTH < 4)
#include "grey.h"

PLUGIN_HEADER

/* variable button definitions */
#if CONFIG_KEYPAD == RECORDER_PAD
#define GREYSCALE_SHIFT BUTTON_ON
#define GREYSCALE_UP BUTTON_UP
#define GREYSCALE_DOWN BUTTON_DOWN
#define GREYSCALE_LEFT BUTTON_LEFT
#define GREYSCALE_RIGHT BUTTON_RIGHT
#define GREYSCALE_OFF BUTTON_OFF

#elif CONFIG_KEYPAD == ONDIO_PAD
#define GREYSCALE_SHIFT BUTTON_MENU
#define GREYSCALE_UP BUTTON_UP
#define GREYSCALE_DOWN BUTTON_DOWN
#define GREYSCALE_LEFT BUTTON_LEFT
#define GREYSCALE_RIGHT BUTTON_RIGHT
#define GREYSCALE_OFF BUTTON_OFF

#elif CONFIG_KEYPAD == IRIVER_H100_PAD
#define GREYSCALE_SHIFT BUTTON_ON
#define GREYSCALE_UP BUTTON_UP
#define GREYSCALE_DOWN BUTTON_DOWN
#define GREYSCALE_LEFT BUTTON_LEFT
#define GREYSCALE_RIGHT BUTTON_RIGHT
#define GREYSCALE_OFF BUTTON_OFF

#define GREYSCALE_RC_OFF BUTTON_RC_STOP

#elif (CONFIG_KEYPAD == IPOD_4G_PAD) || (CONFIG_KEYPAD == IPOD_3G_PAD) || \
      (CONFIG_KEYPAD == IPOD_1G2G_PAD)
#define GREYSCALE_SHIFT BUTTON_SELECT
#define GREYSCALE_UP BUTTON_SCROLL_BACK
#define GREYSCALE_DOWN BUTTON_SCROLL_FWD
#define GREYSCALE_LEFT BUTTON_LEFT
#define GREYSCALE_RIGHT BUTTON_RIGHT
#define GREYSCALE_OFF BUTTON_MENU

#elif CONFIG_KEYPAD == IRIVER_IFP7XX_PAD
#define GREYSCALE_SHIFT BUTTON_PLAY
#define GREYSCALE_UP BUTTON_UP
#define GREYSCALE_DOWN BUTTON_DOWN
#define GREYSCALE_LEFT BUTTON_LEFT
#define GREYSCALE_RIGHT BUTTON_RIGHT
#define GREYSCALE_OFF BUTTON_EQ
#endif

#define GFX_HEIGHT (LCD_HEIGHT-8)
#if LCD_WIDTH < 160
#define GFX_GREYTONE_WIDTH 86
#define GFX_GREYTONE_STEP 3
#else
#define GFX_GREYTONE_WIDTH 128
#define GFX_GREYTONE_STEP 2
#endif
/******************************* Globals ***********************************/

static struct plugin_api* rb; /* global api struct pointer */
static char pbuf[32];         /* global printf buffer */
static unsigned char *gbuf;
static size_t gbuf_size = 0;

/**************************** main function ********************************/

void cleanup(void *parameter)
{
    (void)parameter;

    grey_release(); /* switch off overlay and deinitialize */
    /* Turn on backlight timeout (revert to settings) */
    backlight_use_settings(rb); /* backlight control in lib/helper.c */
}

/* this is only a demo of what the framework can do */
int main(void)
{
    int time;
    int x, y, i;
    int button, scroll_amount;
    bool black_border = false;

    static const unsigned char rockbox[] = {
    /* ...........................................
     * .####...###...###..#...#.####...###..#...#.
     * .#...#.#...#.#...#.#..#..#...#.#...#..#.#..
     * .####..#...#.#.....###...####..#...#...#...
     * .#..#..#...#.#...#.#..#..#...#.#...#..#.#..
     * .#...#..###...###..#...#.####...###..#...#.
     * ...........................................
     * 43 x 7 pixel, 1 bpp
     */
       0x00, 0x3E, 0x0A, 0x0A, 0x1A, 0x24, 0x00, 0x1C, 0x22, 0x22,
       0x22, 0x1C, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x14, 0x00, 0x3E,
       0x08, 0x08, 0x14, 0x22, 0x00, 0x3E, 0x2A, 0x2A, 0x2A, 0x14,
       0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x22, 0x14, 0x08,
       0x14, 0x22, 0x00
    };
    
    static const unsigned char showing[] = {
    /* .......................................
     * ..####.#...#..###..#...#.#.#...#..####.
     * .#.....#...#.#...#.#...#.#.##..#.#.....
     * ..###..#####.#...#.#.#.#.#.#.#.#.#..##.
     * .....#.#...#.#...#.#.#.#.#.#..##.#...#.
     * .####..#...#..###...#.#..#.#...#..####.
     * .......................................
     * 39 x 7 pixel, 1 bpp
     */
       0x00, 0x24, 0x2A, 0x2A, 0x2A, 0x12, 0x00, 0x3E, 0x08, 0x08,
       0x08, 0x3E, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00, 0x1E,
       0x20, 0x18, 0x20, 0x1E, 0x00, 0x3E, 0x00, 0x3E, 0x04, 0x08,
       0x10, 0x3E, 0x00, 0x1C, 0x22, 0x22, 0x2A, 0x3A, 0x00
    };
    
    static const unsigned char grayscale_grey[] = {
    /* .......................................................
     * ..####.####...###..#...#..####..###...###..#.....#####.
     * .#.....#...#.#...#.#...#.#.....#...#.#...#.#.....#.....
     * .#..##.####..#####..#.#...###..#.....#####.#.....####..
     * .#...#.#..#..#...#...#.......#.#...#.#...#.#.....#.....
     * ..####.#...#.#...#...#...####...###..#...#.#####.#####.
     * .......................................................
     * 55 x 7 pixel, 8 bpp
     */
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110,
       120,120, 20, 20, 20, 20,120,222,222,222,222,120,120,120, 24, 24,
        24,120,120,226,120,120,120,226,120,120, 28, 28, 28, 28,120,120,
       230,230,230,120,120,120, 32, 32, 32,120,120,234,120,120,120,120,
       120, 36, 36, 36, 36, 36,120,
       130, 20,130,130,130,130,130,222,130,130,130,222,130, 24,130,130,
       130, 24,130,226,130,130,130,226,130, 28,130,130,130,130,130,230,
       130,130,130,230,130, 32,130,130,130, 32,130,234,130,130,130,130,
       130, 36,130,130,130,130,130,
       140, 20,140,140, 20, 20,140,222,222,222,222,140,140, 24, 24, 24,
        24, 24,140,140,226,140,226,140,140,140, 28, 28, 28,140,140,230,
       140,140,140,140,140, 32, 32, 32, 32, 32,140,234,140,140,140,140,
       140, 36, 36, 36, 36,140,140,
       130, 20,130,130,130, 20,130,222,130,130,222,130,130, 24,130,130,
       130, 24,130,130,130,226,130,130,130,130,130,130,130, 28,130,230,
       130,130,130,230,130, 32,130,130,130, 32,130,234,130,130,130,130,
       130, 36,130,130,130,130,130,
       120,120, 20, 20, 20, 20,120,222,120,120,120,222,120, 24,120,120,
       120, 24,120,120,120,226,120,120,120, 28, 28, 28, 28,120,120,120,
       230,230,230,120,120, 32,120,120,120, 32,120,234,234,234,234,234,
       120, 36, 36, 36, 36, 36,120,
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,
       110,110,110,110,110,110,110
    };

    /* Turn off backlight timeout */
    backlight_force_on(rb); /* backlight control in lib/helper.c */

    rb->lcd_setfont(FONT_SYSFIXED);   /* select default font */

    /* get the remainder of the plugin buffer */
    gbuf = (unsigned char *) rb->plugin_get_buffer(&gbuf_size);

    /* initialize the greyscale buffer:
       Archos: 112 pixels wide, 7 rows (56 pixels) high.
       H1x0: 160 pixels wide, 30 rows (120 pixels) high. */
    if (!grey_init(rb, gbuf, gbuf_size, true, LCD_WIDTH, GFX_HEIGHT, NULL))
    {
        rb->splash(HZ, "Not enough memory.");
        return PLUGIN_ERROR;
    }

    /* place greyscale overlay 1 row down */
    grey_set_position(0, 8);

    rb->lcd_puts(0, 0, "Shades: 129");
    rb->lcd_update();

#ifdef HAVE_ADJUSTABLE_CPU_FREQ
    rb->cpu_boost(true);
#endif
    grey_show(true);          /* switch on greyscale overlay */

    time = *rb->current_tick; /* start time measurement */

    grey_set_background(150);
    grey_clear_display();     /* fill everything with grey 150 */

    /* draw a dark grey line star background */
    grey_set_foreground(80);
    for (y = 0; y < GFX_HEIGHT; y += 8)                   /* horizontal part */
    {
        grey_drawline(0, y, (LCD_WIDTH-1), (GFX_HEIGHT-1) - y); /*grey lines */
    }
    for (x = 10; x <= LCD_WIDTH; x += 10)                    /* vertical part */
    {
        grey_drawline(x, 0, (LCD_WIDTH-1) - x, (GFX_HEIGHT-1)); /*grey lines */
    }

    grey_set_foreground(0);
    grey_drawrect(0, 0, LCD_WIDTH, GFX_HEIGHT);   /* black border */

    /* draw grey tones */
    for (i = 0; i < GFX_GREYTONE_WIDTH; i++)
    {
        x = ((LCD_WIDTH-GFX_GREYTONE_WIDTH)/2) + i;
        grey_set_foreground(GFX_GREYTONE_STEP * i);
        /* vertical lines */
        grey_vline(x, (GFX_HEIGHT/8), (GFX_HEIGHT-GFX_HEIGHT/8-1));
    }

    grey_set_drawmode(DRMODE_COMPLEMENT);
    /* invert rectangle (lower half) */
    grey_fillrect((LCD_WIDTH-GFX_GREYTONE_WIDTH)/2, (GFX_HEIGHT/2+1),
                  GFX_GREYTONE_WIDTH, (GFX_HEIGHT/2-GFX_HEIGHT/8-1));
    /* invert a line */
    grey_hline((LCD_WIDTH-GFX_GREYTONE_WIDTH)/2,
               (LCD_WIDTH+GFX_GREYTONE_WIDTH)/2, (GFX_HEIGHT/2-1));

    /* show bitmaps (1 bit and 8 bit) */
    /* opaque */
    grey_set_drawinfo(DRMODE_SOLID, 255, 100);
    grey_mono_bitmap(rockbox,
                     MAX((LCD_WIDTH/2-47), ((LCD_WIDTH-GFX_GREYTONE_WIDTH)/2)),
                     (5*GFX_HEIGHT/16-4), 43, 7);
    /* transparent */
    grey_set_drawinfo(DRMODE_FG, 0, 100);
    grey_mono_bitmap(showing, (LCD_WIDTH/2+4) , (5*GFX_HEIGHT/16-4), 39, 7);
    /* greyscale */
    grey_gray_bitmap(grayscale_grey, ((LCD_WIDTH-55)/2), (11*GFX_HEIGHT/16-4),
                     55, 7);

    grey_update();

    time = *rb->current_tick - time;  /* end time measurement */

    rb->snprintf(pbuf, sizeof(pbuf), "Shades: 129, %d.%02ds", 
                 time / 100, time % 100);
    rb->lcd_puts(0, 0, pbuf);
    grey_deferred_lcd_update();       /* schedule an lcd_update() */
#ifdef HAVE_ADJUSTABLE_CPU_FREQ
    rb->cpu_boost(false);
#endif

    /* drawing is now finished, play around with scrolling 
     * until you press OFF or connect USB
     */
    grey_set_background(255);
    while (true)
    {
        scroll_amount = 1;

        button = rb->button_get(true);

        if (rb->default_event_handler_ex(button, cleanup, NULL) 
            == SYS_USB_CONNECTED)
            return PLUGIN_USB_CONNECTED;

        if (button & GREYSCALE_SHIFT)
        {
            if (!black_border)
            {
                grey_set_background(0);
                black_border = true;
            }
        }
        else
        {
            if (black_border)
            {
                grey_set_background(255);
                black_border = false;
            }
        }

        if (button & BUTTON_REPEAT)
            scroll_amount = 4;

        switch (button & ~(GREYSCALE_SHIFT | BUTTON_REPEAT))
        {
            case GREYSCALE_LEFT:

                grey_scroll_left(scroll_amount);  /* scroll left */
                grey_update();
                break;

            case GREYSCALE_RIGHT:

                grey_scroll_right(scroll_amount); /* scroll right */
                grey_update();
                break;

            case GREYSCALE_UP:

                grey_scroll_up(scroll_amount);    /* scroll up */
                grey_update();
                break;

            case GREYSCALE_DOWN:

                grey_scroll_down(scroll_amount);  /* scroll down */
                grey_update();
                break;
#ifdef GREYSCALE_RC_OFF
            case GREYSCALE_RC_OFF:
#endif
            case GREYSCALE_OFF:

                cleanup(NULL);
                return PLUGIN_OK;
        }
    }
}

/*************************** Plugin entry point ****************************/

enum plugin_status plugin_start(struct plugin_api* api, void* parameter)
{
    rb = api; /* copy to global api pointer */
    (void)parameter;

    return main();
}

#endif /* #ifdef HAVE_LCD_BITMAP */
