/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 Itai Shaked
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
Snake!

by Itai Shaked

ok, a little explanation - 
board holds the snake and apple position - 1+ - snake body (the number
represents the age [1 is the snake's head]).
-1 is an apple, and 0 is a clear spot.
dir is the current direction of the snake - 0=up, 1=right, 2=down, 3=left;

*/

#include "plugin.h"
#ifdef HAVE_LCD_BITMAP

PLUGIN_HEADER

/* variable button definitions */
#if CONFIG_KEYPAD == RECORDER_PAD
#define SNAKE_QUIT BUTTON_OFF
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_PLAY

#elif CONFIG_KEYPAD == ARCHOS_AV300_PAD
#define SNAKE_QUIT BUTTON_OFF
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif CONFIG_KEYPAD == ONDIO_PAD
#define SNAKE_QUIT BUTTON_OFF
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_MENU

#elif (CONFIG_KEYPAD == IRIVER_H100_PAD) || \
      (CONFIG_KEYPAD == IRIVER_H300_PAD)
#define SNAKE_QUIT BUTTON_OFF
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_ON

#define SNAKE_RC_QUIT BUTTON_RC_STOP

#elif (CONFIG_KEYPAD == IPOD_4G_PAD) || \
      (CONFIG_KEYPAD == IPOD_3G_PAD) || \
      (CONFIG_KEYPAD == IPOD_1G2G_PAD)
#define SNAKE_QUIT (BUTTON_SELECT|BUTTON_MENU)
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_MENU
#define SNAKE_DOWN BUTTON_PLAY
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif (CONFIG_KEYPAD == IAUDIO_X5M5_PAD)
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_PLAY

#elif (CONFIG_KEYPAD == GIGABEAT_PAD)
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif (CONFIG_KEYPAD == SANSA_E200_PAD) || \
    (CONFIG_KEYPAD == SANSA_C200_PAD) || \
    (CONFIG_KEYPAD == SANSA_CLIP_PAD) || \
    (CONFIG_KEYPAD == SANSA_M200_PAD)
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif (CONFIG_KEYPAD == SANSA_FUZE_PAD)
#define SNAKE_QUIT (BUTTON_HOME|BUTTON_REPEAT)
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif (CONFIG_KEYPAD == IRIVER_H10_PAD)
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_SCROLL_UP
#define SNAKE_DOWN BUTTON_SCROLL_DOWN
#define SNAKE_PLAYPAUSE BUTTON_PLAY

#elif (CONFIG_KEYPAD == GIGABEAT_S_PAD)
#define SNAKE_QUIT BUTTON_BACK
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif (CONFIG_KEYPAD == MROBE100_PAD)
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_SELECT

#elif CONFIG_KEYPAD == IAUDIO_M3_PAD
#define SNAKE_QUIT BUTTON_RC_REC
#define SNAKE_LEFT BUTTON_RC_REW
#define SNAKE_RIGHT BUTTON_RC_FF
#define SNAKE_UP   BUTTON_RC_VOL_UP
#define SNAKE_DOWN BUTTON_RC_VOL_DOWN
#define SNAKE_PLAYPAUSE BUTTON_RC_PLAY

#define SNAKE_RC_QUIT BUTTON_REC

#elif (CONFIG_KEYPAD == COWOND2_PAD)
#define SNAKE_QUIT BUTTON_POWER

#elif CONFIG_KEYPAD == CREATIVEZVM_PAD
#define SNAKE_QUIT BUTTON_BACK
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_PLAY

#elif CONFIG_KEYPAD == PHILIPS_HDD1630_PAD
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_LEFT BUTTON_LEFT
#define SNAKE_RIGHT BUTTON_RIGHT
#define SNAKE_UP   BUTTON_UP
#define SNAKE_DOWN BUTTON_DOWN
#define SNAKE_PLAYPAUSE BUTTON_MENU

#elif CONFIG_KEYPAD == MROBE500_PAD
#define SNAKE_QUIT BUTTON_POWER
#define SNAKE_RC_QUIT BUTTON_RC_DOWN

#elif (CONFIG_KEYPAD == ONDAVX747_PAD)
#define SNAKE_QUIT BUTTON_POWER

#else
#error No keymap defined!
#endif

#ifdef HAVE_TOUCHSCREEN
#ifndef SNAKE_QUIT
#define SNAKE_QUIT      BUTTON_TOPLEFT
#endif
#ifndef SNAKE_LEFT
#define SNAKE_LEFT      BUTTON_MIDLEFT
#endif
#ifndef SNAKE_RIGHT
#define SNAKE_RIGHT     BUTTON_MIDRIGHT
#endif
#ifndef SNAKE_UP
#define SNAKE_UP        BUTTON_TOPMIDDLE
#endif
#ifndef SNAKE_DOWN
#define SNAKE_DOWN      BUTTON_BOTTOMMIDDLE
#endif
#ifndef SNAKE_PLAYPAUSE
#define SNAKE_PLAYPAUSE BUTTON_CENTER
#endif
#endif

#define BOARD_WIDTH (LCD_WIDTH/4)
#define BOARD_HEIGHT (LCD_HEIGHT/4)

static int board[BOARD_WIDTH][BOARD_HEIGHT],snakelength;
static unsigned int score,hiscore=0,level=1;
static int dir,dead=0;
static bool apple;

void die (void)
{
    char pscore[17];
    rb->lcd_clear_display();
    rb->snprintf(pscore,sizeof(pscore),"Your score: %d",score);
    rb->lcd_puts(0,0,"Oops...");
    rb->lcd_puts(0,1, pscore);
    if (score>hiscore) {
        hiscore=score;
        rb->lcd_puts(0,2,"New High Score!");
    }
    else {
        rb->snprintf(pscore,sizeof(pscore),"High Score: %d",hiscore);
        rb->lcd_puts(0,2,pscore);
    }
    rb->lcd_update();
    rb->sleep(3*HZ);
    dead=1;
}

void colission (short x, short y)
{
    switch (board[x][y]) {
        case 0:
            break; 
        case -1:
            snakelength+=2;
            score+=level;
            apple=false;
            break;
        default:
            die();
            break;
    }
    if (x==BOARD_WIDTH || x<0 || y==BOARD_HEIGHT || y<0) 
        die();
}

void move_head (short x, short y)
{
    switch (dir) {
        case 0:
            y-=1;
            break;
        case 1:
            x+=1; 
            break;
        case 2:
            y+=1;
            break;
        case 3:
            x-=1;
            break;
    }
    colission (x,y);
    if (dead)
        return;
    board[x][y]=1;
    rb->lcd_fillrect(x*4,y*4,4,4);
}

void frame (void)
{
    short x,y,head=0;
    for (x=0; x<BOARD_WIDTH; x++) {
        for (y=0; y<BOARD_HEIGHT; y++) {
            switch (board[x][y]) {
                case 1:
                    if (!head) {
                        move_head(x,y);
                        if (dead)
                            return;
                        board[x][y]++;
                        head=1;
                    }
                    break;
                case 0:
                    break;
                case -1:
                    break;
                default:
                    if (board[x][y]==snakelength) {
                        board[x][y]=0;
                        rb->lcd_set_drawmode(DRMODE_SOLID|DRMODE_INVERSEVID);
                        rb->lcd_fillrect(x*4,y*4,4,4);
                        rb->lcd_set_drawmode(DRMODE_SOLID);
                    }
                    else 
                        board[x][y]++;
                    break;
            }
        }
    }
    rb->lcd_update();
}

void redraw (void)
{
    short x,y;
    rb->lcd_clear_display();
    for (x=0; x<BOARD_WIDTH; x++) {
        for (y=0; y<BOARD_HEIGHT; y++) {
            switch (board[x][y]) {
                case -1:
                    rb->lcd_fillrect((x*4)+1,y*4,2,4);
                    rb->lcd_fillrect(x*4,(y*4)+1,4,2);
                    break;
                case 0:
                    break;
                default:
                    rb->lcd_fillrect(x*4,y*4,4,4);
                    break;
            }
        }
    }
    rb->lcd_update();
}

void game_pause (void) {
    int button;
    rb->lcd_clear_display();
    rb->lcd_putsxy(3,12,"Game Paused");
#if CONFIG_KEYPAD == RECORDER_PAD
    rb->lcd_putsxy(3,22,"[Play] to resume");
#elif CONFIG_KEYPAD == ONDIO_PAD
    rb->lcd_putsxy(3,22,"[Mode] to resume");
#endif
    rb->lcd_putsxy(3,32,"[Off] to quit");
    rb->lcd_update();
    while (1) {
        button=rb->button_get(true);
        switch (button) {
#ifdef SNAKE_RC_QUIT
            case SNAKE_RC_QUIT:
#endif
            case SNAKE_QUIT:
                dead=1;
                return;
            case SNAKE_PLAYPAUSE:
                redraw();
                rb->sleep(HZ/2);
                return;
            default:
                if (rb->default_event_handler(button)==SYS_USB_CONNECTED) {
                    dead=2;
                    return;
                }
                break;
        }
    }
}


void game (void) {
    int button;
    short x,y;
    while (1) {
        frame();
        if (dead)
            return;
        if (!apple) {
            do {
                x=rb->rand() % BOARD_WIDTH;
                y=rb->rand() % BOARD_HEIGHT;
            } while (board[x][y]);
            apple=true;
            board[x][y]=-1;
            rb->lcd_fillrect((x*4)+1,y*4,2,4);
            rb->lcd_fillrect(x*4,(y*4)+1,4,2);
        }

        rb->sleep(HZ/level);

        button=rb->button_get(false);
        
#ifdef HAS_BUTTON_HOLD
        if (rb->button_hold())
        button = SNAKE_PLAYPAUSE;
#endif

        switch (button) {
             case SNAKE_UP:
                 if (dir!=2) dir=0;
                 break;
             case SNAKE_RIGHT:
                 if (dir!=3) dir=1;
                 break;
             case SNAKE_DOWN:
                 if (dir!=0) dir=2;
                 break;
             case SNAKE_LEFT:
                 if (dir!=1) dir=3;
                 break;
#ifdef SNAKE_RC_QUIT
             case SNAKE_RC_QUIT:
#endif
             case SNAKE_QUIT:
                 dead=1;
                 return;
             case SNAKE_PLAYPAUSE:
                 game_pause();
                 break;
             default:
                 if (rb->default_event_handler(button)==SYS_USB_CONNECTED) {
                     dead=2;
                     return;
                 }
                 break;
        }
    }
}

void game_init(void) {
    int selection=0;
    short x,y;
    bool menu_quit = false;

    for (x=0; x<BOARD_WIDTH; x++) {
        for (y=0; y<BOARD_HEIGHT; y++) {
            board[x][y]=0;
        }
    }
    dead=0;
    apple=false;
    snakelength=4;
    score=0;
    board[11][7]=1;   
    
    MENUITEM_STRINGLIST(menu,"Snake Menu",NULL,"Start New Game","Starting Level",
                        "Quit");
                                           
    while (!menu_quit) {
        switch(rb->do_menu(&menu, &selection, NULL, false))
        {
            case 0:
                menu_quit = true; /* start playing */
                break;
                
            case 1:
                rb->set_int("Starting Level", "", UNIT_INT, &level, NULL,
                            1, 1, 9, NULL );
                break;
                
            default:
                dead=1; /* quit program */
                menu_quit = true;
                break;

        }
    }
}

enum plugin_status plugin_start(const void* parameter)
{
    (void)(parameter);

    game_init(); 
    rb->lcd_clear_display();
    game();
    return (dead==1)?PLUGIN_OK:PLUGIN_USB_CONNECTED;
}

#endif
