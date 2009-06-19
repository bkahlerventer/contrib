/***************************************************************************

 NListtree.mcc - New Listtree MUI Custom Class
 Copyright (C) 1999-2001 by Carsten Scholling
 Copyright (C) 2001-2007 by NList Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 NList classes Support Site:  http://www.sf.net/projects/nlist-classes

 $Id$

***************************************************************************/

 /*
  * If this file is named 'locale.c' it is automatically
  * generated by 'FlexCat' from C_c.sd. PLEASE DO NOT EDIT!!!
  */

#include <proto/locale.h>

#include "SDI_compiler.h"
#include "Debug.h"

struct FC_Type { const long ID; const char * const Str; };

const struct FC_Type _MSG_TAB_LAYOUT = { 1, "Layout" };
const struct FC_Type _MSG_GROUP_IMAGES_COLORS = { 2, "Images/Colors" };
const struct FC_Type _MSG_BUTTON_LABEL_CLOSED = { 3, "Closed" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_CLOSED = { 4, "c" };
const struct FC_Type _MSG_HELP_CLOSED_IMAGE = { 5, "Image to use for closed nodes." };
const struct FC_Type _MSG_WIN_TITLE_CLOSED_IMAGE = { 6, "Adjust image for closed nodes" };
const struct FC_Type _MSG_BUTTON_LABEL_LINES = { 7, "Lines" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_LINES = { 8, "l" };
const struct FC_Type _MSG_HELP_LINES_COLOR = { 9, "Color to use for lines." };
const struct FC_Type _MSG_WIN_TITLE_LINES_COLOR = { 10, "Adjust lines color" };
const struct FC_Type _MSG_BUTTON_LABEL_OPEN = { 11, "Open" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_OPEN = { 12, "o" };
const struct FC_Type _MSG_HELP_OPEN_IMAGE = { 13, "Image to use for open nodes." };
const struct FC_Type _MSG_WIN_TITLE_OPEN_IMAGE = { 14, "Adjust image for open nodes" };
const struct FC_Type _MSG_BUTTON_LABEL_SHADOW = { 15, "Shadow" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_SHADOW = { 16, "w" };
const struct FC_Type _MSG_HELP_SHADOW_COLOR = { 17, "Color to use to cast\n"\
	"shadows after lines." };
const struct FC_Type _MSG_WIN_TITLE_SHADOW_COLOR = { 18, "Adjust shadow color" };
const struct FC_Type _MSG_BUTTON_LABEL_SPECIAL = { 19, "Special" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_SPECIAL = { 20, "i" };
const struct FC_Type _MSG_HELP_SPECIAL_IMAGE = { 21, "A special image to be\n"\
	"displayed after every node." };
const struct FC_Type _MSG_WIN_TITLE_SPECIAL_IMAGE = { 22, "Adjust special image" };
const struct FC_Type _MSG_BUTTON_LABEL_DRAW = { 23, "Draw" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_DRAW = { 24, "d" };
const struct FC_Type _MSG_HELP_DRAW_COLOR = { 25, "A draw color to be combined\n"\
	"with the lines color." };
const struct FC_Type _MSG_WIN_TITLE_DRAW_COLOR = { 26, "Adjust draw color" };
const struct FC_Type _MSG_GROUP_STYLE = { 27, "Style" };
const struct FC_Type _MSG_CYCLE_LABEL_STYLE = { 28, "Style" };
const struct FC_Type _MSG_CYCLE_SHORTCUT_STYLE = { 29, "s" };
const struct FC_Type _MSG_HELP_STYLE = { 30, "Select here the global style of the tree.\n"\
	"\n"\
	"Depending on your choice, some of the\n"\
	"options above may become (un)available." };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_NORMAL = { 31, "Normal" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_INSERTED = { 32, "Inserted" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_LINES = { 33, "Lines" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_WIN98 = { 34, "Win98" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_MAC = { 35, "Mac" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_LINES3D = { 36, "Lines 3D" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_WIN98PLUS = { 37, "Win98+" };
const struct FC_Type _MSG_CYCLE_ITEM_STYLE_GLOW = { 38, "Glow" };
const struct FC_Type _MSG_BUTTON_LABEL_SPACE = { 39, "Spacing" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_SPACE = { 40, "p" };
const struct FC_Type _MSG_HELP_SPACE = { 41, "The more spacing, the wider\n"\
	"the listtree will be." };
const struct FC_Type _MSG_BUTTON_LABEL_REMEMBER = { 42, "Remember status?" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_REMEMBER = { 43, "r" };
const struct FC_Type _MSG_HELP_REMEMBER = { 44, "If enabled, the open/closed status of nodes\n"\
	"will be 'remembered' after the listtree is closed\n"\
	"the next time it is opened again.\n"\
	"\n"\
	"Please note, however, that an application\n"\
	"can override your choice here." };
const struct FC_Type _MSG_BUTTON_LABEL_AUTOSCROLL = { 45, "Autoscroll?" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_AUTOSCROLL = { 46, "a" };
const struct FC_Type _MSG_HELP_AUTOSCROLL = { 47, "If enabled, the listview will auto-scroll its\n"\
	"contents when opening nodes to try and fit the\n"\
	"visible area." };
const struct FC_Type _MSG_TAB_EXAMPLE = { 48, "Example" };
const struct FC_Type _MSG_HELP_LISTVIEW_EXAMPLE = { 49, "This NListtree object reacts in real time to\n"\
	"the changes you apply under the 'Layout' tab.\n"\
	"\n"\
	"Give it a try! :)" };
const struct FC_Type _MSG_BUTTON_LABEL_EXPAND = { 50, "Expand" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_EXPAND = { 51, "n" };
const struct FC_Type _MSG_BUTTON_LABEL_COLLAPSE = { 52, "Collapse" };
const struct FC_Type _MSG_BUTTON_SHORTCUT_COLLAPSE = { 53, "e" };

static struct Catalog *NL_Catalog = NULL;

/*** Catalog functions ***/
/// tr()
const char *tr(const void *fcstr)
{
  const char *defaultstr = ((struct FC_Type *)fcstr)->Str;

  if(LocaleBase)
    return (const char *)GetCatalogStr(NL_Catalog, ((struct FC_Type *)fcstr)->ID, (STRPTR)defaultstr);

  return defaultstr;
}
///
/// CloseCat()
void CloseCat(void)
{
  ENTER();

  if(NL_Catalog)
  {
    CloseCatalog(NL_Catalog);
    NL_Catalog = NULL;
  }

  LEAVE();
}
///
/// OpenCat(void)
BOOL OpenCat(void)
{
  static const struct TagItem tags[] = {
                                         { OC_BuiltInLanguage, (IPTR)"english" },
                                         { OC_Version,         2 },
                                         { TAG_DONE,           0  }
                                       };

  ENTER();

  if(LocaleBase && NL_Catalog == NULL)
    NL_Catalog = OpenCatalogA(NULL, (STRPTR)"NListtree_mcp.catalog", (struct TagItem *)&tags[0]);

  RETURN((BOOL)(NL_Catalog != NULL));
  return (BOOL)(NL_Catalog != NULL);
}
///
