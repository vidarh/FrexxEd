#
# Generic makefile. Requires "uname" to work properly.
#

UNAME := $(shell uname)
ifndef OS
OS    := $UNAME
endif

# Default compiler settings
CC = gcc
AS = gcc -c
CFLAGS= -I. -Igeneric -g -Wall -DFREXXED -DUSE_C_SCREEN_SETUP -DDEBUG -DLOG_FILE_EXECUTION
LDFLAGS=
ASMFLAGS = -c
STRIP= strip

# pass SET_V39_PLUS=1 to make to exclude any code to handle AmigaOS < v39.
# For AROS this is defined by default. Note that the resulting binary WILL NOT
# work on older versions of AmigaOS, as for example the menu layout code
# will depend on gadtools rather than use internal code, but a binary compiled
# *without* SET_V39_PLUS will still work on v39 and above.
ifdef SET_V39_PLUS
CFLAGS += -DV39PLUS
endif

CP=cp
MKDIR=mkdir
BASEDIR=..
WILD=*
CP_R=cp -Rap


ifeq ($(UNAME), Linux)
# FIXME: Handle optional AROS cross-compilation under Linux (env-variable?)
CFLAGS += -c -unsigned-char -DLOG_FILE_EXECUTION -Dtest
ASMFLAGS = -c

	ifeq ($(OS),AROS)
	CC = i386-aros-gcc
	AS = i386-aros-gcc -c
	endif
endif

ifeq ($(OS), AROS)
CFLAGS += -DAMIGA -DV39PLUS -c -unsigned-char -DAMIGA -Dtest -I ../../FPL/include/ -DNO_PPACKER -DNO_XPK -funsigned-char -D__AMIGADATE__="\"`date`\""
LDFLAGS+= -lamiga -larosc -ldos -lexec -lintuition #-lreqtools
STRIP= i386-aros-strip --strip-unneeded --remove-section .comment
endif

# FIXME: Detect gcc or v(bc)c
ifeq ($(UNAME), Amiga)
CC = vc
AS = vc -c
CFLAGS = -c -c99 -unsigned-char -DAMIGA -Dtest
LDFLAGS+= -lamiga -lvc -lauto -lreqtools
endif

include Rules.mk
