#ifndef _UNIT_H_
#define _UNIT_H_

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA.
*/

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <exec/errors.h>

#include <devices/sana2.h>
#include <devices/sana2specialstats.h>
#include <devices/newstyle.h>

#include <utility/utility.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/battclock.h>

#include <stdlib.h>

#include "rtl8139.h"
#include LC_LIBDEFS_FILE

IPTR RTL_AllocLONGAligned(struct RTL8139Unit *, IPTR *, int);
struct RTL8139Unit *CreateUnit(LIBBASETYPEPTR, OOP_Object *, ULONG, char *, char *);
void DeleteUnit(struct RTL8139Base *, struct RTL8139Unit *);
void FlushUnit(LIBBASETYPEPTR, struct RTL8139Unit *, UBYTE, BYTE);
BOOL AddressFilter(struct RTL8139Base *, struct RTL8139Unit *, UBYTE *);
VOID CopyPacket(struct RTL8139Base *, struct RTL8139Unit *, struct IOSana2Req *, UWORD, UWORD, struct eth_frame *);
VOID ReportEvents(struct RTL8139Base *, struct RTL8139Unit *, ULONG);
struct TypeStats *FindTypeStats(struct RTL8139Base *, struct RTL8139Unit *, struct MinList *, ULONG);

BOOL AddMulticastRange(LIBBASETYPEPTR, struct RTL8139Unit *, const UBYTE *, const UBYTE *);
BOOL RemMulticastRange(LIBBASETYPEPTR, struct RTL8139Unit *, const UBYTE *, const UBYTE *);

#endif //_UNIT_H_
