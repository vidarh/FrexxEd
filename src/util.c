
#include <stdio.h>
#include <string.h>

#include "compat.h"

#ifndef FILESIZE
#define FILESIZE 255
#endif

void __stack_chk_fail() {}

inline int min(int left, int right) { return left <= right ? left : right; }
inline int max(int left, int right) { return left >= right ? left : right; }

/* Get the filename extension. */

// GPL, from http://trac.yam.ch/browser/trunk/yamos/extrasrc/?rev=80
int stcgfe(char *ext, const char *name)
{
   const char *p = name + strlen(name);
   const char *q = p;
   while (p > name && *--p != '.' && *p != '/' && *p != ':');
   if (*p++ == '.' && q - p < FESIZE)
   {
      memcpy(ext, p, q - p + 1);
      return q - p;
   }
   *ext = '\0';
   return 0;
}

#ifndef __AROS__
/* Copy the q to the n chars buffer pointed by p.
   The result is null terminated.
   Returns the number of copied bytes, including '\0'. */
// GPL, from http://trac.yam.ch/browser/trunk/yamos/extrasrc/?rev=80
int stccpy(char *p, const char *q, int n)
{
   char *t = p;
   while ((*p++ = *q++) && --n > 0);
   p[-1] = '\0';
   return p - t;
}
#endif

/* Append a file name to a path. */
// GPL, from http://trac.yam.ch/browser/trunk/yamos/extrasrc/?rev=80
void strmfp(char *name, const char *path, const char *node)
{
  size_t len = path ? strlen(path) : 0;
  if (len)
  {
    memcpy(name, path, len);
    if (name[len-1] != '/' && name[len-1] != ':')
      name[len++] = '/';
  }
  strcpy(name + len, node);
}


void movmem(char *src, char *dest, int num) {
  memmove(dest,src,num);
}



int Stcgfn(char *name, char *filename)
{
  register int len=0;

  if (filename) {
    while (*filename && len<FILESIZE) {/* Vänta till namnet är slut */
      name[len]=*filename;	/* Kopiera namnet */
      len++;			/* Öka längden av resultatet */
      if (*filename==':' || *filename=='/') /* Kolla efter ':' eller '/' */
        len=0;			/* Börja om kopieringen från början */
      filename++;			/* Öka på räknaren av namnet */
    }
  }
  name[len]=0;			/* Nollbyte på slutet */
  return(len);			/* Returnera längden av filnamnet */
}

int Stcgfp(char *path, char *filename)
{
  register int len=0;
  register int lastend=0;

  if (filename) {
    while (*filename && len<FILESIZE) {/* Vänta till namnet är slut */
      path[len]=*filename;	/* Kopiera namnet */
      len++;			/* Öka längden av resultatet */
      if (*filename==':' || *filename=='/') /* Kolla efter ':' eller '/' */
        lastend=len;		/* Spara positionen för slut av möjlig path */
      filename++;			/* Öka på räknaren av namnet */
    }
  }
  path[lastend]=0;		/* Nollbyte på slutet */
  return(lastend);		/* Returnera längden av pathen */
}

/* Returns number of chars to skip (includes the '|' and '&', so to say) */
int GetWildWord(char *from, char *store)
{
  register int len=0;
  
  while (*from) {
    len++;
    if (*from=='|' || *from=='&')
      break;
    *store++=*from++;
  }
  *store=0;
  return(len);
}




