/* 
    Copyright � 1999, David Le Corfec.
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#ifndef __SLIDERDATA_H__
#define __SLIDERDATA_H__

#include <eventhandler.h>

struct MUI_SliderData
{
    struct Gadget *gadget;
    ULONG flags;
    struct MUI_EventHandlerNode ehn;
};


enum flags {
    SLIDER_HORIZ = (1<<0),
    SLIDER_QUIET = (1<<1),
};


#endif
