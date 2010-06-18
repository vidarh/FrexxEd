
#include <proto/exec.h>
#include <proto/iffparse.h>
#include <libraries/iffparse.h>

#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')

/*************************************************************
*
*  int String2Clip(BufStruct *Storage, char *sendstring, int sendlen, int clipno)
*
*  Copy the string to given ClipBoard.
*  Return TRUE/FALSE.
************/
int __asm String2Clip(register __a0 char *string, register __d0 int stringlen, register __d1 int clipno)
{
  struct Library *IFFParseBase ;
  struct IFFHandle *iff=NULL;
  long error=0;
  int ret;

  ret=FALSE;	// error
  if (IFFParseBase=OpenLibrary("iffparse.library", 0L)) {
    if (iff=AllocIFF()) {
      if (iff->iff_Stream=(ULONG)OpenClipboard(clipno)) {
        InitIFFasClip(iff);
        if (!(error=OpenIFF(iff, IFFF_WRITE))) {
          if (!(error=PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))) {
            if (!(error=PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN))) {
              if (WriteChunkBytes(iff, string, stringlen)==stringlen) {
                ret=TRUE;	// OK
              } else
                error=IFFERR_WRITE;

              if (!error)
                error=PopChunk(iff);
            }
            if (!error)
              error=PopChunk(iff);
          }
          CloseIFF(iff);
        }
        CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
    }
    CloseLibrary(IFFParseBase);
  }
  return(ret);
}          

