/***************************************************************************
*             __________               __   ___.
*   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
*   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
*   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
*   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
*                     \/            \/     \/    \/            \/
* $Id$
*
* Copyright (C) 2005 Jens Arnold
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

#include <stdbool.h>
#include "config.h"
#include "cpu.h"
#include "system.h"
#include "timer.h"
#include "logf.h"

static int timer_prio = -1;
void SHAREDBSS_ATTR (*pfn_timer)(void) = NULL;      /* timer callback */
void SHAREDBSS_ATTR (*pfn_unregister)(void) = NULL; /* unregister callback */
#ifdef CPU_COLDFIRE
static int base_prescale;
#elif defined CPU_PP || CONFIG_CPU == PNX0101
static long SHAREDBSS_ATTR cycles_new = 0;
#endif

#ifndef __TIMER_SET
/* Define these if not defined by target to make the #else cases compile
 * even if the target doesn't have them implemented. */
#define __TIMER_SET(cycles, set) false
#define __TIMER_REGISTER(reg_prio, unregister_callback, cycles, \
                         int_prio, timer_callback) false
#define __TIMER_UNREGISTER(...)
#endif

/* interrupt handler */
#if CONFIG_CPU == SH7034
void IMIA4(void) __attribute__((interrupt_handler));
void IMIA4(void)
{
    if (pfn_timer != NULL)
        pfn_timer();
    and_b(~0x01, &TSR4); /* clear the interrupt */
}
#elif defined CPU_COLDFIRE
void TIMER1(void) __attribute__ ((interrupt_handler));
void TIMER1(void)
{
    if (pfn_timer != NULL)
        pfn_timer();
    TER1 = 0xff; /* clear all events */
}
#elif CONFIG_CPU == AS3525
void INT_TIMER1(void)
{
    if (pfn_timer != NULL)
        pfn_timer();

    TIMER1_INTCLR = 0; /* clear interrupt */
}
#elif defined(CPU_PP)
void TIMER2(void)
{
    TIMER2_VAL; /* ACK interrupt */
    if (cycles_new > 0)
    {
        TIMER2_CFG = 0xc0000000 | (cycles_new - 1);
        cycles_new = 0;
    }
    if (pfn_timer != NULL)
    {
        cycles_new = -1;
        /* "lock" the variable, in case timer_set_period()
         * is called within pfn_timer() */
        pfn_timer();
        cycles_new = 0;
    }
}
#elif CONFIG_CPU == PNX0101
void TIMER1_ISR(void)
{
    if (cycles_new > 0)
    {
        TIMER1.load = cycles_new - 1;
        cycles_new = 0;
    }
    if (pfn_timer != NULL)
    {
        cycles_new = -1;
        /* "lock" the variable, in case timer_set_period()
         * is called within pfn_timer() */
        pfn_timer();
        cycles_new = 0;
    }
    TIMER1.clr = 1; /* clear the interrupt */
}
#endif /* CONFIG_CPU */

static bool timer_set(long cycles, bool start)
{
#if CONFIG_CPU == SH7034 || defined(CPU_COLDFIRE) || CONFIG_CPU == AS3525
    int phi = 0; /* bits for the prescaler */
    int prescale = 1;

#if CONFIG_CPU == SH7034 || defined(CPU_COLDFIRE)
#define PRESCALE_STEP 1
#else /* CONFIG_CPU == AS3525 */
#define PRESCALE_STEP 4
#endif

    while (cycles > 0x10000)
    {   /* work out the smallest prescaler that makes it fit */
#if CONFIG_CPU == SH7034 || CONFIG_CPU == AS3525
        phi++;
#endif
        prescale <<= PRESCALE_STEP;
        cycles >>= PRESCALE_STEP;
    }
#endif

#if CONFIG_CPU == PNX0101
    if (start)
    {
        if (pfn_unregister != NULL)
        {
            pfn_unregister();
            pfn_unregister = NULL;
        }
        TIMER1.ctrl &= ~0x80; /* disable the counter */
        TIMER1.ctrl |= 0x40;  /* reload after counting down to zero */
        TIMER1.ctrl &= ~0xc;  /* no prescaler */
        TIMER1.clr = 1;       /* clear an interrupt event */
    }
    if (start || (cycles_new == -1)) /* within isr, cycles_new is "locked" */
    {                                /* enable timer */
        TIMER1.load = cycles - 1;
        TIMER1.ctrl |= 0x80;        /* enable the counter */
    }
    else
        cycles_new = cycles;

    return true;
#elif CONFIG_CPU == SH7034
    if (prescale > 8)
        return false;

    if (start)
    {
        if (pfn_unregister != NULL)
        {
            pfn_unregister();
            pfn_unregister = NULL;
        }

        and_b(~0x10, &TSTR); /* Stop the timer 4 */
        and_b(~0x10, &TSNC); /* No synchronization */
        and_b(~0x10, &TMDR); /* Operate normally */

        TIER4 = 0xF9;        /* Enable GRA match interrupt */
    }

    TCR4 = 0x20 | phi;   /* clear at GRA match, set prescaler */
    GRA4 = (unsigned short)(cycles - 1);
    if (start || (TCNT4 >= GRA4))
        TCNT4 = 0;
    and_b(~0x01, &TSR4); /* clear an eventual interrupt */

    return true;
#elif CONFIG_CPU == AS3525
    /* XXX: 32 bits cycles could be used */
    if (prescale > 256 || cycles > 0x10000)
        return false;

    if (start)
    {
        if (pfn_unregister != NULL)
        {
            pfn_unregister();
            pfn_unregister = NULL;
        }
    }

    TIMER1_LOAD = TIMER1_BGLOAD = cycles;
    /* /!\ bit 4 (reserved) must not be modified
     * periodic mode, interrupt enabled, 16 bits counter */
    TIMER1_CONTROL = (TIMER1_CONTROL & (1<<4)) | 0xe0 | (phi<<2);
    return true;
#elif defined CPU_COLDFIRE
    if (prescale > 4096/CPUFREQ_MAX_MULT)
        return false;

    if (prescale > 256/CPUFREQ_MAX_MULT)
    {
        phi = 0x05;      /* prescale sysclk/16, timer enabled */
        prescale >>= 4;
    }
    else
        phi = 0x03;      /* prescale sysclk, timer enabled */

    base_prescale = prescale;
    prescale *= (cpu_frequency / CPU_FREQ);

    if (start)
    {
        if (pfn_unregister != NULL)
        {
            pfn_unregister();
            pfn_unregister = NULL;
        }
        phi &= ~1;       /* timer disabled at start */

        /* If it is already enabled, writing a 0 to the RST bit will clear
           the register, so we clear RST explicitly before writing the real
           data. */
        TMR1 = 0;
    }

    /* We are using timer 1 */
    TMR1 = 0x0018 | (unsigned short)phi | ((unsigned short)(prescale - 1) << 8);
    TRR1 = (unsigned short)(cycles - 1);
    if (start || (TCN1 >= TRR1))
        TCN1 = 0; /* reset the timer */
    TER1 = 0xff;  /* clear all events */

    return true;
#elif defined(CPU_PP)
    if (cycles > 0x20000000 || cycles < 2)
        return false;

    if (start)
    {
        if (pfn_unregister != NULL)
        {
            pfn_unregister();
            pfn_unregister = NULL;
        }
        CPU_INT_DIS = TIMER2_MASK;
        COP_INT_DIS = TIMER2_MASK;
    }
    if (start || (cycles_new == -1))  /* within isr, cycles_new is "locked" */
        TIMER2_CFG = 0xc0000000 | (cycles - 1);    /* enable timer */
    else
        cycles_new = cycles;

    return true;
#else
    return __TIMER_SET(cycles, start);
#endif /* CONFIG_CPU */
}

#ifdef CPU_COLDFIRE
void timers_adjust_prescale(int multiplier, bool enable_irq)
{
    /* tick timer */
    TMR0 = (TMR0 & 0x00ef)
         | ((unsigned short)(multiplier - 1) << 8)
         | (enable_irq ? 0x10 : 0);

    if (pfn_timer)
    {
        /* user timer */
        int prescale = base_prescale * multiplier;
        TMR1 = (TMR1 & 0x00ef)
             | ((unsigned short)(prescale - 1) << 8)
             | (enable_irq ? 0x10 : 0);
    }
}
#endif

/* Register a user timer, called every <cycles> TIMER_FREQ cycles */
bool timer_register(int reg_prio, void (*unregister_callback)(void),
                    long cycles, int int_prio, void (*timer_callback)(void)
                    IF_COP(, int core))
{
    if (reg_prio <= timer_prio || cycles == 0)
        return false;

#if CONFIG_CPU == SH7034
    if (int_prio < 1 || int_prio > 15)
        return false;
#endif

    if (!timer_set(cycles, true))
        return false;

    pfn_timer = timer_callback;
    pfn_unregister = unregister_callback;
    timer_prio = reg_prio;

#if CONFIG_CPU == SH7034
    IPRD = (IPRD & 0xFF0F) | int_prio << 4;  /* interrupt priority */
    or_b(0x10, &TSTR); /* start timer 4 */
    return true;
#elif defined CPU_COLDFIRE
    ICR2 = 0x90;       /* interrupt on level 4.0 */
    and_l(~(1<<10), &IMR);
    TMR1 |= 1;         /* start timer */
    return true;
#elif defined(CPU_PP)
    /* unmask interrupt source */
#if NUM_CORES > 1
    if (core == COP)
        COP_INT_EN = TIMER2_MASK;
    else
#endif
        CPU_INT_EN = TIMER2_MASK;
    return true;
#elif CONFIG_CPU == PNX0101
    irq_set_int_handler(IRQ_TIMER1, TIMER1_ISR);
    irq_enable_int(IRQ_TIMER1);
    return true;
#elif CONFIG_CPU == AS3525
    CGU_PERI |= CGU_TIMER1_CLOCK_ENABLE;    /* enable peripheral */
    VIC_INT_ENABLE |= INTERRUPT_TIMER1;
    return true;
#else
    return __TIMER_REGISTER(reg_prio, unregister_callback, cycles,
                            int_prio, timer_callback);
#endif
    /* Cover for targets that don't use all these */
    (void)reg_prio;
    (void)unregister_callback;
    (void)cycles;
    /* TODO: Implement for PortalPlayer and iFP (if possible) */
    (void)int_prio;
    (void)timer_callback;
}

bool timer_set_period(long cycles)
{
    return timer_set(cycles, false);
}

void timer_unregister(void)
{
#if CONFIG_CPU == SH7034
    and_b(~0x10, &TSTR);    /* stop the timer 4 */
    IPRD = (IPRD & 0xFF0F); /* disable interrupt */
#elif defined CPU_COLDFIRE
    TMR1 = 0;               /* disable timer 1 */
    or_l((1<<10), &IMR);    /* disable interrupt */
#elif defined(CPU_PP)
    TIMER2_CFG = 0;         /* stop timer 2 */
    CPU_INT_DIS = TIMER2_MASK;
    COP_INT_DIS = TIMER2_MASK;
#elif CONFIG_CPU == PNX0101
    TIMER1.ctrl &= ~0x80;  /* disable timer 1 */
    irq_disable_int(IRQ_TIMER1);
#elif CONFIG_CPU == AS3525
    TIMER1_CONTROL &= 0x10; /* disable timer 1 (don't modify bit 4) */
    VIC_INT_EN_CLEAR = INTERRUPT_TIMER1;  /* disable interrupt */
    CGU_PERI &= ~CGU_TIMER1_CLOCK_ENABLE;   /* disable peripheral */
#else
    __TIMER_UNREGISTER();
#endif
    pfn_timer = NULL;
    pfn_unregister = NULL;
    timer_prio = -1;
}

