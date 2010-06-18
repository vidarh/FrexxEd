#include "system.h"

#include "/GadLib/FrexxLayout.h"
#include "/GadLib/FrexxLayoutProtos.h"

#define DEFAULT_SETUP "FrexxTool.setup"
#define MAXGADS 9		/* Max number of buttons */
#define GADWIDTH 12		/* Number of characters in a button */
#define MAX_LINE_LEN 256	/* Max number of chars on an input line */
struct LayoutGadgets Gads[MAXGADS + 1];
char *Commands[MAXGADS];	/* Holds the ARexx command strings */

/**************************************************************
** This one reads the setup file and creates the FrexxLayout
** structure for the buttons. It also fills in the Commands[]
** array with all the ARexx commands to be sent.
********************************/

int GetSetup(char *filename, int columns)
{
   char array[MAX_LINE_LEN];	/* File input buffer */
   struct TagItem *tags;	/* Pointer to the current tag array */
   int tag;			/* Index in the tag array */
   char *col;			/* Points to the first colon on the line */
   char *name;			/* Holds the allocated button name string */
   long num = 0;		/* Gadget counter */
   int ret = FALSE;		/* Return value, TRUE if error */
   FILE *file;			/* Input file handle */

   if (!filename || !filename[0])
   {
      filename = DEFAULT_SETUP;
   }

   file = fopen(filename, "r");
   if (file)
   {
      while (fgets(array, MAX_LINE_LEN, file))
      {
	 col = strchr(array, ':');
	 if (!col)		/* illegal line */
	 {
	    continue;
	 }
	 name = malloc(col - array + 1);
	 if (!name)
	 {
	    ret = TRUE;		/* fail everything malloc()s get cleaned up! */
	    break;
	 }
	 strncpy(name, array, col - array);
	 tags = malloc(sizeof(struct TagItem) * 7);	/* 7 tags */
	 tag = 0;
	 tags[tag].ti_Tag = FL_GadgetKind;
	 tags[tag++].ti_Data = BUTTON_KIND;
	 tags[tag].ti_Tag = FL_GadgetText;
	 tags[tag++].ti_Data = (ULONG)name;
	 if(num % columns)
	 {
	    tags[tag].ti_Tag = FL_RelLeftEdge;
	    tags[tag++].ti_Data = num - 1;
	 }
	 else
	 {
	    tags[tag].ti_Tag = FL_LeftEdge;
	    tags[tag++].ti_Data = 8;
	 }
	 if(num / columns)
	 {
	    tags[tag].ti_Tag = FL_RelTopEdge;
	    tags[tag++].ti_Data = num - columns;
	 }
	 else
	 {
	    tags[tag].ti_Tag = FL_TopEdge;
	    tags[tag++].ti_Data = 2;
	 }
	 tags[tag].ti_Tag = FL_Columns;
	 tags[tag++].ti_Data = GADWIDTH;
	 tags[tag].ti_Tag = FL_Rows;
	 tags[tag++].ti_Data = 1;
	 tags[tag].ti_Tag = TAG_END;

	 Gads[num].GadgetID = num;
	 Gads[num].LayoutTags = tags;
	 Gads[num].GadToolsTags = NULL;
	 Gads[num].Gadget = NULL;
	 name[col - array] = '\0';
	 Commands[num] = strdup(++col);
	 if (!Commands[num])
	 {
	    ret = TRUE;		/* fail everything malloc()s get cleaned up! */
	    break;
	 }

	 if (++num == MAXGADS)
	    break;
      }
      fclose(file);
      Gads[num].GadgetID = -1;
      Gads[num].LayoutTags = NULL;
      Gads[num].GadToolsTags = NULL;
      Gads[num].Gadget = NULL;
   }
   return ret;
}
