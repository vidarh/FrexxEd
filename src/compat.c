
#include "compat.h"

#ifndef AMIGA
#include <strings.h>

void * AllocMem(long size, int flags) {
    return 0;
}

void FreeMem(void * addr, long size) {
}

int stricmp(const char * src, const char * dest) {
    return strcasecmp(src,dest);
}

int Stricmp(const char * src, const char * dest) {
    return strcasecmp(src,dest);
}

int Strnicmp(const char * src, const char * dest, int num) {
    return strncasecmp(src,dest, num);
}

void * Open(const char * filename, int flags) {
    return 0;
}

void Delay(long num) { }

void * CreateGadget() { }

void FPuts() { }

int AvailMem() { }

void Close() { }
void ToUpper() { }
void DisplayBeep() {}
void RefreshGList() {}
void CloseWindow() { }
void SetWindowTitles() { }
long Write() { }
void DeleteFile() { }
int IoErr() { }
void SetDrMd() { }
void Forbid() { }
void Permit() { }
void SetWrMsk() {}

void AllocIFF() { }
void Fault() { }
void NameFromLock() { }
void PathPart() { }
void BADDR() { }
void MatchFirst() { }
void LayoutMenus() { }
void CreateMenus() { }
void TextLength() { }
void SetFont() { }
void FreeMenus() { }

void MallocCycle() { }
void FPrintf() { }
void FreeCycle() { }
void * OpenLibrary(char * name,long ver) { }

void Eat() { }
void Malloc() {}
void * GetMsg(void * port) {}
void GetEnd() { }
void ModifyIDCMP() { }
void Expression() { }
void CloseLibrary() { }
void SwapMem() { }
void ReplyMsg() { }
void SetAPen() { }
void SetBPen() { }
void GetIdentifier() { }
void UnlockPubScreen() { }
void DeleteMsgPort() { }
void GetDiskObject() { }
void RefreshWindowFrame() { }
struct Task * FindTask(void * name) { }
void GetRGB4() {}
void FreeKind() { }
int Wait(int foo) { }
void SetProtection() {}
void LockPubScreen() {}
void RemoveGadget() {}

void AbortIO() {}
void ActivateGadget() {}
void ActivateWindow() {}
void AddAppIcon() {}
void AddAppWindow() {}
void AddDosEntry() {}
void AddGadget() {}
void AddLevel() {}
void AddVar() {}
void ARG0() {}
void ArrayNum() {}
void ArrayResize() {}
void AssignArg() {}
void AttemptLockDosList() {}
void BitToggle() {}
void CacheClearU() {}
void ChangeWindowBox() {}
void CheckIO() {}
void CheckRexxMsg() {}
void ClearMenuStrip() {}
void CloseCatalog() {}
void CloseClipboard() {}
void CloseDevice() {}
void CloseFont() {}
void CloseIFF() {}
void CloseScreen() {}
void CmpBreak() {}
void CmpDeclare() {}
void CmpExport() {}
void CmpExpr() {}
void CmpReset() {}
void CmpSwitch() {}
void CompareDates() {}
void CreateArgstring() {}
void CreateContext() {}
void CreateExtIO() {}
void CreateIORequest() {}
void CreateMsgPort() {}
void CreateNewProcTagList() {}
void CreatePort() {}
void CreateRexxMsg() {}
void CurrentChunk() {}
void CurrentDir() {}
void DateStamp() {}
void DateToStr() {}
void Dealloc() {}
void DefaultAlloc() {}
void DefaultDealloc() {}
void DeleteArgstring() {}
void DeleteExtIO() {}
void DeleteIORequest() {}
void DeleteMessage() {}
void DeletePort() {}
void DeleteRexxMsg() {}
void DelIdentifier() {}
void DelLocalVar() {}
void DelProgram() {}
void DiskfontBase() {}
void DOSBase() {}
void DoubleClick() {}
void DrawBorder() {}
void EasyRequestArgs() {}
void Examine() {}
void fileReqFileListNotEmpty() {}
void FindDosEntry() {}
void FindName() {}
void FindPort() {}
void FindProp() {}
void FlushFree() {}
void fpl_functions() {}
void FreeAll() {}
void FreeArgs() {}
void FreeDiskObject() {}
void FreeDosEntry() {}
void FreeGadgets() {}
void FreeIFF() {}
void FreeVec() {}
void FreeVisualInfo() {}
void functions() {}
void GadToolsBase() {}
void GetCatalogStr() {}
void GetDisplayInfoData() {}
void GetErrorMsg() {}
void GetLong() {}
void GetMessage() {}
void GetPrefs() {}
void GetProgram() {}
void GetProgramName() {}
void getreg() {}
void GetRexxVar() {}
void GetRGB32() {}
void GetScreenDrawInfo() {}
void GetShort() {}
void GetVar() {}
void GetVisualInfo() {}
void GetVPModeID() {}
void Getword() {}
void GfxBase() {}
void GT_GetGadgetAttrs() {}
void GT_GetIMsg() {}
void GTMENUITEM_USERDATA() {}
void GT_RefreshWindow() {}
void GT_ReplyIMsg() {}
void GT_SetGadgetAttrs() {}
void IconBase() {}
void InitFree() {}
void InitIFFasClip() {}
void InitIFFasDOS() {}
void InitRastPort() {}
void InitSemaphore() {}
void InterfaceCall() {}
void IntuitionBase() {}
void ItemAddress() {}
void ITEMNUM() {}
void LeaveProgram() {}
void LoadSeg() {}
void LocaleBase() {}
void LockPubScreenList() {}
void MakeDosEntry() {}
void MAKE_ID() {}
void MatchEnd() {}
void MatchNext() {}
void MENUNUM() {}
void MKBADDR() {}
void Move() {}
void my_memicmp() {}
void Newline() {}
void NewModifyProp() {}
void ObtainSemaphore() {}
void OffGadget() {}
void OnGadget() {}
void OpenCatalogA() {}
void OpenClipboard() {}
void OpenDevice() {}
void OpenDiskFont() {}
void OpenFont() {}
void OpenIFF() {}
void OpenScreenTags() {}
void OpenWindowTags() {}
void Output() {}
void ParentDir() {}
void ParseIFF() {}
void PopChunk() {}
void PrintFault() {}
void PropChunk() {}
void PubScreenStatus() {}
void PushChunk() {}
void PutDiskObject() {}
void PutMsg() {}
void putreg() {}
void RawDoFmt() {}
void RawKeyConvert() {}
void Read() {}
void ReadArgs() {}
void ReadChunkBytes() {}
void ReleaseSemaphore() {}
void RemDosEntry() {}
void Remove() {}
void RemoveAppIcon() {}
void RemoveAppWindow() {}
void Rename() {}
void RenameIdentifier() {}
void round() {}
void rtAllocRequest() {}
void rtAllocRequestA() {}
void rtChangeReqAttr() {}
void rtEZRequest() {}
void rtFileRequest() {}
void rtFontRequest() {}
void rtFreeFileList() {}
void rtFreeRequest() {}
void rtGetLong() {}
void rtGetString() {}
void rtLockWindow() {}
void rtPaletteRequest() {}
void rtScreenModeRequest() {}
void rtUnlockWindow() {}
void ScreenToFront() {}
void Script() {}
void ScrollRaster() {}
void Send() {}
void SendIO() {}
void SetAfPt() {}
void SetComment() {}
void SetFileHandler() {}
void SetMenuStrip() {}
void SetRexxVar() {}
void SetRGB32() {}
void SetRGB4() {}
void SetSoftStyle() {}
void SetTaskPri() {}
void SetupCompiled() {}
void SetVar() {}
void Signal() {}
void SizeWindow() {}
void sortFileName() {}
void Sscanf() {}
void StopChunk() {}
void Strcasecmp() {}
void Strdup() {}
void strnicmp() {}
void SUBNUM() {}
void SystemTags() {}
void Text() {}
void TextExtent() {}
void type() {}
void TypeMem() {}
void UnLoadSeg() {}
void UnLockDosList() {}
void UnlockPubScreenList() {}
void upper_digits() {}
void UtilityBase() {}
void WaitIO() {}
void WaitPort() {}
void WindowToBack() {}
void WindowToFront() {}
void WorkbenchBase() {}
void WriteChunkBytes() {}

struct Library * SysBase;
struct Library * IFFParseBase;

void * Lock() {
    return 0;
}

void UnLock(void * foo) {
}

void RectFill() {}


#endif
