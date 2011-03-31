int __regargs SearchAsk(BufStruct *Storage, int replace)
{
  int width;
  int ret=OK;
  BOOL copy=FALSE;
  ButtonStruct Buttons;
  ButtonArrayStruct butarray[ANTALSEARCH];

  memset(butarray, 0, sizeof(butarray));

  butarray[posWILD].name=RetString(STR_WILDCARD);
  butarray[posCASE].name=RetString(STR_CASE_SENSITIVE);
  butarray[posWORD].name=RetString(STR_ONLY_WORDS);
  butarray[posFORW].name=RetString(STR_FORWARD_SEARCH);
  butarray[posBLOK].name=RetString(STR_INSIDE_BLOCK);
  butarray[posPRRE].name=RetString(STR_PROMPT_REPLACE);
  butarray[posLIWI].name=RetString(STR_LIMIT_WILDCARD);
  butarray[posINFO].name=RetString(STR_INSIDE_FOLD);


  if (Search.flags & WILDCARD)
    butarray[posWILD].flags=1;

  if (Search.flags & CASE_SENSITIVE)
    butarray[posCASE].flags=1;

  if (Search.flags & ONLY_WORDS)
    butarray[posWORD].flags=1;

  if (Search.flags & FORWARD)
    butarray[posFORW].flags=1;

  if (Search.flags & INSIDE_BLOCK)
    butarray[posBLOK].flags=1;

  if (Search.flags & PROMPT_REPLACE)
    butarray[posPRRE].flags=1;

  if (Search.flags & LIMIT_WILDCARD)
    butarray[posLIWI].flags=1;

  if (Search.flags & INSIDE_FOLD)
    butarray[posINFO].flags=1;

  {
    char *string;
    char *string2=NULL;
    char *sstring;
    int maxlength=Search.buflen;
    int retval;
    {
      if (maxlength<SearchHistory.maxlength)
          maxlength=SearchHistory.maxlength;
      if (maxlength<Search.repbuflen)
        maxlength=Search.repbuflen;
    }
    width=maxlength;
    maxlength+=EXTRA_ALLOC;

    string=Malloc(replace?maxlength<<1:maxlength);
    if (string) {
      string[0]=0;
      if (Search.repbuffer) {
        sstring=Strdup(Search.realrepbuffer);
        HistoryAdd(sstring, replace);
      }
      if (Search.buffer) {
        sstring=Strdup(Search.realbuffer);
        HistoryAdd(sstring, TRUE);
      }
      if (replace) {
        string2=&string[maxlength];
        string2[0]=0;
      }
      InitButtonStruct(&Buttons);
      Buttons.array=butarray;
      Buttons.antal=ANTALSEARCH;
      Buttons.toptext=RetString(STR_SEARCH_FLAGS);
      Buttons.maxheight=RequestFont->tf_YSize<<3;
      if (replace)
        retval=Reqlist(Storage,
                       REQTAG_ARRAY, SearchHistory.text, SearchHistory.strings,
                       REQTAG_WIDTH, width,
                       REQTAG_STRING1, string, maxlength,
                       REQTAG_STRING2, string2, maxlength,
                       REQTAG_BUTTON, &Buttons,
                       REQTAG_TITLE, RetString(STR_SEARCH_REPLACE), REQTAG_END);
      else
        retval=Reqlist(Storage,
                       REQTAG_ARRAY, SearchHistory.text, SearchHistory.strings,
                       REQTAG_WIDTH, width,
                       REQTAG_STRING1, string, maxlength,
                       REQTAG_BUTTON, &Buttons,
                       REQTAG_TITLE, RetString(STR_SEARCH_FOR), REQTAG_END);
      if (retval<0 || replace || !string[0]) {
        if (retval==rl_CANCEL || !string[0])
          ret=FUNCTION_CANCEL;
        else if(retval==rl_ERROR)
          ret=OUT_OF_MEM;
        else {
          if (replace) {
            Dealloc(Search.repbuffer);
            Dealloc(Search.realrepbuffer);
            Search.realrepbuffer=Strdup(string2);
            Search.repbuflen=fplConvertString(Anchor, string2, string2);
            Search.repbuffer=Malloc(Search.repbuflen);
            if (Search.repbuffer) {
              sstring=Strdup(Search.realrepbuffer);
              HistoryAdd(sstring, TRUE);
              memcpy(Search.repbuffer, string2, Search.repbuflen);
            } else
              ret=OUT_OF_MEM;
          }
          Dealloc(Search.realbuffer);
          Search.realbuffer=Strdup(string);
          Search.lastpos=-1;
          copy=TRUE;
        }
      } else {
        copy=TRUE;
        Dealloc(Search.realbuffer);
        Search.realbuffer=Strdup(SearchHistory.text[retval]);
      }
      if (copy) {
        Dealloc(Search.buffer);
        Dealloc(Search.buf.buffer);
        Search.buf.buffer=NULL;
        Search.buf.allocated=0;
        Search.pattern_compiled=FALSE;
        searchcompiled=FALSE;

        Search.buflen=fplConvertString(Anchor, Search.realbuffer, string);
        Search.buffer=Malloc(Search.buflen);
        if (Search.buffer) {
          memcpy(Search.buffer, string, Search.buflen);
          HistoryAdd(Strdup(Search.realbuffer), TRUE);
        } else {
          ret=OUT_OF_MEM;
          Search.buflen=0;
        }
      }
      Dealloc(string);
    } else
      ret=OUT_OF_MEM;
  }
  
  if (ret == OK) {
    Search.flags=0;

    if (butarray[posWILD].flags)
      Search.flags|=WILDCARD;

    if (butarray[posCASE].flags)
      Search.flags|=CASE_SENSITIVE;

    if (butarray[posWORD].flags)
      Search.flags|=ONLY_WORDS;

    if (butarray[posFORW].flags)
      Search.flags|=FORWARD;

    if (butarray[posBLOK].flags)
      Search.flags|=INSIDE_BLOCK;

    if (butarray[posPRRE].flags)
      Search.flags|=PROMPT_REPLACE;

    if (butarray[posLIWI].flags)
      Search.flags|=LIMIT_WILDCARD;

    if (butarray[posINFO].flags)
      Search.flags|=INSIDE_FOLD;

    Default.search_flags=Search.flags;
    ConvertSearchFlags();
  }
  return(ret);
}

