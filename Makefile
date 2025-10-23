# By Pierre Sarrazin <http://sarrazip.com/>
# This file is in the public domain.

PACKAGE = hirestxt
VERSION = 0.5.0

TARGET = coco

# The OS-9 demo for this library uses the bcontrol library.
ifeq "$(TARGET)" "os9"
BCONTROL_LIB = bcontrol
BCONTROL_LIB_DIR = ../bcontrol
DEMO_EXTRA_CFLAGS = -I $(BCONTROL_LIB_DIR)
DEMO_EXTRA_LDFLAGS = -L $(BCONTROL_LIB_DIR) -l$(BCONTROL_LIB)
endif

LIBSRC = \
	animateCursor.c \
	clear.c \
	clearRowsN.c \
	clearRowsToEOS.c \
	closeHiResTextScreen.c \
	clrscr.c \
	clrtobot.c \
	clrtoeol.c \
	fillNybbles.c \
	getCursorColumn.c \
	getCursorRow.c \
	init.c \
	initHiResTextScreen.c \
	initHiResTextScreen2.c \
	initVT52.c \
	invertPixelsAtCursor.c \
	moveCursor.c \
	processConsoleOutChar.c \
	putBitmaskInScreenWord.c \
	removeCursor.c \
	scrollTextScreenUp.c \
	setForegroundColor.c \
	setForegroundBoldColor.c \
	setBackgroundColor.c \
	waitKeyBlinkingCursor.c \
	writeCenteredLine.c \
	writeCharAt.c \
	writeChar.c \
	writeDecWord.c \
	writeString.c \
	writeCharAt_42cols.c \
	font5x8.c \
	writeCharAt_51cols.c \
	writeCharAt_320x16.c \
	font4x8.c \
	setInverseVideoMode.c \
	setBoldMode.c

OS9_GRAPHICS_SRC = \
	showOS9PMode4Screen.c \
	quitOS9Graphics.c
OS9_TIMER_SRC = \
	os9timer.c

ifeq "$(TARGET)" "os9"
# C files that are only relevant when compiling the OS-9 version of this library.

LIBSRC += $(OS9_GRAPHICS_SRC)

# Include OS-9 timer support unless OMIT_OS9_TIMER is not empty.
ifeq "$(OMIT_OS9_TIMER)" ""
LIBSRC += $(OS9_TIMER_SRC)
endif

endif

LIBHEADERS = $(PACKAGE).h $(PACKAGE)_private.h font4x8.h font5x8.h
STATICLIB = lib$(PACKAGE).a

ifeq "$(TARGET)" "coco"
EXEEXT = .bin
endif
ifeq "$(TARGET)" "dragon"
EXEEXT = .bin
endif
ifeq "$(TARGET)" "usim"
EXEEXT = .srec
endif
ifeq "$(TARGET)" "os9"
EXEEXT =
endif

DEMOSRC = $(PACKAGE)-demo.c
DEMOBIN = $(PACKAGE)$(EXEEXT)

DISTFILES = README.md Makefile $(LIBHEADERS) \
		$(LIBSRC) \
		$(OS9_GRAPHICS_SRC) $(OS9_TIMER_SRC) \
		$(DEMOSRC)

TARGET = coco
ORG = #--org=2800
INTERMEDIATE = #--intermediate
NDEBUG = -DNDEBUG
HIRESTEXT_NO_VT52 = #-DHIRESTEXT_NO_VT52
DEFINES = -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\" $(NDEBUG) $(HIRESTEXT_NO_VT52) -DALT_5X8_FONT

ifneq "$(OMIT_OS9_TIMER)" ""
# Omitting OS-9 timer support, so have the compiler define OMIT_OS9_TIMER.
DEFINES += -DOMIT_OS9_TIMER=1
endif

OPTIM = -fomit-frame-pointer
WARNINGS = -Wfor-condition-sizes
WERROR = -Werror
CFLAGS = --$(TARGET) $(WARNINGS) $(WERROR) $(DEFINES) $(INTERMEDIATE) $(OPTIM)  -Werror -I .
LDFLAGS = --$(TARGET) $(ORG) $(INTERMEDIATE)

DEMOOBJ = $(DEMOSRC:.c=.demo.o)
LIBOBJ = $(LIBSRC:.c=.o)

CMOC = cmoc
LWAR = lwar
TAR = tar

all: $(DEMOBIN)

$(DEMOBIN): $(DEMOOBJ) $(STATICLIB)
	$(CMOC) -o $(DEMOBIN) $(LDFLAGS) $(DEMOOBJ) -L. -l$(PACKAGE) $(DEMO_EXTRA_LDFLAGS)

$(STATICLIB): $(LIBOBJ)
	$(LWAR) --create $(STATICLIB) $(LIBOBJ)

%.demo.o: %.c
	$(CMOC) -c $(CFLAGS) $(DEMO_EXTRA_CFLAGS) -o $@ $<

%.o: %.c
	$(CMOC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(DEMOBIN) $(DEMOOBJ) $(STATICLIB) $(LIBOBJ)

.PHONY: dist distcheck
dist:
	$(TAR) -czf $(PACKAGE)-$(VERSION).tar.gz --transform 's,^,$(PACKAGE)-$(VERSION)/,' $(DISTFILES)

distcheck: dist
	mkdir ,distcheck
	$(TAR) -C ,distcheck -xzf $(PACKAGE)-$(VERSION).tar.gz
	$(MAKE) -C ,distcheck/$(PACKAGE)-$(VERSION)
	$(MAKE) -C ,distcheck/$(PACKAGE)-$(VERSION) TARGET=dragon clean all
	$(MAKE) -C ,distcheck/$(PACKAGE)-$(VERSION) TARGET=os9 BCONTROL_LIB_DIR=../../../bcontrol clean all
	rm -fr ,distcheck
	@echo "SUCCESS: $(PACKAGE)-$(VERSION).tar.gz ready for distribution"

# Automatic prerequisite generation.
%.d: %.c
	$(CMOC) --deps-only $(CFLAGS) $(DEMO_EXTRA_CFLAGS) $<

.PHONY: cleandeps
cleandeps:
	rm -f $(DEMOSRC:.c=.d) $(LIBSRC:.c=.d)

include $(DEMOSRC:.c=.d)
include $(LIBSRC:.c=.d)
