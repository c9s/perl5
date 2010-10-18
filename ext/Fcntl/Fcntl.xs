#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef VMS
#  include <file.h>
#else
#if defined(__GNUC__) && defined(__cplusplus) && defined(WIN32)
#define _NO_OLDNAMES
#endif 
#  include <fcntl.h>
#if defined(__GNUC__) && defined(__cplusplus) && defined(WIN32)
#undef _NO_OLDNAMES
#endif 
#endif

#ifdef I_UNISTD
#include <unistd.h>
#endif

/* This comment is a kludge to get metaconfig to see the symbols
    VAL_O_NONBLOCK
    VAL_EAGAIN
    RD_NODATA
    EOF_NONBLOCK
   and include the appropriate metaconfig unit
   so that Configure will test how to turn on non-blocking I/O
   for a file descriptor.  See config.h for how to use these
   in your extension. 
   
   While I'm at it, I'll have metaconfig look for HAS_POLL too.
   --AD  October 16, 1995
*/

static XS(XS_Fcntl_S_ISREG); /* prototype to pass -Wmissing-prototypes */
static
XS(XS_Fcntl_S_ISREG)
{
    dVAR;
    dXSARGS;
    dXSI32;
    /* Preserve the semantics of the perl code, which was:
       sub S_ISREG    { ( $_[0] & _S_IFMT() ) == S_IFREG()   }
    */
    SV *mode;

    PERL_UNUSED_VAR(cv); /* -W */
    SP -= items;

    if (items > 0)
	mode = ST(0);
    else {
	mode = &PL_sv_undef;
	EXTEND(SP, 1);
    }
    PUSHs(((SvUV(mode) & S_IFMT) == ix) ? &PL_sv_yes : &PL_sv_no);
    PUTBACK;
}

#include "const-c.inc"

MODULE = Fcntl		PACKAGE = Fcntl

INCLUDE: const-xs.inc

BOOT:
    {
        CV *cv;
#ifdef S_IFREG
        cv = newXS("Fcntl::S_ISREG", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFREG;
#endif
#ifdef S_IFDIR
        cv = newXS("Fcntl::S_ISDIR", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFDIR;
#endif
#ifdef S_IFLNK
        cv = newXS("Fcntl::S_ISLNK", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFLNK;
#endif
#ifdef S_IFSOCK
        cv = newXS("Fcntl::S_ISSOCK", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFSOCK;
#endif
#ifdef S_IFBLK
        cv = newXS("Fcntl::S_ISBLK", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFBLK;
#endif
#ifdef S_IFCHR
        cv = newXS("Fcntl::S_ISCHR", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFCHR;
#endif
#ifdef S_IFIFO
        cv = newXS("Fcntl::S_ISFIFO", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFIFO;
#endif
#ifdef S_IFWHT
        cv = newXS("Fcntl::S_ISWHT", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_IFWHT;
#endif
#ifdef S_IFENFMT
        cv = newXS("Fcntl::S_ISENFMT", XS_Fcntl_S_ISREG, file);
        XSANY.any_i32 = S_ENFMT;
#endif
    }
