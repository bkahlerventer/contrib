/* 
    Copyright � 1999, David Le Corfec.
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#ifndef __EVENT_IMPL_H__
#define __EVENT_IMPL_H__

#include <gdk/gdktypes.h>

struct MUI_ApplicationData;

void __zune_main_do_event (GdkEvent *event);
void __application_wait_event (struct MUI_ApplicationData *data, gboolean may_block);

#endif
