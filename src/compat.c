
#include "compat.h"
#ifndef AMIGA
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void * AllocMem(long size, int flags) {
    return malloc(size);
}

void FreeMem(void * addr, long size) {
    free(addr);
}

int stricmp(const char * src, const char * dest) {
    return strcasecmp(src,dest);
}

int strnicmp(const char * src, const char * dest, size_t n) {
    return strncasecmp(src,dest,n);
}

int Stricmp(const char * src, const char * dest) {
    return strcasecmp(src,dest);
}

int Strnicmp(const char * src, const char * dest, int num) {
    return strncasecmp(src,dest, num);
}

void * Open(const char * filename, int flags) {
    fprintf(stderr,"Open '%s', %d\n",filename,flags);
    return 0;
}

void Delay(long num) {
    sleep(num);
}

void * CreateGadget() { 
    fprintf(stderr,"CreateGadget\n");
    return 0;
}

int AvailMem() { 
    fprintf(stderr,"AvailMem\n");
    return 0;
}

void Close() { 
    fprintf(stderr,"Close\n");
}
void ToUpper() { 
    fprintf(stderr,"ToUpper\n");
}

void DisplayBeep() {
    fprintf(stderr,"DisplayBeep\n");
}
void RefreshGList() {
    fprintf(stderr,"RefreshGList\n");
}
void CloseWindow() { 
    fprintf(stderr,"CloseWindow\n");
}
void SetWindowTitles() { 
    fprintf(stderr,"SetWindowTitles\n");
}
long Write() { 
    fprintf(stderr,"Write() called\n");
    return 0;
}
void DeleteFile() { 
    fprintf(stderr,"DeleteFile\n");
}
int IoErr() { 
    fprintf(stderr,"IOErr\n");
    return 0;
}
void SetDrMd() { }
void Forbid() { }
void Permit() { }
void SetWrMsk() {}

void AllocIFF() { }
void Fault() { }

char * NameFromLock() { 
    fprintf(stderr,"NameFromLock\n");
    return 0;
}

void PathPart() { }

int MatchFirst(const char * wildcard, void * file) { 
    fprintf(stderr,"MatchFirst '%s',%p\n",wildcard,file);
    return 0;
}

void MatchEnd(void * file) {
    fprintf(stderr,"MatchEnd %p\n",file);
}

int MatchNext(void * file) {
    fprintf(stderr,"MatchNext %p\n",file);
    return 1;
}


int LayoutMenus() { 
    return 1;
}

struct Menu * CreateMenus(struct NewMenu * menulist) { 
    fprintf(stderr,"CreateMenus\n");
    static struct Menu menu;
    memset(&menu, 0, sizeof(struct Menu));
    return & menu;
}

void TextLength() { }
void SetFont() { }
void FreeMenus() { }

void MallocCycle() { }

void FreeCycle() { }
void * OpenLibrary(const STRPTR name, long version) {
    fprintf(stderr,"OpenLibrary '%s'\n",name);
    return 0;
}

#include <assert.h>

void Eat() { }
struct Message * GetMsg(struct MsgPort * port) { 
    struct IntuiMessage msg;
    assert(port == 0 || port > 1024);
    fprintf(stderr,"GetMsg %p\n",port);
    sleep(1);

    memset(&msg, 0, sizeof(struct IntuiMessage));
    msg.Class = IDCMP_INTUITICKS;
    return &msg;
}
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
void FreeKind() { }

struct Task * FindTask(void * name) { 
    static struct Process task;
    task.pr_WindowPtr = 0;
    task.pr_MsgPort = 12345;
    return (struct Task *)&task;
}
int Wait(int foo) { 
    fprintf(stderr,"Wait\n");
    return 0;
}
int CloseScreen(struct Screen * s) {
    fprintf(stderr,"CloseScreen %p\n",s);
    return 0;
}

void CurrentDir() {
    fprintf(stderr,"CurrentDir\n");
}
void FreeArgs(void * args) {
    free(args);
    fprintf(stderr,"FreeArgs\n");
}
FILE * Output() {
    return stdout;
}
void PrintFault(LONG code, STRPTR header) {
    fprintf(stdout,"%s%sFault %d\n", header ? header : "", header ? ": " : "", code);
}

long Read() { 
    fprintf(stderr,"Read\n"); 
    return 0;
}
struct RDArgs * ReadArgs(const char * template, IPTR * opts, void * _) {
    struct RDArgs * r = (void *)malloc(sizeof(struct RDArgs));
    memset(r, 0, sizeof(struct RDArgs));
    fprintf(stderr,"ReadArgs: '%s'\n", template);
    return r;
}

void Malloc() { fprintf(stderr,"Malloc()\n"); }
void GetRGB4() { fprintf(stderr,"GetRGB4()\n"); }
void SetProtection() { fprintf(stderr,"SetProtection()\n"); }

struct Screen * LockPubScreen(const char * name) { 
    static struct Screen defaultpub;
    static short locked = 0;
    fprintf(stderr,"LockPubScreen('%s')\n",name); 
    if (!name && !locked) {
        memset(&defaultpub,0,sizeof(struct Screen));
    }
    if (!name) return &defaultpub;
    return 0; 
}
void RemoveGadget() { fprintf(stderr,"RemoveGadget()\n"); }
void AbortIO() { fprintf(stderr,"AbortIO()\n"); }
void ActivateGadget() { fprintf(stderr,"ActivateGadget()\n"); }
void ActivateWindow() { fprintf(stderr,"ActivateWindow()\n"); }
void AddAppIcon() { fprintf(stderr,"AddAppIcon()\n"); }
void AddAppWindow() { fprintf(stderr,"AddAppWindow()\n"); }
void AddDosEntry() { fprintf(stderr,"AddDosEntry()\n"); }
void AddGadget() { fprintf(stderr,"AddGadget()\n"); }
void AddLevel() { fprintf(stderr,"AddLevel()\n"); }
void AddVar() { fprintf(stderr,"AddVar()\n"); }
void ARG0() { fprintf(stderr,"ARG0()\n"); }
void ArrayNum() { fprintf(stderr,"ArrayNum()\n"); }
void ArrayResize() { fprintf(stderr,"ArrayResize()\n"); }
void AssignArg() { fprintf(stderr,"AssignArg()\n"); }
void AttemptLockDosList() { fprintf(stderr,"AttemptLockDosList()\n"); }
void BitToggle() { fprintf(stderr,"BitToggle()\n"); }
void CacheClearU() { fprintf(stderr,"CacheClearU()\n"); }
void ChangeWindowBox() { fprintf(stderr,"ChangeWindowBox()\n"); }
void CheckIO() { fprintf(stderr,"CheckIO()\n"); }
void CheckRexxMsg() { fprintf(stderr,"CheckRexxMsg()\n"); }
void ClearMenuStrip() { fprintf(stderr,"ClearMenuStrip()\n"); }
void CloseCatalog() { fprintf(stderr,"CloseCatalog()\n"); }
void CloseClipboard() { fprintf(stderr,"CloseClipboard()\n"); }
void CloseDevice() { fprintf(stderr,"CloseDevice()\n"); }
void CloseFont() { fprintf(stderr,"CloseFont()\n"); }
void CloseIFF() { fprintf(stderr,"CloseIFF()\n"); }
void CmpBreak() { fprintf(stderr,"CmpBreak()\n"); }
void CmpDeclare() { fprintf(stderr,"CmpDeclare()\n"); }
void CmpExport() { fprintf(stderr,"CmpExport()\n"); }
void CmpExpr() { fprintf(stderr,"CmpExpr()\n"); }
void CmpReset() { fprintf(stderr,"CmpReset()\n"); }
void CmpSwitch() { fprintf(stderr,"CmpSwitch()\n"); }
void CompareDates() { fprintf(stderr,"CompareDates()\n"); }
void CreateArgstring() { fprintf(stderr,"CreateArgstring()\n"); }
void CreateContext() { fprintf(stderr,"CreateContext()\n"); }
void CreateExtIO() { fprintf(stderr,"CreateExtIO()\n"); }
void CreateIORequest() { fprintf(stderr,"CreateIORequest()\n"); }

struct MsgPort * CreateMsgPort() { 
    struct MsgPort * port;
    port = malloc(sizeof(struct MsgPort));
    if (!port) return 0;
    memset(port,0,sizeof(struct MsgPort));
    fprintf(stderr,"CreateMsgPort()\n"); 
    return port;
}

void CreateNewProcTagList() { fprintf(stderr,"CreateNewProcTagList()\n"); }

struct MsgPort * CreatePort() { 
    static struct MsgPort port;
    memset(&port,0,sizeof(struct MsgPort));
    fprintf(stderr,"CreatePort()\n"); 
    return &port;
}

void CreateRexxMsg() { fprintf(stderr,"CreateRexxMsg()\n"); }
void CurrentChunk() { fprintf(stderr,"CurrentChunk()\n"); }
void DateStamp() { fprintf(stderr,"DateStamp()\n"); }
void DateToStr() { fprintf(stderr,"DateToStr()\n"); }
void Dealloc() { fprintf(stderr,"Dealloc()\n"); }
void DefaultAlloc() { fprintf(stderr,"DefaultAlloc()\n"); }
void DefaultDealloc() { fprintf(stderr,"DefaultDealloc()\n"); }
void DeleteArgstring() { fprintf(stderr,"DeleteArgstring()\n"); }
void DeleteExtIO() { fprintf(stderr,"DeleteExtIO()\n"); }
void DeleteIORequest() { fprintf(stderr,"DeleteIORequest()\n"); }
void DeleteMessage() { fprintf(stderr,"DeleteMessage()\n"); }
void DeletePort() { fprintf(stderr,"DeletePort()\n"); }
void DeleteRexxMsg() { fprintf(stderr,"DeleteRexxMsg()\n"); }
void DelIdentifier() { fprintf(stderr,"DelIdentifier()\n"); }
void DelLocalVar() { fprintf(stderr,"DelLocalVar()\n"); }
void DelProgram() { fprintf(stderr,"DelProgram()\n"); }
void DiskfontBase() { fprintf(stderr,"DiskfontBase()\n"); }
void DOSBase() { fprintf(stderr,"DOSBase()\n"); }
void DoubleClick() { fprintf(stderr,"DoubleClick()\n"); }
void DrawBorder() { fprintf(stderr,"DrawBorder()\n"); }

void EasyRequestArgs(void * p1, struct EasyStruct * req, void * p2, void * p3) { 
    fprintf(stderr,"EasyRequestArgs(%p,%p,%p,%p)\n",p1,req,p2,p3); 
    fprintf(stderr,"Msg: %s\n",req->es_TextFormat);
    fprintf(stderr,"Buttons: %s\n",req->es_GadgetFormat);
}


void Examine() { fprintf(stderr,"Examine()\n"); }
void fileReqFileListNotEmpty() { fprintf(stderr,"fileReqFileListNotEmpty()\n"); }
void FindDosEntry() { fprintf(stderr,"FindDosEntry()\n"); }
struct Node * FindName(struct Node *node, const char * name) { 
    fprintf(stderr,"FindName(%p,%s)\n",node, name);
    return 0;
}
void FindPort() { fprintf(stderr,"FindPort()\n"); }
void FindProp() { fprintf(stderr,"FindProp()\n"); }
void FlushFree() { fprintf(stderr,"FlushFree()\n"); }
void fpl_functions() { fprintf(stderr,"fpl_functions()\n"); }
void FreeAll() { fprintf(stderr,"FreeAll()\n"); }
void FreeDiskObject() { fprintf(stderr,"FreeDiskObject()\n"); }
void FreeDosEntry() { fprintf(stderr,"FreeDosEntry()\n"); }
void FreeGadgets() { fprintf(stderr,"FreeGadgets()\n"); }
void FreeIFF() { fprintf(stderr,"FreeIFF()\n"); }
void FreeVec() { fprintf(stderr,"FreeVec()\n"); }
void FreeVisualInfo() { fprintf(stderr,"FreeVisualInfo()\n"); }
void functions() { fprintf(stderr,"functions()\n"); }
void GadToolsBase() { fprintf(stderr,"GadToolsBase()\n"); }
void * GetCatalogStr(void * catalog, long offset, const char * str) { 
#ifdef DEBUG
    //    fprintf(stderr,"GetCatalogStr(%p,%d,%s)\n",catalog,offset,str);
#endif
    return str; 
}
void GetDisplayInfoData() { fprintf(stderr,"GetDisplayInfoData()\n"); }
void GetErrorMsg() { fprintf(stderr,"GetErrorMsg()\n"); }
void GetLong() { fprintf(stderr,"GetLong()\n"); }
void GetMessage() { fprintf(stderr,"GetMessage()\n"); }
void GetPrefs() { fprintf(stderr,"GetPrefs()\n"); }
void GetProgram() { fprintf(stderr,"GetProgram()\n"); }
void GetProgramName() { fprintf(stderr,"GetProgramName()\n"); }
//void getreg() { fprintf(stderr,"getreg()\n"); }
void GetRexxVar() { fprintf(stderr,"GetRexxVar()\n"); }
void GetRGB32() { fprintf(stderr,"GetRGB32()\n"); }
struct DrawInfo * GetScreenDrawInfo(struct Screen * scr) { 
    static struct DrawInfo dri;
    fprintf(stderr,"GetScreenDrawInfo()\n"); 
    return &dri;
}

void GetShort() { fprintf(stderr,"GetShort()\n"); }
int GetVar(const char * var, STRPTR buffer, int len, int flags) { 
    fprintf(stderr,"GetVar('%s')\n",var); 
    buffer[0] = 0;
    return 0;
}
void GetVisualInfo() { fprintf(stderr,"GetVisualInfo()\n"); }
long GetVPModeID() { fprintf(stderr,"GetVPModeID()\n"); return 0; }
void Getword() { fprintf(stderr,"Getword()\n"); }
void GfxBase() { fprintf(stderr,"GfxBase()\n"); }
void GT_GetGadgetAttrs() { fprintf(stderr,"GT_GetGadgetAttrs()\n"); }
void GT_GetIMsg() { fprintf(stderr,"GT_GetIMsg()\n"); }
void GTMENUITEM_USERDATA() { fprintf(stderr,"GTMENUITEM_USERDATA()\n"); }
void GT_RefreshWindow() { fprintf(stderr,"GT_RefreshWindow()\n"); }
void GT_ReplyIMsg() { fprintf(stderr,"GT_ReplyIMsg()\n"); }
void GT_SetGadgetAttrs() { fprintf(stderr,"GT_SetGadgetAttrs()\n"); }
void IconBase() { fprintf(stderr,"IconBase()\n"); }
void InitFree() { fprintf(stderr,"InitFree()\n"); }
void InitIFFasClip() { fprintf(stderr,"InitIFFasClip()\n"); }
void InitIFFasDOS() { fprintf(stderr,"InitIFFasDOS()\n"); }
void InitRastPort() { fprintf(stderr,"InitRastPort()\n"); }
void InitSemaphore() { fprintf(stderr,"InitSemaphore()\n"); }
void InterfaceCall() { fprintf(stderr,"InterfaceCall()\n"); }
void IntuitionBase() { fprintf(stderr,"IntuitionBase()\n"); }
void ItemAddress() { fprintf(stderr,"ItemAddress()\n"); }
void ITEMNUM() { fprintf(stderr,"ITEMNUM()\n"); }
void LeaveProgram() { fprintf(stderr,"LeaveProgram()\n"); }
void LoadSeg() { fprintf(stderr,"LoadSeg()\n"); }
struct List * LockPubScreenList() { 
    fprintf(stderr,"LockPubScreenList()\n"); 
    return 0;
}
void MakeDosEntry() { fprintf(stderr,"MakeDosEntry()\n"); }
void MAKE_ID() { fprintf(stderr,"MAKE_ID()\n"); }
void MENUNUM() { fprintf(stderr,"MENUNUM()\n"); }
void MKBADDR() { fprintf(stderr,"MKBADDR()\n"); }
void Move() { fprintf(stderr,"Move()\n"); }
void my_memicmp() { fprintf(stderr,"my_memicmp()\n"); }
void Newline() { fprintf(stderr,"Newline()\n"); }
void NewModifyProp() { fprintf(stderr,"NewModifyProp()\n"); }
void ObtainSemaphore(void * sem) { 
    fprintf(stderr,"ObtainSemaphore()\n"); 
}
void OffGadget() { fprintf(stderr,"OffGadget()\n"); }
void OnGadget() { fprintf(stderr,"OnGadget()\n"); }
void OpenCatalogA() { fprintf(stderr,"OpenCatalogA()\n"); }
void OpenClipboard() { fprintf(stderr,"OpenClipboard()\n"); }
void OpenDevice() { fprintf(stderr,"OpenDevice()\n"); }
void OpenDiskFont() { fprintf(stderr,"OpenDiskFont()\n"); }

struct TextFont * OpenFont(struct TextAttr * font) { 
    static struct TextFont fonts;
    fprintf(stderr,"OpenFont(%s)\n", font->ta_Name); 
    memset(&fonts,0,sizeof(struct TextFont));
    fonts.tf_XSize = 8;
    fonts.tf_YSize = 8;
    return &fonts;
}

void OpenIFF() { fprintf(stderr,"OpenIFF()\n"); }

struct Screen * OpenScreenTags() { 
    fprintf(stderr,"OpenScreenTags()\n"); 
    return 0;
}

struct Window * OpenWindowTags(struct NewWindow * newwin, ...) { 
    static struct Window win;
    fprintf(stderr,"OpenWindowTags('%s')\n",newwin->Title); 
    memset(&win, 0, sizeof(struct Window));
    return &win;
}

void ParentDir() { fprintf(stderr,"ParentDir()\n"); }
void ParseIFF() { fprintf(stderr,"ParseIFF()\n"); }
void PopChunk() { fprintf(stderr,"PopChunk()\n"); }
void PropChunk() { fprintf(stderr,"PropChunk()\n"); }
void PubScreenStatus() { fprintf(stderr,"PubScreenStatus()\n"); }
void PushChunk() { fprintf(stderr,"PushChunk()\n"); }
void PutDiskObject() { fprintf(stderr,"PutDiskObject()\n"); }
void PutMsg() { fprintf(stderr,"PutMsg()\n"); }
void putreg() { fprintf(stderr,"putreg()\n"); }
void RawDoFmt() { fprintf(stderr,"RawDoFmt()\n"); }
void RawKeyConvert() { fprintf(stderr,"RawKeyConvert()\n"); }
void ReadChunkBytes() { fprintf(stderr,"ReadChunkBytes()\n"); }
void ReleaseSemaphore() { fprintf(stderr,"ReleaseSemaphore()\n"); }
void RemDosEntry() { fprintf(stderr,"RemDosEntry()\n"); }
void Remove() { fprintf(stderr,"Remove()\n"); }
void RemoveAppIcon() { fprintf(stderr,"RemoveAppIcon()\n"); }
void RemoveAppWindow() { fprintf(stderr,"RemoveAppWindow()\n"); }
void Rename() { fprintf(stderr,"Rename()\n"); }
void RenameIdentifier() { fprintf(stderr,"RenameIdentifier()\n"); }
void round() { fprintf(stderr,"round()\n"); }
void rtAllocRequest() { fprintf(stderr,"rtAllocRequest()\n"); }
void rtAllocRequestA() { fprintf(stderr,"rtAllocRequestA()\n"); }
void rtChangeReqAttr() { fprintf(stderr,"rtChangeReqAttr()\n"); }
void rtEZRequest() { fprintf(stderr,"rtEZRequest()\n"); }
void rtFileRequest() { fprintf(stderr,"rtFileRequest()\n"); }
void rtFontRequest() { fprintf(stderr,"rtFontRequest()\n"); }
void rtFreeFileList() { fprintf(stderr,"rtFreeFileList()\n"); }
void rtFreeRequest(void * p1) { 
    fprintf(stderr,"rtFreeRequest(%p)\n",p1); 
}
void rtGetLong() { fprintf(stderr,"rtGetLong()\n"); }
void rtGetString() { fprintf(stderr,"rtGetString()\n"); }
void rtLockWindow() { fprintf(stderr,"rtLockWindow()\n"); }
void rtPaletteRequest() { fprintf(stderr,"rtPaletteRequest()\n"); }
void rtScreenModeRequest() { fprintf(stderr,"rtScreenModeRequest()\n"); }
void rtUnlockWindow() { fprintf(stderr,"rtUnlockWindow()\n"); }
void ScreenToFront() { fprintf(stderr,"ScreenToFront()\n"); }
void Script() { fprintf(stderr,"Script()\n"); }
void ScrollRaster() { fprintf(stderr,"ScrollRaster()\n"); }
void Send() { fprintf(stderr,"Send()\n"); }
void SendIO() { fprintf(stderr,"SendIO()\n"); }
void SetAfPt() { fprintf(stderr,"SetAfPt()\n"); }
void SetComment() { fprintf(stderr,"SetComment()\n"); }
void SetFileHandler() { fprintf(stderr,"SetFileHandler()\n"); }
int SetMenuStrip() { fprintf(stderr,"SetMenuStrip()\n"); 
    return 1;
}
void SetRexxVar() { fprintf(stderr,"SetRexxVar()\n"); }
void SetRGB32() { fprintf(stderr,"SetRGB32()\n"); }
void SetRGB4() { fprintf(stderr,"SetRGB4()\n"); }
void SetSoftStyle() { fprintf(stderr,"SetSoftStyle()\n"); }
void SetTaskPri() { fprintf(stderr,"SetTaskPri()\n"); }
void SetupCompiled() { fprintf(stderr,"SetupCompiled()\n"); }
void SetVar() { fprintf(stderr,"SetVar()\n"); }
void Signal() { fprintf(stderr,"Signal()\n"); }
void SizeWindow() { fprintf(stderr,"SizeWindow()\n"); }
void sortFileName() { fprintf(stderr,"sortFileName()\n"); }
void Sscanf() { fprintf(stderr,"Sscanf()\n"); }
void StopChunk() { fprintf(stderr,"StopChunk()\n"); }
void Strcasecmp() { fprintf(stderr,"Strcasecmp()\n"); }
void Strdup() { fprintf(stderr,"Strdup()\n"); }

void SUBNUM() { fprintf(stderr,"SUBNUM()\n"); }
void SystemTags() { fprintf(stderr,"SystemTags()\n"); }
void Text(struct RastPort * rp, const char * str, int len) { 
    fprintf(stderr,"Text(%p,'%s',%d)\n",rp,str,len);
}
void TextExtent() { fprintf(stderr,"TextExtent()\n"); }
void type() { fprintf(stderr,"type()\n"); }
void TypeMem() { fprintf(stderr,"TypeMem()\n"); }
void UnLoadSeg() { fprintf(stderr,"UnLoadSeg()\n"); }
void UnLockDosList() { fprintf(stderr,"UnLockDosList()\n"); }
void UnlockPubScreenList() { fprintf(stderr,"UnlockPubScreenList()\n"); }
void upper_digits() { fprintf(stderr,"upper_digits()\n"); }
void UtilityBase() { fprintf(stderr,"UtilityBase()\n"); }
void WaitIO() { fprintf(stderr,"WaitIO()\n"); }
void WaitPort() { fprintf(stderr,"WaitPort()\n"); }
void WindowToBack() { fprintf(stderr,"WindowToBack()\n"); }
void WindowToFront() { fprintf(stderr,"WindowToFront()\n"); }
void WorkbenchBase() { fprintf(stderr,"WorkbenchBase()\n"); }
void WriteChunkBytes() { fprintf(stderr,"WriteChunkBytes()\n"); }
void RectFill() { fprintf(stderr,"RectFill()\n"); }

struct Library * SysBase;
struct Library * IFFParseBase;

void * Lock() {
    fprintf(stderr,"Lock\n");
    return 0;
}

void UnLock(void * foo) {
    fprintf(stderr,"UnLock\n");
}


#endif
