/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Icon.c
 *
 *********/

#include "compat.h"

#include <exec/types.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <workbench/startup.h>
#include <stdio.h>
#include <string.h>

#include "buf.h"
#include "command.h"
#include "alloc.h"
#include "icon.h"

/* IE ICON GENERATION */

static USHORT Image1Data[] = {
  0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x4000, 
  0x0000, 0x0000, 0x2000, 
  0x0010, 0x0000, 0x1000, 
  0x0020, 0x0000, 0x0800, 
  0x0140, 0x0000, 0x0400, 
  0x0080, 0x0000, 0xFE00, 
  0x0100, 0x0000, 0x0300, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0349, 0x6A40, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0316, 0x2288, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0244, 0x9290, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0294, 0x8000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0312, 0x6A28, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0244, 0x9440, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0332, 0x3510, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0244, 0x9280, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0269, 0x1150, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0120, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0100, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x0000, 0x0000, 0x0000, 

  0xFFFF, 0xFFFE, 0x8000, 
  0xFFFF, 0xFFFE, 0xC000, 
  0xFFFF, 0xFFFE, 0xE000, 
  0xFF9F, 0xFFFE, 0xF000, 
  0xFF3F, 0xFFFE, 0xF800, 
  0xF97F, 0xFFFE, 0xFC00, 
  0xFCFF, 0xFFFE, 0xFE00, 
  0xFFFF, 0xFFFF, 0x8300, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFF49, 0x6A4F, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFF16, 0x2289, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFE44, 0x9297, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFE94, 0x9FFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFF12, 0x6A29, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFE44, 0x944F, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFF32, 0x3513, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFE44, 0x92BF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFE69, 0x1157, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFD27, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0xFFFF, 0xFFFF, 0xFE00, 
  0x4000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
};

static struct Image Image1 = {
  0, 0, 41, 37, 2, &Image1Data[0], 3, 0, NULL
};

static USHORT Image2Data[] = {
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x4000, 
  0x0000, 0x0000, 0x2000, 
  0x0000, 0x0000, 0x1000, 
  0x0008, 0x0000, 0x0800, 
  0x0010, 0x0000, 0x0400, 
  0x00A0, 0x0000, 0x0200, 
  0x0040, 0x0000, 0x7F00, 
  0x0080, 0x0000, 0x0180, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x01A4, 0xB520, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x018B, 0x1144, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0122, 0x4948, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x014A, 0x4000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0189, 0x3514, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0122, 0x4A20, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0199, 0x1A88, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0122, 0x4940, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0134, 0x88A8, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0090, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0080, 
  0x3FFF, 0xFFFF, 0xFF80, 

  0x0000, 0x0000, 0x0000, 
  0x7FFF, 0xFFFF, 0x4000, 
  0x7FFF, 0xFFFF, 0x6000, 
  0x7FFF, 0xFFFF, 0x7000, 
  0x7FCF, 0xFFFF, 0x7800, 
  0x7F9F, 0xFFFF, 0x7C00, 
  0x7CBF, 0xFFFF, 0x7E00, 
  0x7E7F, 0xFFFF, 0x7F00, 
  0x7FFF, 0xFFFF, 0xC180, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FA4, 0xB527, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F8B, 0x1144, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F22, 0x494B, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F4A, 0x4FFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F89, 0x3514, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F22, 0x4A27, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F99, 0x1A89, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F22, 0x495F, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7F34, 0x88AB, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7E93, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x7FFF, 0xFFFF, 0xFF00, 
  0x2000, 0x0000, 0x0000, 
};

static struct Image Image2 = {
  0, 0, 41, 37, 2, &Image2Data[0], 3, 0, NULL
};

static struct DiskObject Icon = {
  WB_DISKMAGIC,
  WB_DISKVERSION,
  {
    NULL,
    104, 109, 41, 38,
    GADGIMAGE | GADGHIMAGE,
    RELVERIFY | GADGIMMEDIATE,
    BOOLGADGET,
    (APTR) &Image1,
    (APTR) &Image2,
    NULL,
    0,
    NULL,
    0,
    NULL
  },
  WBPROJECT,
  NULL,
  NULL,
  NO_ICON_POSITION,
  NO_ICON_POSITION,
  NULL,
  NULL,
  4000
};

/********************
 *
 * MakeIcon()
 *
 * Updates/creates an icon for the specified file.
 *
 * Returns TRUE on failure.
 *
 *****/

int MakeIcon(char *file)
{
  struct DiskObject *dobj;
  char pathbuffer[41];
  char extension[FESIZE];

  dobj = GetDiskObject(file);

  if(dobj) {
    /* There already is an icon, ignore it! */
    FreeDiskObject(dobj); /* free icon */

  } else  {
    /* no previous icon was found */
    if(!stcgfe(extension, file))
      strcpy(extension, FREXXED_NONE);

    strcpy(pathbuffer, FREXXED_ICONDIR);
    strcpy(pathbuffer+ strlen(FREXXED_ICONDIR), extension);

    dobj = GetDiskObject(pathbuffer); /* read icon to use! */

    if(!dobj)
      /* extension icon wasn't found, try the default icon! */
      dobj = GetDiskObject(FREXXED_ICONDIR FREXXED_DEFAULT); /* read default icon */
    
    if(!dobj) {
      /* no custom icon was found, use internal! */
      GetProgramName(pathbuffer, 40);

      Icon.do_DefaultTool = pathbuffer; /* set running FrexxEd name as tool! */

      /* write our icon! */
      PutDiskObject(file, &Icon);
    } else {
      dobj->do_CurrentX = NO_ICON_POSITION;
      dobj->do_CurrentY = NO_ICON_POSITION;

      PutDiskObject(file, dobj); /* write icon */
      FreeDiskObject(dobj); /* free icon information */
    }
  }
  return OK;
}

/*******************************************************************
 *
 * CheckIcon()
 *
 * Returns TRUE if there is an icon for the named file.
 *
 ******************/

char CheckIcon(char *name)
{
  struct DiskObject *dobj = GetDiskObject(name);
  if(dobj)
    FreeDiskObject(dobj);
  return (char)(dobj?TRUE:FALSE);
}


#ifdef TOOLTYPEWRITING

/*********************************************************************
 *
 * GetToolTypes()
 *
 * This function returns an allocated array of char pointers to
 * the tool types from the specified file.
 *
 * The only tool types that are returned are those within the "FrexxEd
 * soft tooltypes" marks (the FREXXED_TOOL defined string).
 *
 * All calls must have a following call to FreeToolTypes().
 *
 *************************/

RetCode GetToolTypes(char *file, struct ToolInfo *tool)
{
  struct DiskObject *dobj;
  char **array=NULL;
  char **temp;
  char found=FALSE;
  int count=0;
  int first;
  int n;
  RetCode ret=OK;

  memset(tool, 0, sizeof( struct ToolInfo ));

  dobj = GetDiskObject(file);
  if(dobj) {
    temp = dobj->do_ToolTypes;
    if(temp) {
      n=0;
      while(temp[n]) {
        if(!strncmp(temp[n], FREXXED_TOOL, strlen(FREXXED_TOOL))) {
          if(!found)
            tool->FirstCustom=n;
          found^=TRUE;
        } else if(found)
          count ++;
        n++;
      }
      tool->TotalNum = n;
      tool->CustomNum = count+2; /* add two for the separation lines! */
      if(count && !found) {
        array = (char **)Malloc(count * sizeof(char *) + 1);
        if(array) {
          first = tool->FirstCustom+1;
          array[count]=NULL;
          n=0;
          while(count--)
            array[n++] = temp[first++];
        } else
          ret = OUT_OF_MEM;
      }
    }
    FreeDiskObject(dobj);
  }

  tool->File = file;
  tool->Tools = array;
  return ret;
}

/*********************************************************************
 *
 * SetToolTypes()
 *
 * Writes the specified array as the new FrexxEd custom tool types to
 * the icon of the specified file!
 *
 *************************/

RetCode SetToolTypes(char *file, char **newtools)
{
  struct ToolInfo tool;
  struct DiskObject *dobj;
  char **temp;
  int line=0;
  int new_lines=0;
  int new_line=0;
  int old_line=0;
  char **array=newtools;
  int num_of_lines;
  RetCode ret=OK;

  GetToolTypes(file, &tool);

  dobj = GetDiskObject(file);
  if(!dobj) {
    MakeIcon(file); /* create a default icon! */
    dobj = GetDiskObject(file);
    if(!dobj)
      return OUT_OF_MEM;
  }

  temp = dobj->do_ToolTypes;

  while(array[new_lines])
    new_lines++;

  if(new_lines) {
    num_of_lines = new_lines + (tool.TotalNum- tool.CustomNum) +2;
  
    array = (char **)Malloc(num_of_lines * sizeof(char *) + 1);
  
    if(array) {
      array[num_of_lines]=NULL;
  
      while(old_line<tool.FirstCustom) {
        array[line]=temp[old_line];
        line++;
        old_line++;
      }
      array[line++] = FREXXED_TOOL;
      while(new_line<new_lines) {
        array[line] = newtools[new_line];
        line++;
        new_line++;
      }
      array[line++] = FREXXED_TOOL;
  
      old_line+=tool.CustomNum; /* pass the custom tool types */
      while(old_line<tool.TotalNum) {
        array[line]=temp[old_line];
        line++;
        old_line++;
      }
      dobj->do_ToolTypes = array;
      PutDiskObject(tool.File, dobj);
      dobj->do_ToolTypes = temp;
      Dealloc(array);
    } else
      ret = OUT_OF_MEM;
  }

  FreeDiskObject(dobj);
  if(tool.Tools)
    FreeToolTypes(&tool);
  return ret;
}

/*********************************************************************
 *
 * FreeToolTypes()
 *
 * Frees the resources associated with the input ToolInfo structure.
 *
 *************************/

RetCode FreeToolTypes(struct ToolInfo *tool)
{
  Dealloc(tool->Tools);
  return OK;
}
#endif
