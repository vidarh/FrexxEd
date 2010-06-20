#ifndef __FREXXED_UTIL_H
#define __FREXXED_UTIL_H
// Implemntation of a few SAS/C / Lattice C functions that are not standard

extern int stccpy(char *to, const char * from, int length);
extern int stcgfe(char *, const char *);
extern void strmfp(char *to, const char * path, const char * file);
extern void movmem(char *src, char *dest, int num);

inline int min(int left, int right) { return left <= right ? left : right; }
inline int max(int left, int right) { return left >= right ? left : right; }

#endif
