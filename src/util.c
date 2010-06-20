
#include <stdio.h>
#include <string.h>

#include "compat.h"

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
