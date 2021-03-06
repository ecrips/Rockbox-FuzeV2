CFLAGS=-Wall -W

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
OUTPUT=beastpatcher.exe
CROSS=
CFLAGS+=-mno-cygwin
else
ifeq ($(findstring MINGW,$(shell uname)),MINGW)
OUTPUT=beastpatcher.exe
CROSS=
else
OUTPUT=beastpatcher
CROSS=i586-mingw32msvc-
endif
endif

ifneq ($(findstring MINGW,$(shell uname)),MINGW)
LIBS = /usr/lib/libmtp.a /usr/lib/libusb.a
CFLAGS += $(shell printf \
	'\#include <libmtp.h>\nlibmtp version: LIBMTP_VERSION\n' | \
	gcc -E -P - -o - | grep -q '^libmtp version: 0\.2' && echo '-DOLDMTP')
endif
WINLIBS = ../MTP_DLL.dll

NATIVECC = gcc
CC = $(CROSS)gcc

all: $(OUTPUT)

beastpatcher: beastpatcher.c bootimg.c mtp_common.h mtp_libmtp.c
	gcc $(CFLAGS) -o beastpatcher beastpatcher.c bootimg.c mtp_libmtp.c $(LIBS)
	strip beastpatcher

beastpatcher.exe:  beastpatcher.c bootimg.c mtp_common.h mtp_win32.c $(WINLIBS)
	$(CC) $(CFLAGS) -o beastpatcher.exe beastpatcher.c bootimg.c mtp_win32.c $(WINLIBS)
	$(CROSS)strip beastpatcher.exe

beastpatcher-mac: beastpatcher-i386 beastpatcher-ppc
	lipo -create beastpatcher-ppc beastpatcher-i386 -output beastpatcher-mac

beastpatcher-i386:  beastpatcher.c  bootimg.c usb.h libusb-i386.a
	$(CC) -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -framework iokit -framework coreservices -arch i386 $(CFLAGS) -o beastpatcher-i386 beastpatcher.c bootimg.c -I. libusb-i386.a
	strip beastpatcher-i386

beastpatcher-ppc: beastpatcher.c bootimg.c usb.h libusb-ppc.a
	$(CC) -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -framework iokit -framework coreservices -arch ppc $(CFLAGS) -o beastpatcher-ppc beastpatcher.c bootimg.c -I. libusb-ppc.a
	strip beastpatcher-ppc

bin2c: ../../../rbutil/sansapatcher/bin2c.c
	$(NATIVECC) $(CFLAGS) -o bin2c ../../../rbutil/sansapatcher/bin2c.c

bootimg.c: bootloader.bin bin2c
	./bin2c bootloader.bin bootimg

clean:
	rm -f beastpatcher.exe beastpatcher-mac beastpatcher-i386 beastpatcher-ppc beastpatcher bin2c bootimg.c bootimg.h *~
