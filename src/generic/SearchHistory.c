
#include "Alloc.h"
#include "Search.h"
/* For MoveUp4: */
#include "Edit.h"

#include <string.h>

extern HistoryStruct SearchHistory;

/*****************************************************
 *
 * HistoryAdd(char *string, int remove)
 *
 * Add and Dealloc a string to the SearchHistory.
 * It will take control of the string pointer, ie make a Strdup() if you want to keep it.
 * if a copy exist, it will be removed.
 *
 ***********/
void HistoryAdd(char *oldstring, int remove)
{
  int counter;

  if (oldstring) {
    int len=strlen(oldstring);
    if (len) {
      if (SearchHistory.maxlength<len)
        SearchHistory.maxlength=len;
      for (counter=0; counter<SearchHistory.strings; counter++) {
        if (strcmp(SearchHistory.text[counter], oldstring)==0) {
          if (remove) {
            Dealloc(SearchHistory.text[counter]);
            memcpy(&SearchHistory.text[counter], &SearchHistory.text[counter+1],
                   sizeof(char *)*(SearchHistory.strings-counter));
            SearchHistory.strings--;
          } else {
            Dealloc(oldstring);
            return;
          }
          break;
        }
      }
      
      if (SearchHistory.strings+4>SearchHistory.maxalloc) {
        register char **temp;
        SearchHistory.maxalloc+=20;
        temp=(char **)Remalloc((char *)SearchHistory.text,SearchHistory.maxalloc*sizeof(char *));
        if (!temp) {
          SearchHistory.maxalloc-=20;
          Dealloc(oldstring);
          return;
        }
        SearchHistory.text=temp;
      }
      MoveUp4(&SearchHistory.text[1], &SearchHistory.text[0], SearchHistory.strings);
      SearchHistory.text[0]=oldstring;
      SearchHistory.strings++;
    }
  } else
    Dealloc(oldstring);
}


void ClearHistory()
{
    /* Deleata SearchHistoryn */
    while(SearchHistory.strings)
        Dealloc(SearchHistory.text[--SearchHistory.strings]);
}

