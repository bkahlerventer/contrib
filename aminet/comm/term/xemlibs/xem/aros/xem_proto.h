#ifndef PROTO_XEM_H
#define PROTO_XEM_H

/*
    (C) 1995-2003 AROS - The Amiga Research OS
*/

#ifndef AROS_SYSTEM_H
#   include <aros/system.h>
#endif

#ifndef XEmulatorBase
extern struct Library * XEmulatorBase;
#endif

//#include <clib/xem_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#   include <inline/xem.h>
#else
#   include <defines/xem.h>
#endif

#endif /* PROTO_XEM_H */
