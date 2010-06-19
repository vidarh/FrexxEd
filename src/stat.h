#ifndef __FREXXED_STAT
#define __FREXXED_STAT

#ifndef S_IWRITE

#define S_ISUID 0004000/* set user id on execution */
#define S_ISGID 0002000/* set group id on execution */
#define S_ISVTX 0001000/* save swapped text even after use */

#define S_IRWXU 0000700/* RWX mask for owner */
#define S_IRUSR 0000400/* R for owner */
#define S_IWUSR 0000200/* W for owner */
#define S_IXUSR 0000100/* X for owner */

#define S_IREAD  S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC  S_IXUSR

#define S_IRWXG 0000070/* RWX mask for group */
#define S_IRGRP 0000040/* R for group */
#define S_IWGRP 0000020/* W for group */
#define S_IXGRP 0000010/* X for group */

#define S_IRWXO 0000007/* RWX mask for other */
#define S_IROTH 0000004/* R for other */
#define S_IWOTH 0000002/* W for other */
#define S_IXOTH 0000001/* X for other */

#define S_IHIDDEN    (1<<7)
#define S_ISCRIPT    (1<<6)
#define S_IPURE      (1<<5)
#define S_IARCHIVE   (1<<4)
#define S_IEXECUTE   (1<<1)
#define S_IDELETE    (1<<0)

#endif

#endif
