/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2007 Rob Purchase
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#ifndef __TCC780X_H__
#define __TCC780X_H__

/* General-purpose IO */

#define PORTCFG0    (*(volatile unsigned long *)0xF005A000)
#define PORTCFG1    (*(volatile unsigned long *)0xF005A004)
#define PORTCFG2    (*(volatile unsigned long *)0xF005A008)
#define PORTCFG3    (*(volatile unsigned long *)0xF005A00C)

#define GPIOA       (*(volatile unsigned long *)0xF005A020)
#define GPIOB       (*(volatile unsigned long *)0xF005A040)
#define GPIOC       (*(volatile unsigned long *)0xF005A060)
#define GPIOD       (*(volatile unsigned long *)0xF005A080)
#define GPIOE       (*(volatile unsigned long *)0xF005A0A0)

#define GPIOA_DIR   (*(volatile unsigned long *)0xF005A024)
#define GPIOB_DIR   (*(volatile unsigned long *)0xF005A044)
#define GPIOC_DIR   (*(volatile unsigned long *)0xF005A064)
#define GPIOD_DIR   (*(volatile unsigned long *)0xF005A084)
#define GPIOE_DIR   (*(volatile unsigned long *)0xF005A0A4)

#define GPIOA_SET   (*(volatile unsigned long *)0xF005A028)
#define GPIOB_SET   (*(volatile unsigned long *)0xF005A048)
#define GPIOC_SET   (*(volatile unsigned long *)0xF005A068)
#define GPIOD_SET   (*(volatile unsigned long *)0xF005A088)
#define GPIOE_SET   (*(volatile unsigned long *)0xF005A0A8)

#define GPIOA_CLEAR (*(volatile unsigned long *)0xF005A02C)
#define GPIOB_CLEAR (*(volatile unsigned long *)0xF005A04C)
#define GPIOC_CLEAR (*(volatile unsigned long *)0xF005A06C)
#define GPIOD_CLEAR (*(volatile unsigned long *)0xF005A08C)
#define GPIOE_CLEAR (*(volatile unsigned long *)0xF005A0AC)

/* Clock Generator */

#define CLKCTRL    (*(volatile unsigned long *)0xF3000000)
#define PLL0CFG    (*(volatile unsigned long *)0xF3000004)
#define PLL1CFG    (*(volatile unsigned long *)0xF3000008)
#define CLKDIVC    (*(volatile unsigned long *)0xF300000C)
#define CLKDIVC1   (*(volatile unsigned long *)0xF3000010)
#define MODECTR    (*(volatile unsigned long *)0xF3000014)
#define BCLKCTR    (*(volatile unsigned long *)0xF3000018)
#define SWRESET    (*(volatile unsigned long *)0xF300001C)
#define PCLKCFG0   (*(volatile unsigned long *)0xF3000020)
#define PCLKCFG1   (*(volatile unsigned long *)0xF3000024)
#define PCLKCFG2   (*(volatile unsigned long *)0xF3000028)
#define PCLKCFG3   (*(volatile unsigned long *)0xF300002C)
#define PCLK_LCD   (*(volatile unsigned long *)0xF3000030)
#define PCLKCFG5   (*(volatile unsigned long *)0xF3000034)
#define PCLKCFG6   (*(volatile unsigned long *)0xF3000038)
#define PCLKCFG7   (*(volatile unsigned long *)0xF300003C)
#define PCLKCFG8   (*(volatile unsigned long *)0xF3000040)
#define PCLK_TCT   (*(volatile unsigned long *)0xF3000044)
#define PCLKCFG10  (*(volatile unsigned long *)0xF3000048)
#define PCLKCFG11  (*(volatile unsigned long *)0xF300004C)
#define PCLK_ADC   (*(volatile unsigned long *)0xF3000050)
#define PCLKCFG13  (*(volatile unsigned long *)0xF3000054)
#define PCLKCFG14  (*(volatile unsigned long *)0xF3000058)
#define PCLK_RFREQ (*(volatile unsigned long *)0xF300005C)
#define PCLKCFG16  (*(volatile unsigned long *)0xF3000060)
#define PCLKCFG17  (*(volatile unsigned long *)0xF3000064)

#define PCK_EN (1<<28)

#define CKSEL_PLL0 0
#define CKSEL_PLL1 1
#define CKSEL_XIN  4

/* IRQ Controller */

#define IEN     (*(volatile unsigned long *)0xF3001000)
#define CREQ    (*(volatile unsigned long *)0xF3001004)
#define IRQSEL  (*(volatile unsigned long *)0xF300100C)
#define MREQ    (*(volatile unsigned long *)0xF3001014)
#define MIRQ    (*(volatile unsigned long *)0xF3001028)
#define MFIQ    (*(volatile unsigned long *)0xF300102C)
#define ALLMASK (*(volatile unsigned long *)0xF3001044)
#define VAIRQ   (*(volatile unsigned long *)0xF3001080)
#define VAFIQ   (*(volatile unsigned long *)0xF3001084)
#define VNIRQ   (*(volatile unsigned long *)0xF3001088)
#define VNFIQ   (*(volatile unsigned long *)0xF300108C)

#define TIMER_IRQ_MASK (1<<6)

/* Timer / Counters */

#define TCFG0      (*(volatile unsigned long *)0xF3003000)
#define TCNT0      (*(volatile unsigned long *)0xF3003004)
#define TREF0      (*(volatile unsigned long *)0xF3003008)
#define TCFG1      (*(volatile unsigned long *)0xF3003010)
#define TCNT1      (*(volatile unsigned long *)0xF3003014)
#define TREF1      (*(volatile unsigned long *)0xF3003018)

#define TIREQ      (*(volatile unsigned long *)0xF3003060)
/* ref. value reached */
#define TF0 (1<<8)
#define TF1 (1<<9)
/* irq. status */
#define TI0 (1<<0)
#define TI1 (1<<1)

#define TC32EN     (*(volatile unsigned long *)0xF3003080)
#define TC32LDV    (*(volatile unsigned long *)0xF3003084)
#define TC32MCNT   (*(volatile unsigned long *)0xF3003094)
#define TC32IRQ    (*(volatile unsigned long *)0xF3003098)

/* ADC */

#define ADCCON     (*(volatile unsigned long *)0xF3004000)
#define ADCDATA    (*(volatile unsigned long *)0xF3004004)
#define ADCCONA    (*(volatile unsigned long *)0xF3004080)
#define ADCSTATUS  (*(volatile unsigned long *)0xF3004084)
#define ADCCFG     (*(volatile unsigned long *)0xF3004088)

/* Memory Controller */

#define SDCFG      (*(volatile unsigned long *)0xF1000000)
#define SDFSM      (*(volatile unsigned long *)0xF1000004)
#define MCFG       (*(volatile unsigned long *)0xF1000008)
#define CSCFG0     (*(volatile unsigned long *)0xF1000010)
#define CSCFG1     (*(volatile unsigned long *)0xF1000014)
#define CSCFG2     (*(volatile unsigned long *)0xF1000018)
#define CSCFG3     (*(volatile unsigned long *)0xF100001C)
#define CLKCFG     (*(volatile unsigned long *)0xF1000020)
#define SDCMD      (*(volatile unsigned long *)0xF1000024)

#define SDCFG1     (*(volatile unsigned long *)0xF1001000)
#define MCFG1      (*(volatile unsigned long *)0xF1001008)

/* Misc */

#define ECFG0      (*(volatile unsigned long *)0xF300500C)
#define MBCFG      (*(volatile unsigned long *)0xF3005020)

#define TCC780_VER (*(volatile unsigned long *)0xE0001FFC)

#endif