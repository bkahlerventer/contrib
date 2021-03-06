/***************************************************************************

 Mailtext - An MUI Custom Class for eMessage display
 Copyright (C) 1996-2001 by Olaf Peters
 Copyright (C) 2002-2006 by Mailtext Open Source Team

 GCC & OS4 Portage by Alexandre Balaban

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 Mailtext OpenSource project : http://sourceforge.net/projects/mailtext/

 $Id$

 Sourcecode for Mailtext.mcc

 MCC_Mailtext uses the following tags:

    0x805701xx

 Highest defined tagvalue:

 define MUICFG_Mailtext_Font         0x80570141

***************************************************************************/

/*
#define BETARELEASE "7"
*/
                                                                               /***/
/*** Include stuff ***/                                                        /***/
                                                                               /***/
#ifndef MAILTEXT_MCP_PRIVATE_H                                                 /***/
#define MAILTEXT_MCP_PRIVATE_H                                                 /***/
                                                                               /***/
#ifndef LIBRARIES_MUI_H                                                        /***/
#include "libraries/mui.h"                                                     /***/
#endif                                                                         /***/
                                                                               /***/
struct Data
{
            char    *regTitles[7] ;
            char    *text ;

            Object  *mailtextObj,
                    *distObj,
                    *qdistObj,
                    *disPatObj,
                    *enPatObj,
                    *ssPatObj,
                    *niCharsObj,
                    *termCharsObj,
                    *wsCharsObj,
                    *qCharsObj,
                    *showObj,
                    *contextObj,
                    *uQCharsObj,
                    *colors[5],
                    *textcolor,
                    *fontObj,
                    *urlcolor,
                    *group,
                    *sigSepObj,
                    *uulObj,
                    *ustackObj,
                    *uactStrObj,
                    *uabeepObj,
                    *wrapObj ;

    struct  Catalog *catalog;
};

#endif
