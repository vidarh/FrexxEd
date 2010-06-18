CC		=	sc
CFLAGS		=
LD		=	slink
LDFLAGS		=	SMALLCODE

GSTHEADER	=	system
TARGET		=	FrexxTool
LIBS		=	LIB:sc.lib LIB:amiga.lib /GadLib/FrexxLayout.lib
STARTUP		=	LIB:c.o

#
#	Include files
#
INCLUDES	=	/GadLib/FrexxLayout.h \
			/GadLib/FrexxLayoutProtos.h

#
#	Object files
#
OBJS		=	main.o \
			rexx.o \
			setup.o

#
#	Default rules
#
.c.o:
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) GST=$(GSTHEADER).gst $<

.s.o:
	@echo "Assembling $<"
	@$(ASM) $(ASMFLAGS) $<

#
#	Main target
#
$(TARGET): $(OBJS) $(LIBS)
	@$(LD) $(STARTUP) $(OBJS) TO $(TARGET) LIBRARY $(LIBS) $(LDFLAGS)

#
#	Object file dependencies
#
main.o: main.c $(INCLUDES) SCOPTIONS
rexx.o: rexx.c $(INCLUDES) SCOPTIONS
setup.o: setup.c $(INCLUDES) SCOPTIONS

#
#	GST file dependency
#
$(GSTHEADER).gst: $(GSTHEADER).h
	@echo "Creating $(GSTHEADER).gst"
	@$(CC) MAKEGST $(GSTHEADER).gst $<
	@rm >NIL: $(GSTHEADER).o

#
#	GST dummy target
#
gst:	$(GSTHEADER).gst

