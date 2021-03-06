/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 Jerome Kuptz
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
#include <string.h>
#include <stdlib.h>
#include "config.h"

#include "system.h"
#include "file.h"
#include "lcd.h"
#include "font.h"
#include "backlight.h"
#include "action.h"
#include "kernel.h"
#include "filetypes.h"
#include "debug.h"
#include "sprintf.h"
#include "settings.h"
#include "gwps.h"
#include "gwps-common.h"
#include "audio.h"
#include "usb.h"
#include "status.h"
#include "storage.h"
#include "screens.h"
#include "playlist.h"
#ifdef HAVE_LCD_BITMAP
#include "icons.h"
#include "peakmeter.h"
#endif
#include "lang.h"
#include "bookmark.h"
#include "misc.h"
#include "sound.h"
#include "onplay.h"
#include "abrepeat.h"
#include "playback.h"
#include "splash.h"
#include "cuesheet.h"
#include "ata_idle_notify.h"
#include "root_menu.h"
#include "backdrop.h"
#include "quickscreen.h"
#include "pitchscreen.h"
#include "appevents.h"
#include "viewport.h"
#include "pcmbuf.h"
#include "option_select.h"
#include "dsp.h"

#define RESTORE_WPS_INSTANTLY       0l
#define RESTORE_WPS_NEXT_SECOND     ((long)(HZ+current_tick))
/* in milliseconds */
#define DEFAULT_SKIP_TRESH          3000ul

static int wpsbars;
/* currently only one wps_state is needed */
struct wps_state wps_state;
struct gui_wps gui_wps[NB_SCREENS];
static struct wps_data wps_datas[NB_SCREENS];

/* initial setup of wps_data  */
static void wps_state_init(void);
static void track_changed_callback(void *param);
static void nextid3available_callback(void* param);


#if defined(HAVE_BACKLIGHT) || defined(HAVE_REMOTE_LCD)
static void gwps_caption_backlight(struct wps_state *state)
{
    if (state && state->id3)
    {
#ifdef HAVE_BACKLIGHT
        if (global_settings.caption_backlight)
        {
            /* turn on backlight n seconds before track ends, and turn it off n
               seconds into the new track. n == backlight_timeout, or 5s */
            int n = global_settings.backlight_timeout * 1000;

            if ( n < 1000 )
                n = 5000; /* use 5s if backlight is always on or off */

            if (((state->id3->elapsed < 1000) ||
                 ((state->id3->length - state->id3->elapsed) < (unsigned)n)) &&
                (state->paused == false))
                backlight_on();
        }
#endif
#ifdef HAVE_REMOTE_LCD
        if (global_settings.remote_caption_backlight)
        {
            /* turn on remote backlight n seconds before track ends, and turn it
               off n seconds into the new track. n == remote_backlight_timeout,
               or 5s */
            int n = global_settings.remote_backlight_timeout * 1000;

            if ( n < 1000 )
                n = 5000; /* use 5s if backlight is always on or off */

            if (((state->id3->elapsed < 1000) ||
                 ((state->id3->length - state->id3->elapsed) < (unsigned)n)) &&
                (state->paused == false))
                remote_backlight_on();
        }
#endif
    }
}
#endif


static void change_dir(int direction)
{
    if (global_settings.prevent_skip)
        return;

    if (direction < 0)
        audio_prev_dir();
    else if (direction > 0)
        audio_next_dir();
}

static void prev_track(unsigned long skip_thresh)
{
    if (wps_state.id3->elapsed < skip_thresh)
    {
        audio_prev();
        return;
    }
    else
    {
        if (cuesheet_is_enabled() && wps_state.id3->cuesheet_type)
        {
            curr_cuesheet_skip(-1, wps_state.id3->elapsed);
            return;
        }

        if (!wps_state.paused)
#if (CONFIG_CODEC == SWCODEC)
            audio_pre_ff_rewind();
#else
        audio_pause();
#endif

        audio_ff_rewind(0);

#if (CONFIG_CODEC != SWCODEC)
        if (!wps_state.paused)
            audio_resume();
#endif
    }
}

static void next_track(void)
{
    /* take care of if we're playing a cuesheet */
    if (cuesheet_is_enabled() && wps_state.id3->cuesheet_type)
    {
        if (curr_cuesheet_skip(1, wps_state.id3->elapsed))
        {
            /* if the result was false, then we really want
               to skip to the next track */
            return;
        }
    }

    audio_next();
}

static void play_hop(int direction)
{
    unsigned long step = ((unsigned long)global_settings.skip_length)*1000;
    unsigned long elapsed = wps_state.id3->elapsed;
    unsigned long remaining = wps_state.id3->length - elapsed;

    if (!global_settings.prevent_skip &&
           (!step ||
            (direction > 0 && step >= remaining) ||
            (direction < 0 && elapsed < DEFAULT_SKIP_TRESH)))
    {   /* Do normal track skipping */
        if (direction > 0)
            next_track();
        else if (direction < 0)
            prev_track(DEFAULT_SKIP_TRESH);
        return;
    }

    if (direction == 1 && step >= remaining)
    {
#if CONFIG_CODEC == SWCODEC
        if(global_settings.beep)
            pcmbuf_beep(1000, 150, 1500*global_settings.beep);
#endif
        return;
    }
    else if ((direction == -1 && elapsed < step))
    {
        elapsed = 0;
    }
    else
    {
        elapsed += step * direction;
    }
    if((audio_status() & AUDIO_STATUS_PLAY) && !wps_state.paused)
    {
#if (CONFIG_CODEC == SWCODEC)
        audio_pre_ff_rewind();
#else
        audio_pause();
#endif
    }
    audio_ff_rewind(wps_state.id3->elapsed = elapsed);
#if (CONFIG_CODEC != SWCODEC)
    if (!wps_state.paused)
        audio_resume();
#endif
}

static void gwps_fix_statusbars(void)
{
#ifdef HAVE_LCD_BITMAP
    int i;  
    wpsbars = VP_SB_HIDE_ALL;
    FOR_NB_SCREENS(i)
    {
        bool draw = false;
        if (gui_wps[i].data->wps_sb_tag)
            draw = gui_wps[i].data->show_sb_on_wps;
        else if (global_settings.statusbar)
            wpsbars |= VP_SB_ONSCREEN(i);
        if (draw)
            wpsbars |= (VP_SB_ONSCREEN(i) | VP_SB_IGNORE_SETTING(i));
    }
#else
    wpsbars = VP_SB_ALLSCREENS;
#endif
}

#if defined(HAVE_LCD_ENABLE) || defined(HAVE_LCD_SLEEP)
/*
 * If the user is unable to see the wps, because the display is deactivated,
 * we surpress updates until the wps gets actived again (the lcd driver will
 * call this hook to issue an instant update)
 * */
static void wps_lcd_activation_hook(void)
{
    wps_state.do_full_update = true;
    /* force timeout in wps main loop, so that the update is instantly */
    queue_post(&button_queue, BUTTON_NONE, 0);
}
#endif

static void gwps_leave_wps(void)
{
    int i, oldbars = VP_SB_HIDE_ALL;

    FOR_NB_SCREENS(i)
        gui_wps[i].display->stop_scroll();
    if (global_settings.statusbar)
        oldbars = VP_SB_ALLSCREENS;

#if LCD_DEPTH > 1
    show_main_backdrop();
#endif
#if defined(HAVE_REMOTE_LCD) && LCD_REMOTE_DEPTH > 1
    show_remote_main_backdrop();
#endif
    viewportmanager_set_statusbar(oldbars);
#if defined(HAVE_LCD_ENABLE) || defined(HAVE_LCD_SLEEP)
    /* Play safe and unregister the hook */
    lcd_activation_set_hook(NULL);
#endif
}

void gwps_draw_statusbars(void)
{
    viewportmanager_set_statusbar(wpsbars);
}
#ifdef HAVE_TOUCHSCREEN
int wps_get_touchaction(struct wps_data *data)
{
    short x,y;
    short vx, vy;
    int type = action_get_touchscreen_press(&x, &y);
    int i;
    static int last_action = ACTION_NONE;
    struct touchregion *r;
    bool repeated = (type == BUTTON_REPEAT);
    bool released = (type == BUTTON_REL);
    for (i=0; i<data->touchregion_count; i++)
    {
        r = &data->touchregion[i];
        /* make sure this region's viewport is visible */
        if (r->wvp->hidden_flags&VP_DRAW_HIDDEN)
            continue;
        /* reposition the touch inside the viewport */    
        vx = x - r->wvp->vp.x;
        vy = y - r->wvp->vp.y;
        /* check if its inside this viewport */
        if (vx >= 0 && vx < r->wvp->vp.x + r->wvp->vp.width &&
            vy >= 0 && vy < r->wvp->vp.y + r->wvp->vp.height)
        {
            /* now see if the point is inside this region */
            if (vx >= r->x && vx < r->x+r->width &&
                vy >= r->y && vy < r->y+r->height)
            {
                if ((repeated && r->repeat) ||
                    (released && !r->repeat))
                {
                    last_action = r->action;    
                    return r->action;
                }    
            }    
        }
    }
    if ((last_action == ACTION_WPS_SEEKBACK || last_action == ACTION_WPS_SEEKFWD))
        return ACTION_WPS_STOPSEEK;
    last_action = ACTION_TOUCHSCREEN;
    return ACTION_TOUCHSCREEN;
}
#endif
/* The WPS can be left in two ways:
 *      a)  call a function, which draws over the wps. In this case, the wps
 *          will be still active (i.e. the below function didn't return)
 *      b)  return with a value evaluated by root_menu.c, in this case the wps
 *          is really left, and root_menu will handle the next screen
 *
 * In either way, call gwps_leave_wps(), in order to restore the correct
 * "main screen" backdrops and statusbars
 */
long gui_wps_show(void)
{
    long button = 0;
    bool restore = true;
    long restoretimer = RESTORE_WPS_INSTANTLY; /* timer to delay screen redraw temporarily */
    bool exit = false;
    bool bookmark = false;
    bool update = false;
    int i;
    long last_left = 0, last_right = 0;

#ifdef HAVE_LCD_CHARCELLS
    status_set_audio(true);
    status_set_param(false);
#endif

#ifdef AB_REPEAT_ENABLE
    ab_repeat_init();
    ab_reset_markers();
#endif
    wps_state_init();

    while ( 1 )
    {
        bool audio_paused = (audio_status() & AUDIO_STATUS_PAUSE)?true:false;

        /* did someone else (i.e power thread) change audio pause mode? */
        if (wps_state.paused != audio_paused) {
            wps_state.paused = audio_paused;

            /* if another thread paused audio, we are probably in car mode,
               about to shut down. lets save the settings. */
            if (wps_state.paused) {
                settings_save();
#if !defined(HAVE_RTC_RAM) && !defined(HAVE_SW_POWEROFF)
                call_storage_idle_notifys(true);
#endif
            }
        }

#ifdef HAVE_LCD_BITMAP
        /* when the peak meter is enabled we want to have a
            few extra updates to make it look smooth. On the
            other hand we don't want to waste energy if it
            isn't displayed */
        bool pm=false;
        FOR_NB_SCREENS(i)
        {
           if(gui_wps[i].data->peak_meter_enabled)
               pm = true;
        }

        if (pm) {
            long next_refresh = current_tick;
            long next_big_refresh = current_tick + HZ / 5;
            button = BUTTON_NONE;
            while (TIME_BEFORE(current_tick, next_big_refresh)) {
                button = get_action(CONTEXT_WPS|ALLOW_SOFTLOCK,TIMEOUT_NOBLOCK);
                if (button != ACTION_NONE) {
                    break;
                }
                peak_meter_peek();
                sleep(0);   /* Sleep until end of current tick. */

                if (TIME_AFTER(current_tick, next_refresh)) {
                    FOR_NB_SCREENS(i)
                    {
                        if(gui_wps[i].data->peak_meter_enabled)
                            gui_wps_redraw(&gui_wps[i], 0,
                                            WPS_REFRESH_PEAK_METER);
                        next_refresh += HZ / PEAK_METER_FPS;
                    }
                }
            }

        }

        /* The peak meter is disabled
           -> no additional screen updates needed */
        else
#endif
        {
            button = get_action(CONTEXT_WPS|ALLOW_SOFTLOCK,
                restore ? HZ/100 : HZ/5);
        }

        /* Exit if audio has stopped playing. This happens e.g. at end of
           playlist or if using the sleep timer. */
        if (!(audio_status() & AUDIO_STATUS_PLAY))
            exit = true;
#ifdef HAVE_TOUCHSCREEN
        if (button == ACTION_TOUCHSCREEN)
            button = wps_get_touchaction(gui_wps[SCREEN_MAIN].data);
#endif
/* The iPods/X5/M5 use a single button for the A-B mode markers,
   defined as ACTION_WPSAB_SINGLE in their config files. */
#ifdef ACTION_WPSAB_SINGLE
        if (!global_settings.party_mode && ab_repeat_mode_enabled())
        {
            static int wps_ab_state = 0;
            if (button == ACTION_WPSAB_SINGLE)
            {
                switch (wps_ab_state)
                {
                    case 0: /* set the A spot */
                        button = ACTION_WPS_ABSETA_PREVDIR;
                        break;
                    case 1: /* set the B spot */
                        button = ACTION_WPS_ABSETB_NEXTDIR;
                        break;
                    case 2:
                        button = ACTION_WPS_ABRESET;
                        break;
                }
                wps_ab_state = (wps_ab_state+1) % 3;
            }
        }
#endif
        switch(button)
        {
            case ACTION_WPS_CONTEXT:
            {
                gwps_leave_wps();
                /* if music is stopped in the context menu we want to exit the wps */
                if (onplay(wps_state.id3->path, 
                           FILE_ATTR_AUDIO, CONTEXT_WPS) == ONPLAY_MAINMENU 
                    || !audio_status())
                    return GO_TO_ROOT;
                restore = true;
            }
            break;

            case ACTION_WPS_BROWSE:
#ifdef HAVE_LCD_CHARCELLS
                status_set_record(false);
                status_set_audio(false);
#endif
                gwps_leave_wps();
                return GO_TO_PREVIOUS_BROWSER;
                break;

                /* play/pause */
            case ACTION_WPS_PLAY:
                if (global_settings.party_mode)
                    break;
                if ( wps_state.paused )
                {
                    wps_state.paused = false;
                    if ( global_settings.fade_on_stop )
                        fade(true, true);
                    else
                        audio_resume();
                }
                else
                {
                    wps_state.paused = true;
                    if ( global_settings.fade_on_stop )
                        fade(false, true);
                    else
                        audio_pause();
                    settings_save();
#if !defined(HAVE_RTC_RAM) && !defined(HAVE_SW_POWEROFF)
                    call_storage_idle_notifys(true);   /* make sure resume info is saved */
#endif
                }
                break;

            case ACTION_WPS_VOLUP:
            {
                FOR_NB_SCREENS(i)
                    gui_wps[i].data->button_time_volume = current_tick;
                global_settings.volume++;
                bool res = false;
                setvol();
                FOR_NB_SCREENS(i)
                {
                    if(update_onvol_change(&gui_wps[i]))
                        res = true;
                }
                if (res) {
                    restore = true;
                    restoretimer = RESTORE_WPS_NEXT_SECOND;
                }
            }
            break;
            case ACTION_WPS_VOLDOWN:
            {
                FOR_NB_SCREENS(i)
                    gui_wps[i].data->button_time_volume = current_tick;
                global_settings.volume--;
                setvol();
                bool res = false;
                FOR_NB_SCREENS(i)
                {
                    if(update_onvol_change(&gui_wps[i]))
                        res = true;
                }
                if (res) {
                    restore = true;
                    restoretimer = RESTORE_WPS_NEXT_SECOND;
                }
            }
                break;
            /* fast forward 
                OR next dir if this is straight after ACTION_WPS_SKIPNEXT */
            case ACTION_WPS_SEEKFWD:
                if (global_settings.party_mode)
                    break;
                if (current_tick -last_right < HZ)
                {
                    if (cuesheet_is_enabled() && wps_state.id3->cuesheet_type)
                    {
                        audio_next();
                    }
                    else
                    {
                        change_dir(1);
                    }
                }
                else
                    ffwd_rew(ACTION_WPS_SEEKFWD);
                last_right = last_left = 0;
                break;
            /* fast rewind 
                OR prev dir if this is straight after ACTION_WPS_SKIPPREV,*/
            case ACTION_WPS_SEEKBACK:
                if (global_settings.party_mode)
                    break;
                if (current_tick -last_left < HZ)
                {
                    if (cuesheet_is_enabled() && wps_state.id3->cuesheet_type)
                    {
                        if (!wps_state.paused)
#if (CONFIG_CODEC == SWCODEC)
                            audio_pre_ff_rewind();
#else
                            audio_pause();
#endif
                        audio_ff_rewind(0);
                    }
                    else
                    {
                        change_dir(-1);
                    }
                }
                else
                    ffwd_rew(ACTION_WPS_SEEKBACK);
                last_left = last_right = 0;
                break;

                /* prev / restart */
            case ACTION_WPS_SKIPPREV:
                if (global_settings.party_mode)
                    break;
                last_left = current_tick;
#ifdef AB_REPEAT_ENABLE
                /* if we're in A/B repeat mode and the current position
                   is past the A marker, jump back to the A marker... */
                if ( ab_repeat_mode_enabled() )
                {
                    if ( ab_after_A_marker(wps_state.id3->elapsed) )
                    {
                        ab_jump_to_A_marker();
                        break;
#if (AB_REPEAT_ENABLE == 2)
                    } else {
                        ab_reset_markers();
#endif
                    }
                }
                else
                /* ...otherwise, do it normally */
#endif
                    play_hop(-1);
                break;

                /* next
                   OR if skip length set, hop by predetermined amount. */
            case ACTION_WPS_SKIPNEXT:
                if (global_settings.party_mode)
                    break;
                last_right = current_tick;
#ifdef AB_REPEAT_ENABLE
                /* if we're in A/B repeat mode and the current position is
                   before the A marker, jump to the A marker... */
                if ( ab_repeat_mode_enabled() )
                {
                    if ( ab_before_A_marker(wps_state.id3->elapsed) )
                    {
                        ab_jump_to_A_marker();
                        break;
#if (AB_REPEAT_ENABLE == 2)
                    } else {
                        ab_reset_markers();
#endif
                    }
                }
                else
                /* ...otherwise, do it normally */
#endif
                    play_hop(1);
                break;
                /* next / prev directories */
                /* and set A-B markers if in a-b mode */
            case ACTION_WPS_ABSETB_NEXTDIR:
                if (global_settings.party_mode)
                    break;
#if defined(AB_REPEAT_ENABLE)
                if (ab_repeat_mode_enabled())
                {
                    ab_set_B_marker(wps_state.id3->elapsed);
                    ab_jump_to_A_marker();
                }
                else
#endif
                {
                    change_dir(1);
                }
                break;
            case ACTION_WPS_ABSETA_PREVDIR:
                if (global_settings.party_mode)
                    break;
#if defined(AB_REPEAT_ENABLE)
                if (ab_repeat_mode_enabled())
                    ab_set_A_marker(wps_state.id3->elapsed);
                else
#endif
                {
                    change_dir(-1);
                }
                break;
            /* menu key functions */
            case ACTION_WPS_MENU:
                gwps_leave_wps();
                return GO_TO_ROOT;
                break;


#ifdef HAVE_QUICKSCREEN
            case ACTION_WPS_QUICKSCREEN:
            {
                gwps_leave_wps();
                if (quick_screen_quick(button))
                    return SYS_USB_CONNECTED;
                restore = true;
            }
            break;
#endif /* HAVE_QUICKSCREEN */

                /* screen settings */
#ifdef BUTTON_F3
            case ACTION_F3:
            {
                gwps_leave_wps();
                if (quick_screen_f3(BUTTON_F3))
                    return SYS_USB_CONNECTED;
                restore = true;
            }
            break;
#endif /* BUTTON_F3 */

                /* pitch screen */
#ifdef HAVE_PITCHSCREEN
            case ACTION_WPS_PITCHSCREEN:
            {
                gwps_leave_wps();
                if (1 == gui_syncpitchscreen_run())
                    return SYS_USB_CONNECTED;
                restore = true;
            }
            break;
#endif /* HAVE_PITCHSCREEN */

#ifdef AB_REPEAT_ENABLE
            /* reset A&B markers */
            case ACTION_WPS_ABRESET:
                if (ab_repeat_mode_enabled())
                {
                    ab_reset_markers();
                    update = true;
                }
                break;
#endif /* AB_REPEAT_ENABLE */

                /* stop and exit wps */
            case ACTION_WPS_STOP:
                if (global_settings.party_mode)
                    break;
                bookmark = true;
                exit = true;
                break;

            case ACTION_WPS_ID3SCREEN:
            {
                gwps_leave_wps();
                browse_id3();
                restore = true;
            }
            break;
#ifdef HAVE_TOUCHSCREEN
            case ACTION_TOUCH_SHUFFLE: /* toggle shuffle mode */
            {
                global_settings.playlist_shuffle = 
                                                !global_settings.playlist_shuffle;
#if CONFIG_CODEC == SWCODEC
                dsp_set_replaygain();
#endif
                if (global_settings.playlist_shuffle)
                    playlist_randomise(NULL, current_tick, true);
                else
                    playlist_sort(NULL, true);
            }
            break;
            case ACTION_TOUCH_REPMODE: /* cycle the repeat mode setting */
            {
                const struct settings_list *rep_setting = 
                                find_setting(&global_settings.repeat_mode, NULL);
                option_select_next_val(rep_setting, false, true);
                audio_flush_and_reload_tracks();
            }
            break;
#endif /* HAVE_TOUCHSCREEN */            
            case ACTION_REDRAW: /* yes are locked, just redraw */
                /* fall througgh */
            case ACTION_NONE: /* Timeout, do an partial update */
                update = true;
                ffwd_rew(button); /* hopefully fix the ffw/rwd bug */
                break;
#ifdef HAVE_RECORDING
            case ACTION_WPS_REC:
                exit = true;
                break;
#endif
            case SYS_POWEROFF:
                default_event_handler(SYS_POWEROFF);
                break;

            default:
                if(default_event_handler(button) == SYS_USB_CONNECTED)
                    return GO_TO_ROOT;
                update = true;
                break;
        }

        if (wps_state.do_full_update || update)
        {
#if defined(HAVE_BACKLIGHT) || defined(HAVE_REMOTE_LCD)
            gwps_caption_backlight(&wps_state);
#endif
            FOR_NB_SCREENS(i)
            {
#if defined(HAVE_LCD_ENABLE) || defined(HAVE_LCD_SLEEP)
                if (lcd_active()
#ifdef HAVE_REMOTE_LCD
                /* currently, all remotes are readable without backlight
                 * so still update those */
                            || (i == SCREEN_REMOTE)
#endif
                                                )
#endif
                {
                    gui_wps_update(&gui_wps[i]);
                }
            }
            wps_state.do_full_update = false;
            update = false;
        }

        if (restore && wps_state.id3 &&
            ((restoretimer == RESTORE_WPS_INSTANTLY) ||
             TIME_AFTER(current_tick, restoretimer)))
        {
            restore = false;
            restoretimer = RESTORE_WPS_INSTANTLY;
            gwps_fix_statusbars();
#if defined(HAVE_LCD_ENABLE) || defined(HAVE_LCD_SLEEP)
            lcd_activation_set_hook(wps_lcd_activation_hook);
#endif
            FOR_NB_SCREENS(i)
            {
                screens[i].stop_scroll();
                gui_wps_display(&gui_wps[i]);
            }
        }

        if (exit) {
#ifdef HAVE_LCD_CHARCELLS
            status_set_record(false);
            status_set_audio(false);
#endif
            if (global_settings.fade_on_stop)
                fade(false, true);

            if (bookmark)
                bookmark_autobookmark();
            audio_stop();
#ifdef AB_REPEAT_ENABLE
            ab_reset_markers();
#endif
            gwps_leave_wps();
#ifdef HAVE_RECORDING
            if (button == ACTION_WPS_REC)
                return GO_TO_RECSCREEN;
#endif
            if (global_settings.browse_current)
                return GO_TO_PREVIOUS_BROWSER;
            return GO_TO_PREVIOUS;
        }

        if (button && !IS_SYSEVENT(button) )
            storage_spin();
    }
    return GO_TO_ROOT; /* unreachable - just to reduce compiler warnings */
}

/* this is called from the playback thread so NO DRAWING! */
static void track_changed_callback(void *param)
{
    wps_state.id3 = (struct mp3entry*)param;
    wps_state.nid3 = audio_next_track();
    
    if (cuesheet_is_enabled() && wps_state.id3->cuesheet_type
        && strcmp(wps_state.id3->path, curr_cue->audio_filename))
    {
        /* the current cuesheet isn't the right one any more */
        /* We need to parse the new cuesheet */
        char cuepath[MAX_PATH];

        if (look_for_cuesheet_file(wps_state.id3->path, cuepath) &&
            parse_cuesheet(cuepath, curr_cue))
        {
            wps_state.id3->cuesheet_type = 1;
            strcpy(curr_cue->audio_filename, wps_state.id3->path);
        }

        cue_spoof_id3(curr_cue, wps_state.id3);
    }
    wps_state.do_full_update = true;
}
static void nextid3available_callback(void* param)
{
    (void)param;
    wps_state.nid3 = audio_next_track();
    wps_state.do_full_update = true;
}

/* wps_state */

static void wps_state_init(void)
{
    wps_state.ff_rewind = false;
    wps_state.paused = false;
    if(audio_status() & AUDIO_STATUS_PLAY)
    {
        wps_state.id3 = audio_current_track();
        wps_state.nid3 = audio_next_track();
    }
    else
    {
        wps_state.id3 = NULL;
        wps_state.nid3 = NULL;
    }
    /* We'll be updating due to restore initialized with true */
    wps_state.do_full_update = false;
    /* add the WPS track event callbacks */
    add_event(PLAYBACK_EVENT_TRACK_CHANGE, false, track_changed_callback);
    add_event(PLAYBACK_EVENT_NEXTTRACKID3_AVAILABLE, false, nextid3available_callback);
}

/* wps_state end*/

#ifdef HAVE_LCD_BITMAP
static void statusbar_toggle_handler(void *data)
{
    (void)data;
    int i;
    gwps_fix_statusbars();

    FOR_NB_SCREENS(i)
    {
        struct viewport *vp = &gui_wps[i].data->viewports[0].vp;
        bool draw = wpsbars & (VP_SB_ONSCREEN(i) | VP_SB_IGNORE_SETTING(i));
        if (!draw)
        {
            vp->y = 0;
            vp->height = screens[i].lcdheight;
        }
        else
        {
            vp->y      = STATUSBAR_HEIGHT;
            vp->height = screens[i].lcdheight - STATUSBAR_HEIGHT;
        }
    }
}
#endif

void gui_sync_wps_init(void)
{
    int i;
    FOR_NB_SCREENS(i)
    {
        wps_data_init(&wps_datas[i]);
#ifdef HAVE_ALBUMART
        wps_datas[i].wps_uses_albumart = 0;
#endif
#ifdef HAVE_REMOTE_LCD
        wps_datas[i].remote_wps = (i != 0);
#endif
        gui_wps[i].data = &wps_datas[i];
        gui_wps[i].display = &screens[i];
        /* Currently no seperate wps_state needed/possible
           so use the only aviable ( "global" ) one */
        gui_wps[i].state = &wps_state;
    }
#ifdef HAVE_LCD_BITMAP
    add_event(GUI_EVENT_STATUSBAR_TOGGLE, false, statusbar_toggle_handler);
#endif
#if LCD_DEPTH > 1
    unload_wps_backdrop();
#endif
#if defined(HAVE_REMOTE_LCD) && LCD_REMOTE_DEPTH > 1
    unload_remote_wps_backdrop();
#endif
}

#ifdef HAVE_ALBUMART
/* Returns true if at least one of the gui_wps screens has an album art
   tag in its wps structure */
bool gui_sync_wps_uses_albumart(void)
{
    int  i;
    FOR_NB_SCREENS(i) {
        struct gui_wps *gwps = &gui_wps[i];
        if (gwps->data && (gwps->data->wps_uses_albumart != WPS_ALBUMART_NONE))
            return true;
    }
    return false;
}
#endif
