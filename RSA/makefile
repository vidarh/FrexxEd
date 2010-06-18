
TARGET = KeyFile

ASMFLAGS = -m0 -oo/ -d2 -iINCLUDE:
#LIBS = LIB:sc.lib LIB:amiga.lib
LIBS
OBJS = o/JumpTable.o o/Crypt.o


$(TARGET): $(OBJS) Makefile
	slink <with <
	STRIPDEBUG
	SMALLCODE
	SMALLDATA
	FWIDTH 20
	PWIDTH 20
	WIDTH 20
	map $(TARGET).map H
	FROM $(OBJS)
	TO $(TARGET)
	LIB $(LIBS)
	<
   
o/Crypt.o:	Crypt.c

o/JumpTable.o:	JumpTable.asm
    asm $(ASMFLAGS) $<

