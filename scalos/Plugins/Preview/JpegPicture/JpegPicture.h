// JpegPicture.h
// $Date$
// $Revision$

#ifndef JPEGPICTURE_H
#define JPEGPICTURE_H

//---------------------------------------------------------------

#include "plugin.h"

//---------------------------------------------------------------

// Debugging...
#define	d(x)	;
#define	d1(x)	;
#define	d2(x)	x;

// aus debug.lib
extern int kprintf(const char *fmt, ...);
extern int KPrintF(const char *fmt, ...);

//---------------------------------------------------------------

// defined in  mempools.lib

#ifndef __amigaos4__
extern int _STI_240_InitMemFunctions(void);
extern void _STD_240_TerminateMemFunctions(void);
#endif

//----------------------------------------------------------------------------

#endif /* JPEGPICTURE_H */

