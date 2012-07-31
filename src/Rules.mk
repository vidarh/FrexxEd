# This contains the generic (non-platform specific) rules for the FrexxEd makefile

TARGET = Fred
FREDLIB = # frexxed.library

# Largely OS independent files
GENERIC = generic/Alloc.o generic/Block.o generic/BufControl.o generic/Cursor.o generic/Face.o \
	generic/Match.o generic/Search.o generic/Fold.o  generic/Sort.o generic/Regex.o generic/Execute.o\
	generic/DoSearch.o generic/MultML.o generic/UpdtScreenC.o generic/UpdtScreen.o generic/Edit.o \
	generic/Replace.o generic/Undo.o generic/Setting.o  generic/FACT.o generic/Prompt.o generic/Change.o \
	generic/Command.o generic/GetFile.o generic/Declare.o generic/SearchHistory.o generic/UpdtBlock.o generic/Actions.o

OBJS = \
 $(GENERIC) BuildMenu.o Button.o Change.o ClipBoard.o\
 CommandA.o \
 ExecuteA.o FH_packets.o FileHandler.o \
 GetFont.o Hook.o Icon.o IDCMP.o Init.o KeyAssign.o\
 Mount.o OpenClose.o Process.o \
 Reqlist.o Request.o Rexx.o \
 Slider.o Startup.o Strings.o Timer.o \
 Winsign.o WindowOutput.o SearchUI.o util.o WBPath.o AppIcon.o \
 Palette.o GetFileA.o DeclareA.o compat.o


NONLIBOBJS = Main.o FrexxEd_rev.o

all:	$(TARGET) $(FREDLIB)

FrexxEd_rev.o: FrexxEd_rev.c makefile Rules.mk
#	AutoRev FrexxEd_rev.c Verbose
	$(CC) $(CFLAGS) -o FrexxEd_rev.o FrexxEd_rev.c

$(TARGET): $(OBJS) $(NONLIBOBJS)
	$(CC) -o $(TARGET) $+ libfpl.a $(LDFLAGS)

dumpcommands: dumpcommands.o Declare.o

.PHONY: clean
clean:
	rm -f *.o *~ generic/*.o

.PHONY: strip
strip:
	$(STRIP) $(TARGET)

# Build a binary distribution
bindist: Fred
	$(MKDIR) FrexxEd
	$(MKDIR) FrexxEd/Docs
	$(CP) Fred FrexxEd
	$(CP_R) $(BASEDIR)/docs/$(WILD) FrexxEd/Docs
	$(MKDIR) FrexxEd/FPL
	$(CP_R) $(BASEDIR)/FPL/$(WILD) FrexxEd/FPL
	$(MKDIR) FrexxEd/icons
	$(CP_R) $(BASEDIR)/icons/* FrexxEd/icons
	lha a FrexxEd.lha FrexxEd


Alloc.o:		Alloc.c Alloc.h Buf.h Function.h
AppIcon.o:	    AppIcon.c
Block.o:		Block.c Buf.h Function.h
BufControl.o:		BufControl.c Buf.h Function.h
BuildMenu.o:		BuildMenu.c BuildMenu.h Buf.h Function.h
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
FileHandler.o:	FileHandler.c FileHandler.h Buf.h FH_packets.h
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
WinSign.o:		Winsign.c Buf.h Function.h
WindowOutput.o:	WindowOutput.c Buf.h Function.h generic/UpdtScreenC.h


Main.o:		Main.c
	$(CC) $(CFLAGS) -DFREXXED -o Main.o $<

MultML.o: MultML.c
NewTaskML.o: NewTaskML.c
