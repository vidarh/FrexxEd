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
CFLAGS= -g -Wall -DFREXXED -DUSE_C_SCREEN_SETUP -DDEBUG -DLOG_FILE_EXECUTION
LDFLAGS=
ASMFLAGS = -c

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
CFLAGS = -g -Wall -c -unsigned-char -DAMIGA -Dtest -I ../../FPL/include/ -DNO_PPACKER -DNO_XPK -funsigned-char -D__AMIGADATE__="\"`date`\""
LDFLAGS= -lamiga
endif

# FIXME: Detect gcc or v(bc)c
ifeq ($(UNAME), Amiga)
CC = vc
AS = vc -c
CFLAGS = -c -c99 -unsigned-char -DAMIGA -Dtest
LDFLAGS= -lamiga -lvc -lauto -lreqtools
endif

include Rules.mk
