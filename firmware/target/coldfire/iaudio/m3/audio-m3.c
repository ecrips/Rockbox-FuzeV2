/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2006 by Michael Sevakis
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include "system.h"
#include "cpu.h"
#include "audio.h"
#include "sound.h"

void audio_set_output_source(int source)
{
    int level = set_irq_level(DMA_IRQ_LEVEL);
    unsigned long txsrc;

    if ((unsigned)source >= AUDIO_NUM_SOURCES)
        txsrc = (3 << 8); /* playback, PDOR3 */
    else
        txsrc = (4 << 8); /* recording, iis1RcvData */

    IIS1CONFIG = (IIS1CONFIG & ~(7 << 8)) | txsrc;
    set_irq_level(level);
} /* audio_set_output_source */

void audio_input_mux(int source, unsigned flags)
{
    /* Prevent pops from unneeded switching */
    static int last_source = AUDIO_SRC_PLAYBACK;
    static bool last_recording = false;

    bool recording = flags & SRCF_RECORDING;

    switch (source)
    {
        default:                            /* playback - no recording */
            source = AUDIO_SRC_PLAYBACK;
        case AUDIO_SRC_PLAYBACK:
            if (source != last_source)
            {
                audiohw_disable_recording();
                audiohw_set_monitor(false);
                coldfire_set_dataincontrol(0);
            }
        break;

        case AUDIO_SRC_MIC:                 /* recording only */
            if (source != last_source)
            {
                audiohw_enable_recording(true);  /* source mic */
                /* Int. when 6 samples in FIFO, PDIR2 src = iis1RcvData */
                coldfire_set_dataincontrol((3 << 14) | (4 << 3));
            }
        break;

        case AUDIO_SRC_LINEIN:              /* recording only */
            if (source != last_source)
            {
                audiohw_enable_recording(false); /* source line */
                /* Int. when 6 samples in FIFO, PDIR2 src = iis1RcvData */
                coldfire_set_dataincontrol((3 << 14) | (4 << 3));
            }
        break;

        case AUDIO_SRC_FMRADIO:             /* recording and playback */
            if (!recording)
                audiohw_set_recvol(23, 23, AUDIO_GAIN_LINEIN);

            /* I2S recording and analog playback */
            if (source == last_source && recording == last_recording)
                break;

            last_recording = recording;

            if (recording)
            {
                /* Int. when 6 samples in FIFO, PDIR2 src = iis1RcvData */
                coldfire_set_dataincontrol((3 << 14) | (4 << 3));
                audiohw_enable_recording(false); /* source line */
            }
            else
            {
                audiohw_disable_recording();
                audiohw_set_monitor(true);       /* analog bypass */
                coldfire_set_dataincontrol(0);
            }
        break;
    } /* end switch */

    /* set line multiplexer */
    if (source == AUDIO_SRC_FMRADIO)
        and_l(~(1 << 25), &GPIO1_OUT);   /* FM radio */
    else
        or_l((1 << 25), &GPIO1_OUT);     /* Line In */

    or_l((1 << 25), &GPIO1_ENABLE);
    or_l((1 << 25), &GPIO1_FUNCTION);

    last_source = source;
} /* audio_input_mux */
