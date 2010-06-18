/*************************************************************************
 *
 * Barfly2Msg.c
 *
 * by Bjorn Reese (breese@imada.ou.dk)
 *
 * Compiled with:
 *   sc LINK OPT SMALLCODE SMALLDATA PARM=R STRIPDBG AUTOREG Barfly2Msg
 *
 * From the document of TDS:
 *
 * `Converter'
 *      Use these gadgets to select the converter program for the
 *      compiler/assembler. The purpose of the converter is to translate
 *      the error messages coming from the compiler/assembler into a format
 *      readable by the editor. The list gadget displays all converters
 *      which are available in the directory `TDS:converters'. If you want
 *      to use a compiler which doesn't have a converter, you can easily
 *      write your own one. All the converter has to do is to read from
 *      standard input the output of the compiler and to write to standard
 *      output the error messages with the following format:
 *
 *           MyConverter <compiler_error_file >ted_error_file source_file
 *
 *           `<test.c> 10 E <Error Message>' for an error
 *           `<test.c> 21 W <Error Message>' for a warning
 *
 *           where
 *           <test.c>        : source file
 *           10              : line
 *           E or W          : error or warning
 *           <Error Message> : error message
 *
 *      The converter program also receives the name of the source file as
 *      the first argument on the command line which sometimes is very
 *      helpful if the compiler didn't write the name of the source file
 *      when outputing the error messages.
 *
 ************************************************************************/

/*************************************************************************

Error in Line 6 in a:a/err.asm.
Error 8: MC680xx doesn't allow this command, adressmode or width.
        mulu.l  d0,d0
                     ^

Warning in Line 7 in test.asm.
Warning 541: You use a supervisor command.
	reset
	     ^

*** Unsupported

Error, Line 0 in Macro `FUNCDEF	SetRPAttrsA'.
Macro is located at Line 179 in `INCLUDE	offset/graphics_lib.i	;  forget. You can make them'
Error 2: ATTENTION » NOT ENOUGH MEMORY «
	FUNCDEF	SetRPAttrsA


*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256

char *GetSource(char *text) {

   int i, j;

   /* skip initial text and search for "in" */
   for(i=14; text[i] != 'i'; i++);
   i +=3;
   for(j = i; text[j] != '\n'; j++);
   text[--j] = '\0';

   return &text[i];
}


void GetErrorMsg(char *text) {

   int i;

   fgets(text, MAXLINE, stdin);
   for(i=0; text[i] != '\n'; i++);
   text[i] = '\0';
}


void main(int argc, char *argv[]) {

   char text[MAXLINE], errmsg[MAXLINE], *source;
   int line;

   while (fgets(text, MAXLINE, stdin) != NULL) {

      if (strncmp(text, "Error in", 8) == 0) {

         source = GetSource(text);
         line = atoi(&text[14]);
         GetErrorMsg(errmsg);

         fprintf(stdout, "<%s> %d E <%s>\n", source, line, errmsg);
      } else {
         if (strncmp(text, "Warning in", 10) == 0) {

            source = GetSource(text);
            line = atoi(&text[16]);
            GetErrorMsg(errmsg);

            fprintf(stdout, "<%s> %d W <%s>\n", source, line, errmsg);
         } else {
            if (strncmp(text, "Error, Line", 11) == 0)
               fprintf(stdout, "<> 0 E <Unsupported error. Check Error File>");
         }
      }
   }

} /* main */
