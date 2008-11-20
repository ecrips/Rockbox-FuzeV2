#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id: Makefile 19082 2008-11-10 23:54:24Z zagor $
#

ROCKBOY_SRCDIR = $(APPSDIR)/plugins/rockboy
ROCKBOY_OBJDIR = $(BUILDDIR)/apps/plugins/rockboy

ROCKBOY_SRC := $(call preprocess, $(ROCKBOY_SRCDIR)/SOURCES)
ROCKBOY_OBJ := $(call c2obj, $(ROCKBOY_SRC))
ROCKBOY_OBJ += $(ROCKBOY_OBJDIR)/sscanf.o

OTHER_SRC += $(ROCKBOY_SRC)

ifndef SIMVER
ifneq (,$(findstring RECORDER,$(TARGET)))
    ## archos recorder targets
    ROCKBOY_INLDS := $(ROCKBOY_SRCDIR)/archos.lds
    ROCKS += $(ROCKBOY_OBJDIR)/rockboy.ovl
else
    ### all other targets
    ROCKBOY_INLDS := $(APPSDIR)/plugins/plugin.lds
    ROCKS += $(ROCKBOY_OBJDIR)/rockboy.rock
endif
    ROCKBOY_OVLFLAGS = -T$(ROCKBOY_OUTLDS) -Wl,--gc-sections -Wl,-Map,$(ROCKBOY_OBJDIR)/$*.map
    ROCKBOY_OUTLDS = $(ROCKBOY_OBJDIR)/rockboy.lds
else
    ### simulator
    ROCKS += $(ROCKBOY_OBJDIR)/rockboy.rock
    ROCKBOY_OVLFLAGS = $(SHARED_FLAG) # <-- from Makefile
endif

$(ROCKBOY_OBJDIR)/sscanf.c: $(FIRMDIR)/common/sscanf.c
	$(SILENT)mkdir -p $(dir $@)
	$(call PRINTS,CP $<)cp $< $@

$(ROCKBOY_OBJDIR)/sscanf.o: $(ROCKBOY_OBJDIR)/sscanf.c

$(ROCKBOY_OUTLDS): $(ROCKBOY_INLDS) $(ROCKBOY_OBJ)
	$(call PRINTS,PP $(<F))$(call preprocess2file,$<,$@)

$(ROCKBOY_OBJDIR)/rockboy.rock: $(ROCKBOY_OBJ) $(ROCKBOY_OUTLDS) $(PLUGINBITMAPLIB)

$(ROCKBOY_OBJDIR)/rockboy.ovl: $(ROCKBOY_OBJ) $(ROCKBOY_OUTLDS) $(PLUGINBITMAPLIB)
	$(SILENT)$(CC) $(PLUGINFLAGS) -o $(ROCKBOY_OBJDIR)/$*.elf \
		$(filter %.o, $^) \
		$(filter %.a, $^) \
		-lgcc $(ROCKBOY_OVLFLAGS)
	$(call PRINTS,LD $(@F))$(OC) -O binary $(ROCKBOY_OBJDIR)/$*.elf $@