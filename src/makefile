#
# VBCC Makefile for classic AmigaOS
#
# To start with we'll get it working without building the libraries

TARGET = Fred
FREDLIB = # frexxed.library

CC = vc
AS = vc -c
CFLAGS = -c -c99 -unsigned-char -DFREXXED -DUSE_C_SCREEN_SETUP -DDEBUG -DLOG_FILE_EXECUTION -DAMIGA -Dtest
LDFLAGS= -lamiga -lvc -lauto -lreqtools
ASMFLAGS = -c

OBJS = \
 Alloc.o Block.o BufControl.o BuildMenu.o Button.o Change.o ClipBoard.o\
 Command.o Cursor.o Declare.o DoSearch.o Edit.o\
 Execute.o Face.o FACT.o FH_packets.o FileHandler.o Fold.o GetFile.o\
 GetFont.o Hook.o Icon.o IDCMP.o Init.o KeyAssign.o\
 Match.o Mount.o MultML.o OpenClose.o Process.o Prompt.o Regex.o\
 Replace.o Reqlist.o Request.o Rexx.o RSADecrypt.o Search.o Setting.o Sort.o\
 Slider.o Startup.o Strings.o Timer.o Undo.o UpdtBlock.o UpdtScreen.o\
 UpdtScreenC.o WinSign.o WindowOutput.o util.o WBPath.o

NONLIBOBJS = Main.o FrexxEd_rev.o

all:	$(TARGET) $(FREDLIB)

FrexxEd_rev.o: FrexxEd_rev.c Makefile
#	AutoRev FrexxEd_rev.c Verbose
	$(CC) $(CFLAGS) -o FrexxEd_rev.o FrexxEd_rev.c

$(TARGET): $(OBJS) $(NONLIBOBJS)
	$(CC) -o $(TARGET) $(LDFLAGS) $+


Alloc.o:		Alloc.c Alloc.h Buf.h Function.h
Block.o:		Block.c Buf.h Function.h
BufControl.o:		BufControl.c Buf.h Function.h
BuildMenu.o:		Buildmenu.c Buildmenu.h Buf.h Function.h
Button.o:		Button.c Buf.h Function.h
Change.o:		Change.c Buf.h Function.h
ClipBoard.o:		ClipBoard.c Buf.h Function.h
Command.o:		Command.c Command.h Function.h Buf.h 
Cursor.o:		Cursor.c Buf.h Function.h
Declare.o:		Declare.c Buf.h Function.h
DoSearch.o:		DoSearch.c Buf.h Function.h
Edit.o:		Edit.c Buf.h Function.h UpdtScreenC.h Edit.h
Execute.o:		Execute.c Buf.h Function.h
Face.o:		Face.c Face.h Buf.h Function.h
FACT.o:		FACT.c FACT.h Buf.h Function.h
FileHandler.o:	FileHandler.c FileHandler.h Buf.h fh_packets.h
FH_packets.o:		FH_packets.c FH_packets.h
Fold.o:		Fold.c Buf.h Function.h
GetFile.o:		GetFile.c Buf.h Function.h
GetFont.o:		GetFont.c Buf.h Function.h
Hook.o:		Hook.c Buf.h Function.h
IDCMP.o:		IDCMP.c Function.h Buf.h Timer.h
Icon.o:		Icon.c Buf.h Function.h
Init.o:		Init.c Buf.h Function.h
KeyAssign.o:		KeyAssign.c Buf.h Function.h
Match.o:		Match.c Match.h Buf.h Function.h
Mount.o:		Mount.c Mount.h
NewTask.o:		NewTask.c NewTask.h Buf.h Function.h
OpenClose.o:		OpenClose.c Buf.h Function.h Timer.h
Process.o:		Process.c Process.h
Prompt.o:		Prompt.c Prompt.h Buf.h Function.h
Regex.o:		Regex.c Buf.h Function.h
Replace.o:		Replace.c Buf.h Function.h
Reqlist.o:		Reqlist.c Reqlist.h Buf.h
Request.o:		Request.c Buf.h Function.h
Rexx.o:		Rexx.c Rexx.h Buf.h Function.h
Search.o:		Search.c Buf.h Function.h
Setting.o:		Setting.c Buf.h Function.h
Slider.o:		Slider.c Buf.h Function.h
Startup.o:		Startup.c Buf.h Function.h
Sort.o:		Sort.c Buf.h Function.h
Strings.o:		Strings.c Strings.h Buf.h Function.h
Timer.o:		Timer.c Timer.h Buf.h Function.h
Undo.o:		Undo.c Buf.h Function.h
UpdtBlock.o:		UpdtBlock.c Buf.h Function.h
UpdtScreen.o:		UpdtScreen.c Buf.h Function.h
UpdtScreenC.o:	UpdtScreenC.c Buf.h Function.h UpdtScreenC.h
WinSign.o:		WinSign.c Buf.h Function.h
WindowOutput.o:	WindowOutput.c Buf.h Function.h UpdtScreenC.h

Main.o:		Main.c
	$(CC) $(CFLAGS) -DFREXXED -o Main.o $<

MultML.o:		MultML.s
	$(CC) $(ASMFLAGS) -o MultML.o $<
NewTaskML.o:		NewTaskML.s
	$(CC) $(ASMFLAGS) -o NewTaskML.o $<
