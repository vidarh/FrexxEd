/* Use this to get a list of each FPL command supported by FrexxEd and their
   according ID. Useful for debugging, since FPL passes the numeric ID's
   internally 

   (C) 2011 Vidar Hokstad. See LEGAL for terms.
*/
#include "Function.h"
#include <stdio.h>

extern struct FrexxEdFunction fred[];
extern const int nofuncs;

int main() {
  int i;

  for (i=0; i < nofuncs; ++i) {
	printf("%d %s\n",fred[i].ID,fred[i].name);
  }

  return 0;
}
