/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Main.c
 *
 * Main-routine for the entire shit!
 *
 *
 * Anrop till FrexxEd.
 *
 *  InitFrexxEd(void)		i Editor.c
 *  ParseArg(char *, LONG *)	i Editor.c
 *  secondmain(LONG *, char **) i Editor.c
 *  CloseFrexxEd(char *)        i OpenClose.c
 *
 *********/
#include <dos/rdargs.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <workbench/startup.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <workbench/workbench.h>

#include "compat.h"
#include "Startup.h"

#define MAX_LINE 4000
#define REQUIRE "FrexxEd requires at least Kickstart 2.04\n"

#define FREXXED_ENVVAR "FrexxEd"

#include "frexxed_protos.h"

#ifdef LIB
#include "frexxed_pragmas.h"

long __stack = 8000; /* nice to start with! ;) */

struct Library *FrexxBase = NULL;
#define FREXXED_LIBRARY "frexxed.library"
#define FREXXEDDEBUG_LIBRARY "freddy.library"
#define FREXXED "fred" /* program named like this doesn't run debug library */
#define FREXXED_VERSION 2
#define REQUIRE_FREXXED "FrexxEd requires frexxed.library version 1\n"
#endif

static char *version="$VER: fred 2.0 " __AMIGADATE__;

typedef struct {
  int size;
  char memory[1];
} malloc_struct;

#ifdef LIB
struct Library __regargs *openlib(char *name, int version)
{
  char tempbuffer[80];
  struct Library *ret;

  strcpy(tempbuffer, "FrexxEd:Libs/");
  strcat(tempbuffer, name);
  if( ret = OpenLibrary( tempbuffer, version ) )
    return ret;

  if( ret = OpenLibrary( name, version ) )
    return ret;

  strcpy(tempbuffer, "FrexxEd:");
  strcat(tempbuffer, name);
  ret = OpenLibrary( tempbuffer, version );

  return ret;
}
#endif

/*****************************************
*
* int main (int, char **);
*
* It's here everything starts...!
*
*****/
int main(int argc, char **argv)
{
  LONG opts[opt_COUNT];
  char **FromWb=NULL;
  char *tempbuffer;
  struct WBStartup *wbargmsg;
  struct WBArg *wbarg;
  struct DiskObject *dobj;
  struct RDArgs *argsptr=NULL;
  int buffers;
  char **files;
  char **lines;
  int ret;
  int noof_files=0;
  int i;
  struct Library *IconBase;

  if (SysBase->LibNode.lib_Version > 36) {
#ifdef LIB
#ifndef FINAL_VERSION
    *(short *)0xdff180=0xfff;
    if(argc && strcmp(argv[0], FREXXED))
      FrexxBase = openlib(FREXXEDDEBUG_LIBRARY, FREXXED_VERSION);
    else
#endif
      FrexxBase = openlib(FREXXED_LIBRARY, FREXXED_VERSION);
    if(FrexxBase)
#endif
    {
      tempbuffer=InitFrexxEd();
      if (!tempbuffer) {
        memset(opts, 0, opt_COUNT*sizeof(long));
        tempbuffer=AllocMem(MAX_LINE, MEMF_PUBLIC);
        if (tempbuffer) {
          i = GetVar(FREXXED_ENVVAR, tempbuffer, MAX_LINE-2,
                     GVF_GLOBAL_ONLY|GVF_LOCAL_ONLY);
          if(i>0) {
            tempbuffer[i]='\n'; /* make it end with newline!!! */
            tempbuffer[i+1]=0; /* ...but still zero terminated! */
            ParseArg(tempbuffer, (LONG *)&opts);
          }
          if(!argc) {
            IconBase = OpenLibrary("icon.library", 36);
            if (IconBase) {
            
              FromWb = argv; /* started from workbench */
        
              wbargmsg = (struct WBStartup *)argv;
              buffers = wbargmsg->sm_NumArgs; /* number of arguments */
        
              files = (char **)AllocMem(buffers * sizeof(char *), MEMF_PUBLIC);
              if(files) {
                wbarg = wbargmsg->sm_ArgList;
                for(i=0;
                    i<buffers;
                    i++, wbarg++) {
                  if(wbarg->wa_Lock) {
                    if(!NameFromLock(wbarg->wa_Lock, tempbuffer,
                                     MAX_LINE-strlen(wbarg->wa_Name)))
                      continue;
          
                    ret = strlen(tempbuffer);
                    if(tempbuffer[ret-1] != '/' && tempbuffer[ret-1] != ':')
                      strcat(tempbuffer, "/"); /* append slash! */
          
                    strcat(tempbuffer, wbarg->wa_Name); /* add filename to path */
        
                    if(i) {
                      register malloc_struct *mem;
                      /* do not include 'FrexxEd' ! */
                      ret = strlen(tempbuffer) +1;
                      mem=(malloc_struct *)AllocMem(ret+sizeof(malloc_struct), MEMF_PUBLIC);
                      if(mem) {
                        mem->size=ret+sizeof(malloc_struct);
                        files[noof_files]=(char *)&mem->memory;
                        strcpy(files[noof_files++], tempbuffer);
                      } else
                        break; /* getting low on memory! */
                    }
        
                    dobj = GetDiskObject(tempbuffer);
                    if(dobj) {
                      lines=dobj->do_ToolTypes;
                      while(lines && *lines) {
                        strcpy(tempbuffer, *lines);
                        strcat(tempbuffer, "\n");
                        ParseArg(tempbuffer, (LONG *)&opts);
                        lines++;
                      }
                      FreeDiskObject(dobj);
                    } /* else no icon found */
                  }
                }
                files[noof_files]=NULL;
                opts[opt_FILE]=(long)files;
              }
            }
            CloseLibrary(IconBase);
          } else {
            argsptr = ReadArgs(TEMPLATE, opts, NULL);
            if(!argsptr) {
              PrintFault(IoErr(), NULL); /* prints the appropriate err message */
              return(NULL);
            }
      
            if (opts[opt_ASK])
              FPrintf(Output(), "%s\n", TEMPLATE+4);
            else
              ParseArg(NULL, (LONG *)&opts);
          }
          FreeMem(tempbuffer, MAX_LINE);
        }
        if (!opts[opt_ASK]) {
          secondmain((long int *)&opts, FromWb);
          if(FromWb) {
            for(i=0; i<noof_files; i++) {
              if(files[i]) {
                register malloc_struct *mem;
                mem=(malloc_struct *)(files[i]-4);
                FreeMem(mem, mem->size);
              }
            }
            FreeMem(files, buffers * sizeof(char *));
          }
        } else
          CloseFrexxEd(NULL);	// Exit due to "?"/Help argument.
        if (argsptr)
          FreeArgs(argsptr);  // Strängarna från ReadArgs kopieras i SecondInit()
      } else
        CloseFrexxEd(tempbuffer);	// Exit due to init error.
#ifdef LIB
      CloseLibrary(FrexxBase); /* Close frexxed.library */
#endif
    }
#ifdef LIB
    else
      Write(Output(), REQUIRE_FREXXED, sizeof(REQUIRE_FREXXED));
#endif
  } else
    Write(Output(), REQUIRE, sizeof(REQUIRE));
  return 0;
}
