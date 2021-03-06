/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2008 by Maurus Cuelenaere
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

#include "system.h"
#include "kernel.h"
#include "logf.h"
#include "audio.h"
#include "sound.h"
#include "pcm.h"
#include "jz4740.h"


/****************************************************************************
 ** Playback DMA transfer
 **/

static bool playback_started = false;
static void* playback_address;
void pcm_postinit(void)
{
    audiohw_postinit();

    /* playback sample: 16 bits burst: 16 bytes */
    __i2s_set_iss_sample_size(16);
    __i2s_set_oss_sample_size(16);
    __i2s_set_transmit_trigger(10);
    __i2s_set_receive_trigger(1);

    /* Flush FIFO */
    __aic_flush_fifo();
}

void pcm_play_dma_init(void)
{
    /* TODO */

    system_enable_irq(DMA_IRQ(DMA_AIC_TX_CHANNEL));

    /* Initialize default register values. */
    audiohw_init();
}

void pcm_dma_apply_settings(void)
{
    /* TODO */
    audiohw_set_frequency(pcm_sampr);
}

static void play_start_pcm(void)
{
    __aic_enable_transmit_dma();
    __aic_enable_replay();

    REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) |= DMAC_DCCSR_EN;

    playback_started = true;
}

static inline void set_dma(const void *addr, size_t size)
{
    logf("%x %x %d %d %x", (unsigned int)addr, size, (REG_AIC_SR>>24) & 0x20, (REG_AIC_SR>>8) & 0x20, REG_AIC_SR & 0xF);

    //__dcache_writeback_all();
    REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) = DMAC_DCCSR_NDES;
    REG_DMAC_DSAR(DMA_AIC_TX_CHANNEL)  = PHYSADDR((unsigned long)addr);
    REG_DMAC_DTAR(DMA_AIC_TX_CHANNEL)  = PHYSADDR((unsigned long)AIC_DR);
    REG_DMAC_DTCR(DMA_AIC_TX_CHANNEL)  = size / 16;
    REG_DMAC_DRSR(DMA_AIC_TX_CHANNEL)  = DMAC_DRSR_RS_AICOUT;
    REG_DMAC_DCMD(DMA_AIC_TX_CHANNEL)  = (DMAC_DCMD_SAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DS_16BYTE | DMAC_DCMD_DWDH_16 | DMAC_DCMD_TIE |
                                          DMAC_DCMD_RDIL_IGN);

    playback_address = (void*)addr;
}

static void play_stop_pcm(void)
{
    REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) = (REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) | DMAC_DCCSR_HLT) & ~DMAC_DCCSR_EN;

    dma_disable();

    __aic_disable_transmit_dma();
    __aic_disable_replay();

    playback_started = false;
}

void pcm_play_dma_start(const void *addr, size_t size)
{
    dma_enable();

    set_dma(addr, size);
    play_start_pcm();
}


static inline void play_dma_callback(void)
{
    unsigned char *start;
    size_t size = 0;
    pcm_callback_for_more(&start, &size);

    if(LIKELY(size > 0))
    {
        set_dma(start, size);
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) |= DMAC_DCCSR_EN;
    }
    else
    {
        /* Error, callback missing or no more DMA to do */
        pcm_play_dma_stop();
        pcm_play_dma_stopped_callback();
    }
}

void DMA_CALLBACK(DMA_AIC_TX_CHANNEL)(void)
{
    if (REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) & DMAC_DCCSR_AR)
    {
        logf("PCM DMA address error");
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) &= ~DMAC_DCCSR_AR;
    }

    if (REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) & DMAC_DCCSR_HLT)
    {
        logf("PCM DMA halt");
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) &= ~DMAC_DCCSR_HLT;
    }

    if (REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) & DMAC_DCCSR_TT)
    {
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) &= ~DMAC_DCCSR_TT;
        play_dma_callback();
    }
}

size_t pcm_get_bytes_waiting(void)
{
    if(playback_started)
        return (REG_DMAC_DTCR(DMA_AIC_TX_CHANNEL) * 16) & ~3;
    else
        return 0;
}

const void * pcm_play_dma_get_peak_buffer(int *count)
{
    /* TODO */
    if(playback_started)
    {
        *count = REG_DMAC_DTCR(DMA_AIC_TX_CHANNEL) >> 2;
        return (void*)(playback_address + ((REG_DMAC_DTCR(DMA_AIC_TX_CHANNEL)*16 + 2) & ~3));
    }
    else
    {
        *count = 0;
        return NULL;
    }
}

void pcm_play_dma_stop(void)
{
    play_stop_pcm();

    /* TODO */
}

void pcm_play_lock(void)
{
    /* TODO */
}

void pcm_play_unlock(void)
{
   /* TODO */
}

void pcm_play_dma_pause(bool pause)
{
    if(pause)
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) &= ~DMAC_DCCSR_EN;
    else
        REG_DMAC_DCCSR(DMA_AIC_TX_CHANNEL) |= DMAC_DCCSR_EN;
}

void audiohw_close(void)
{
}

#ifdef HAVE_RECORDING
/* TODO */
void pcm_rec_dma_init(void)
{
}

void pcm_rec_dma_close(void)
{
}

void pcm_rec_dma_start(void *addr, size_t size)
{
    (void) addr;
    (void) size;
}

void pcm_rec_dma_stop(void)
{
}

void pcm_rec_lock(void)
{
}

void pcm_rec_unlock(void)
{
}

const void * pcm_rec_dma_get_peak_buffer(int *count)
{
    *count = 0;
    return NULL;
}

void pcm_record_more(void *start, size_t size)
{
    (void) start;
    (void) size;
}
#endif
