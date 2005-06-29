/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Miika Pekkarinen
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef _PLAYBACK_H
#define _PLAYBACK_H

#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "id3.h"
#include "mp3data.h"

enum {
    CODEC_SET_FILEBUF_WATERMARK = 1,
    CODEC_SET_FILEBUF_CHUNKSIZE,
    CODEC_SET_FILEBUF_LIMIT,
    CODEC_DSP_ENABLE,
    DSP_SET_FREQUENCY,
    DSP_SET_CLIP_MIN,
    DSP_SET_CLIP_MAX,
    DSP_SET_SAMPLE_DEPTH,
    DSP_SET_STEREO_MODE,
    DSP_RESET,
    DSP_DITHER
};

/* Not yet implemented. */
#define CODEC_SET_AUDIOBUF_WATERMARK    4

#define MAX_TRACK 10
struct track_info {
    struct mp3entry id3;     /* TAG metadata */
    struct mp3info mp3data;  /* MP3 metadata */
    char *codecbuf;          /* Pointer to codec buffer */
    long codecsize;        /* Codec length in bytes */
    
    off_t filerem;           /* Remaining bytes of file NOT in buffer */
    off_t filesize;          /* File total length */
    off_t filepos;           /* Read position of file for next buffer fill */
    off_t start_pos;         /* Position to first bytes of file in buffer */
    volatile int available;  /* Available bytes to read from buffer */
    bool taginfo_ready;      /* Is metadata read */
    int playlist_offset;     /* File location in playlist */
};

/* Functions */
void audio_set_track_changed_event(void (*handler)(struct track_info *ti));

#endif


