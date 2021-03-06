/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2006,2007 by Greg White
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
#ifndef MMU_ARM_H
#define MMU_ARM_H

#define CACHE_ALL   0x0C
#define CACHE_NONE  0
#define BUFFERED    0x04

void memory_init(void);
void ttb_init(void);
void enable_mmu(void);
void map_section(unsigned int pa, unsigned int va, int mb, int flags);

/* Cleans entire DCache */
void clean_dcache(void);

/* Invalidate entire DCache */
/* will do writeback */
void invalidate_dcache(void);

/* Invalidate DCache for this range  */
/* will do writeback */
void invalidate_dcache_range(const void *base, unsigned int size);

/* clean DCache for this range  */
/* forces DCache writeback for the specified range */
void clean_dcache_range(const void *base, unsigned int size);

/* Dump DCache for this range  */
/* Will *NOT* do write back except for buffer ends not on a line boundary */
void dump_dcache_range(const void *base, unsigned int size);

/* Invalidate entire ICache and DCache */
/* will do writeback */
void invalidate_idcache(void);

#define HAVE_CPUCACHE_INVALIDATE
#define HAVE_CPUCACHE_FLUSH

#endif /* MMU_ARM_H */
