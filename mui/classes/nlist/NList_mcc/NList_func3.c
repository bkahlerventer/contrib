#include <proto/intuition.h>

#include "private.h"

/****************************************************************************************/
/****************************************************************************************/
/******************************                    **************************************/
/******************************     NList Class    **************************************/
/******************************                    **************************************/
/****************************************************************************************/
/****************************************************************************************/


/* init: struct InputEvent ievent = {NULL,IECLASS_RAWKEY,0,0,0} */


/*static const ULONG tricky=0x16c04e75; // move.b d0,(a3)+ ; rts */
/*
 * static void mysprintf(STRPTR buffer,STRPTR fmt,...)
 * { RawDoFmt(fmt,(ULONG *)&fmt+1,(void (*)())&tricky,buffer); }
 */


ULONG MyCallHookPkt(Object *obj,BOOL hdata,struct Hook *hook,APTR object,APTR message)
{
  if (hdata)
  { ULONG retval;
    APTR h_Data = hook->h_Data;
    if (!h_Data)
      hook->h_Data = obj;
    retval = CallHookPkt(hook,object,message);
    if (!h_Data)
      hook->h_Data = h_Data;
    return (retval);
  }
  else
    return (CallHookPkt(hook,object,message));
}


ULONG MyCallHookPktA(Object *obj,struct Hook *hook,APTR message,...)
{
/* see how lame this hack is :) probably it can be changed
   with an inline call. it can wait for emm's gcc patch too */
#ifdef MORPHOS
	va_list tags;
	ULONG mav[10];
	int i=1;

	va_start(tags, message);
	mav[0] = message;
	while(i<7)
	{
		mav[i] = va_arg(tags,ULONG);
		i++;
	}
	va_end(tags);
  return (MyCallHookPkt(obj,FALSE,hook,obj,&mav[0]));
#else
  return (MyCallHookPkt(obj,FALSE,hook,obj,&message));
#endif
}


LONG DeadKeyConvert(struct NLData *data,struct IntuiMessage *msg,UBYTE *buf,LONG bufsize,struct KeyMap *kmap)
{
  int posraw,pos,postext = 0;
  UBYTE *text = buf;

  if (msg->Class != IDCMP_RAWKEY)
    return (-2);

  text[0] = '\0';
/*
 *   if (msg->Qualifier)
 *   { D(bug("Qual:"));
 *     if (msg->Qualifier & IEQUALIFIER_LSHIFT)         D(bug("lshift "));
 *     if (msg->Qualifier & IEQUALIFIER_RSHIFT)         D(bug("rshift "));
 *     if (msg->Qualifier & IEQUALIFIER_CAPSLOCK)       D(bug("capslock "));
 *     if (msg->Qualifier & IEQUALIFIER_CONTROL)        D(bug("ctrl "));
 *     if (msg->Qualifier & IEQUALIFIER_LALT)           D(bug("lalt "));
 *     if (msg->Qualifier & IEQUALIFIER_RALT)           D(bug("ralt "));
 *     if (msg->Qualifier & IEQUALIFIER_LCOMMAND)       D(bug("lcommand "));
 *     if (msg->Qualifier & IEQUALIFIER_RCOMMAND)       D(bug("rcommand "));
 *     if (msg->Qualifier & IEQUALIFIER_NUMERICPAD)     D(bug("numpad "));
 *     if (msg->Qualifier & IEQUALIFIER_REPEAT)         D(bug("repeat "));
 *     if (msg->Qualifier & IEQUALIFIER_MIDBUTTON)      D(bug("mbutton "));
 *     if (msg->Qualifier & IEQUALIFIER_RBUTTON)        D(bug("rbutton "));
 *     if (msg->Qualifier & IEQUALIFIER_LEFTBUTTON)     D(bug("lbutton "));
 *     D(bug("\n"));
 *   }
 */
  switch (msg->Code & 0x7F)
  {
    case 0x41 : strcpy(text,"bs"); break;
    case 0x42 : strcpy(text,"tab"); break;
    case 0x43 : strcpy(text,"enter"); break;
    case 0x44 : strcpy(text,"return"); break;
    case 0x45 : strcpy(text,"esc"); break;
    case 0x46 : strcpy(text,"del"); break;
    case 0x4C : strcpy(text,"up"); break;
    case 0x4D : strcpy(text,"down"); break;
    case 0x4E : strcpy(text,"right"); break;
    case 0x4F : strcpy(text,"left"); break;
    case 0x50 : strcpy(text,"f1"); break;
    case 0x51 : strcpy(text,"f2"); break;
    case 0x52 : strcpy(text,"f3"); break;
    case 0x53 : strcpy(text,"f4"); break;
    case 0x54 : strcpy(text,"f5"); break;
    case 0x55 : strcpy(text,"f6"); break;
    case 0x56 : strcpy(text,"f7"); break;
    case 0x57 : strcpy(text,"f8"); break;
    case 0x58 : strcpy(text,"f9"); break;
    case 0x59 : strcpy(text,"f10"); break;
    case 0x5F : strcpy(text,"help"); break;
    default:
      data->ievent.ie_NextEvent = NULL;
      data->ievent.ie_Class = IECLASS_RAWKEY;
      data->ievent.ie_SubClass = 0;
      data->ievent.ie_Code = msg->Code;
      data->ievent.ie_Qualifier = 0;
      data->ievent.ie_position.ie_addr = *((APTR *) msg->IAddress);
      posraw = RawKeyConvert(&data->ievent,text,bufsize-postext-1,kmap);
      if (posraw >= 0)
        text[posraw+postext] = '\0';
      if (posraw > 0)
      {
        ULONG  realc;
        data->rawtext[posraw] = '\0';
        for (pos = 0; pos < posraw; pos++)
        {
          realc = data->rawtext[pos];
/*D(bug("RAWKEY KEY=%ld \n",realc));*/
          if ((realc <= 0x1F)||((realc >= 0x80)&&(realc < 0xa0)))
            data->rawtext[pos] = 0x7f;
        }
      }
      break;
  }
  return ((LONG)strlen(buf));
}


char *ltoa(ULONG val, char *buf, int len)
{
  ULONG v;
  if (len > 0)
    buf[--len] = '\0';
  if (val == 0)
  { if (len > 0) buf[--len] = '0'; }
  else
  { while (val)
    { v = val % 10;
      if (len > 0) buf[--len] = ('0' + ((char) v));
      val = val / 10;
    }
  }
  return (&buf[len]);
}


/*char *stpcpy(char *to,const char *from)*/
#ifndef __SASC
char *stpcpy(char *to,char *from)
{
  register char *to2 = to;
  register char *from2 = from;

  while (*from)
    *to2++ = *from++;
  *to2 = '\0';
  return (to2);
}
#endif


/*static char *stpncpy(char *to,const char *from,int len)*/
static char *stpncpy(char *to,char *from,int len)
{
  register char *to2 = to;
  register char *from2 = from;

  while (*from && (len > 0))
  { *to2++ = *from++;
    len--;
  }
  *to2 = '\0';
  return (to2);
}


/*static char *stpncpy_noesc(char *to,const char *from,int len)*/
static char *stpncpy_noesc(char *to,char *from,int len)
{
  register char *to2 = to;
  register char *from2 = from;

  while (*from && (len > 0))
  { if (*from == '\033')
    { from++;
      len--;
      if ((*from == 'P') || (*from == 'I') || (*from == 'O') || (*from == 'o'))
      { from++;
        len--;
        if (*from == '[')
        { while ((*from) && (*from != ']'))
          { from++;
            len--;
          }
          if (*from == ']')
          { from++;
            len--;
          }
        }
      }
      else if (*from)
      { from++;
        len--;
      }
    }
    else
    { *to2++ = *from++;
      len--;
    }
  }
  *to2 = '\0';
  return (to2);
}

/*#define DEBUG_ALLOCS*/
#define MINVER 39

/*  ((struct Library *)SysBase)->lib_Version = 37;*/

//$$$Sensei: new memory handling functions.

/* Create new pool, don't care about KickStart version and MorphOS. */
/* Called by OM_NEW. */
APTR	NL_Pool_Create( ULONG puddlesize, ULONG threshsize )
{

	APTR	pool;

	/* Is KickStart at least V39+? */
	if( LIBVER( SysBase ) >= 39 )
		pool	= CreatePool( MEMF_ANY, puddlesize, threshsize );
	else
	{

#ifndef	MORPHOS
		pool	= LibCreatePool( MEMF_ANY, puddlesize, threshsize );
#else
		/* This code probably will never be executed. */
		/* MorphOS requires PPC which mostly is in A1200/A4000 with KS V39+. */
		pool	= NULL;
#endif	/* MORPHOS */

	}

	return( pool );

}

/* Delete pool created by NL_Pool_Create(). */
/* Called by OM_DISPOSE. */
VOID	NL_Pool_Delete( APTR pool )
{

	if( pool )
	{

		/* KickStart is at least V39+? */
		if( LIBVER( SysBase ) >= 39 )
			DeletePool( pool );
		else
#ifndef	MORPHOS
			LibDeletePool( pool );
#endif	/* MORPHOS */

	}

}

/* Low level alloc memory of pool created by NL_Pool_Create(). */
APTR	NL_Pool_Alloc( APTR pool, ULONG size )
{

	if( LIBVER( SysBase ) >= MINVER )
		return( AllocPooled( pool, size ) );
	else
#ifndef	MORPHOS
		return( LibAllocPooled( pool, size ) );
#else
		return( 0 );
#endif

}

/* Low level free memory allocated by NL_Pool_Alloc(). */
VOID	NL_Pool_Free( APTR pool, APTR memory, ULONG size )
{

	if( LIBVER( SysBase ) >= MINVER )
		FreePooled( pool, memory, size );
	else
#ifndef	MORPHOS
		LibFreePooled( pool, memory, size );
#endif	/* MORPHOS */

}

/* Low level alloc memory and remember size. */
APTR	NL_Pool_AllocVec( APTR pool, ULONG size )
{
	ULONG	*memory;
	if( memory = NL_Pool_Alloc( pool, size + sizeof( *memory ) ) )
		*memory++	= size;
	return( memory );
}

/* Low level free memory of unknown size. */
VOID	NL_Pool_FreeVec( APTR pool, APTR memory )
{
	ULONG	*memory1	= (ULONG *) memory - 1;
	if( memory )	NL_Pool_Free( pool, memory1, *memory1 + sizeof( *memory1 ) );
}

/* High level alloc memory. */
/*
 * Memory allocated by this routine will be freed when object is disposing
 * but it should never happened!
 */
APTR	NL_Pool_Internal_Alloc( struct NLData *data, ULONG size )
{
	return( NL_Pool_Alloc( data->Pool, size ) );
}

APTR	NL_Pool_Internal_AllocVec( struct NLData *data, ULONG size )
{
	return( NL_Pool_AllocVec( data->Pool, size ) );
}

/* High level free memory. */
VOID	NL_Pool_Internal_Free( struct NLData *data, APTR memory, ULONG size )
{
	NL_Pool_Free( data->PoolInternal, memory, size );
}

VOID	NL_Pool_Internal_FreeVec( struct NLData *data, APTR memory )
{
	NL_Pool_FreeVec( data->PoolInternal, memory );
}

/* These wrappers slow down, we should use macros or correct functions directly instead... */
APTR	NL2_Malloc2( APTR pool, ULONG size, char *string )
{

	//$$$Sensei: use generic memory handling functions. Don't do the same job multiple times!
	return( NL_Pool_AllocVec( pool, size ) );

}

VOID	NL2_Free2( APTR pool, APTR memory, char * string )
{

	//$$$Sensei: use generic memory handling functions. Don't do the same job multiple times!
	NL_Pool_FreeVec( pool, memory );

}

APTR	NL2_Malloc( struct NLData *data, ULONG size, char *string )
{

	//$$$Sensei: use generic memory handling functions. Don't do the same job multiple times!
	return( NL_Pool_Internal_AllocVec( data, size ) );

}

VOID	NL2_Free( struct NLData *data, APTR memory, char *string )
{

	//$$$Sensei: use generic memory handling functions. Don't do the same job multiple times!
	NL_Pool_Internal_FreeVec( data, memory );

}

/*
 * #define FORMAT_TEMPLATE "DELTA=D/N,PREPARSE=P/K,WEIGHT=W/N,MINWIDTH=MIW/N,MAXWIDTH=MAW/N,COL=C/N,BAR/S\n"
 *
 * struct parse_format
 * {
 *   LONG *delta;
 *   LONG *preparse;
 *   LONG *weight;
 *   LONG *minwidth;
 *   LONG *maxwidth;
 *   LONG *col;
 *   LONG bar;
 * };
 */


#define FORMAT_TEMPLATE "DELTA=D/N,PREPARSE=P/K,COL=C/N,BAR/S,TBAR/S,NOBAR=NB/S,SIMPLEBAR=SBAR/S,"\
                        "NOTITLEBUTTON=NOTB/S,WEIGHT=W/N,MINWIDTH=MIW/N,MAXWIDTH=MAW/N,"\
                        "COLWIDTH=CW/N,MINCOLWIDTH=MICW/N,MAXCOLWIDTH=MACW/N,"\
                        "PIXWIDTH=PW/N,MINPIXWIDTH=MIPW/N,MAXPIXWIDTH=MAPW/N\n"

struct parse_format
{
  LONG *delta;
  LONG *preparse;
  LONG *col;
  LONG bar;
  LONG tbar;
  LONG nobar;
  LONG sbar;
  LONG notb;
  LONG *weight;
  LONG *minwidth;
  LONG *maxwidth;
  LONG *colwidth;
  LONG *mincolwidth;
  LONG *maxcolwidth;
  LONG *pixwidth;
  LONG *minpixwidth;
  LONG *maxpixwidth;
};


void NL_Free_Format(Object *obj,struct NLData *data)
{
  if (data->cols)
  { WORD column = 0;
    while (column < data->numcols)
    { if (data->cols[column].preparse)
        NL_Free(data,data->cols[column].preparse,"FreeFormat_preparse");
      column++;
    }
    NL_Free(data,data->cols,"FreeFormat_cols");
  }
  data->cols = NULL;
  data->numcols = data->numcols2 = 0;
  data->format_chge = 1;
}


BOOL NL_Read_Format(Object *obj,struct NLData *data,char *strformat,BOOL oldlist)
{
  LONG column,colmax,pos1,pos2,col = 0;
  char *sf = NULL;
  struct RDArgs *rdargs,*ptr;
  struct parse_format Line;
  struct colinfo *tmpcols;
  if (strformat && (sf = NL_Malloc(data,strlen(strformat)+4,"ReadFormat_sf")))
  { if (ptr = AllocDosObject(DOS_RDARGS, NULL))
    { colmax = 1;
      pos2 = 0;
      while (strformat[pos2] != '\0')
      { if (strformat[pos2] == ',')
          colmax++;
        pos2++;
      }
      if ((colmax > 0) && (colmax < DISPLAY_ARRAY_MAX) && (tmpcols = NL_Malloc(data,(colmax+1)*sizeof(struct colinfo),"ReadFormat_cols")))
      { NL_Free_Format(obj,data);
        data->cols = tmpcols;
        data->numcols = data->numcols2 = colmax;
        column = 0;
        while (column < colmax)
        { data->cols[column].c = &(data->cols[column]);
          data->cols[column].preparse = NULL;
          data->cols[column].colwidthmax = (WORD) -1;
          data->cols[column].colwidthbiggest = (WORD) -1;
          data->cols[column].colwidthbiggestptr = (LONG) -2;
          data->cols[column].delta = (WORD) 4;
          data->cols[column].col = (WORD) column;
          data->cols[column].userwidth = (WORD) -1;
          data->cols[column].titlebutton = (WORD) TRUE;
          if ((column == colmax-1) && !(oldlist && (column == 0)))
            data->cols[column].width = (WORD) 100;
          else
            data->cols[column].width = (WORD) -1;
          data->cols[column].minwidth = (WORD) 5;
          data->cols[column].maxwidth = (WORD) 0;
          data->cols[column].mincolwidth = (WORD) 0;
          data->cols[column].maxcolwidth = (WORD) 0;
          data->cols[column].minpixwidth = (WORD) 6;
          data->cols[column].maxpixwidth = (WORD) 0;
          data->cols[column].bar = (BYTE) 2;
          data->cols[column].width_type = (BYTE) CI_PERCENT;
          column++;
        }
        column = 0;
        pos2 = 0;
        while (strformat[pos2] && (column < colmax))
        { pos1 = 0;
          while ((strformat[pos2] != ',') && (strformat[pos2] != '\0'))
            sf[pos1++] = strformat[pos2++];
          sf[pos1++] = '\n';
          sf[pos1++] = '\0';
          if (strformat[pos2] != '\0')
            pos2++;
/*D(bug("col %ld ->%s",column,sf));*/

          ptr->RDA_Source.CS_Buffer = sf;
          ptr->RDA_Source.CS_Length = strlen(sf);
          ptr->RDA_Source.CS_CurChr = 0;
          ptr->RDA_DAList = NULL;
          ptr->RDA_Buffer = NULL;
          ptr->RDA_BufSiz = 0L;
          ptr->RDA_ExtHelp = NULL;
          ptr->RDA_Flags = 0L;

          Line.delta = NULL;
          Line.preparse = NULL;
          Line.col = NULL;
          Line.bar = NULL;
          Line.tbar = NULL;
          Line.nobar = NULL;
          Line.sbar = NULL;
          Line.notb = NULL;
          Line.weight = NULL;
          Line.minwidth = NULL;
          Line.maxwidth = NULL;
          Line.colwidth = NULL;
          Line.mincolwidth = NULL;
          Line.maxcolwidth = NULL;
          Line.pixwidth = NULL;
          Line.minpixwidth = NULL;
          Line.maxpixwidth = NULL;

          if (rdargs = ReadArgs(FORMAT_TEMPLATE, (LONG *)&Line, ptr))
          {
            if (Line.delta)    data->cols[column].delta = (WORD) *Line.delta;
            if (Line.preparse)
            { if (data->cols[column].preparse = NL_Malloc(data,strlen((char *)Line.preparse)+2,"ReadFormat_preparse"))
                strcpy(data->cols[column].preparse,(char *)Line.preparse);
            }
            if (Line.col)      data->cols[column].col = (WORD) *Line.col;
            if (Line.tbar)     data->cols[column].bar = (WORD) 2;
            if (Line.bar)      data->cols[column].bar = (WORD) 1;
            if (Line.nobar)    data->cols[column].bar = (WORD) 0;
            if (Line.sbar)     data->cols[column].bar |= (WORD) 4;
            if (Line.notb)     data->cols[column].titlebutton = (WORD) FALSE;

            if (Line.weight)
            {
              data->cols[column].width = (WORD) *Line.weight;
              data->cols[column].width_type = (BYTE) CI_PERCENT;
              if ((column == colmax-1) && !(oldlist && (column == 0)) && (data->cols[column].width == -1))
                data->cols[column].width = (WORD) 100;
              else if (data->cols[column].width < 0)
                data->cols[column].width = (WORD) -1;
            }
            if (Line.colwidth)
            { data->cols[column].width = (WORD) *Line.colwidth;
              data->cols[column].width_type = (BYTE) CI_COL;
              if (data->cols[column].width < 1)
                data->cols[column].width = 1;
            }
            if (Line.pixwidth)
            { data->cols[column].width = (WORD) *Line.pixwidth;
              data->cols[column].width_type = (BYTE) CI_PIX;
              if (data->cols[column].width < 4)
                data->cols[column].width = 4;
            }

            if (Line.minwidth)
              data->cols[column].minwidth = (WORD) *Line.minwidth;
            if (Line.maxwidth)
              data->cols[column].maxwidth = (WORD) *Line.maxwidth;

            if (Line.mincolwidth)
              data->cols[column].mincolwidth = (WORD) *Line.mincolwidth;
            if (Line.maxcolwidth)
              data->cols[column].maxcolwidth = (WORD) *Line.maxcolwidth;

            if (Line.minpixwidth)
              data->cols[column].minpixwidth = (WORD) *Line.minpixwidth;
            if (Line.maxpixwidth)
              data->cols[column].maxpixwidth = (WORD) *Line.maxpixwidth;

            data->cols[column].minx = (WORD) -1;
            data->cols[column].maxx = (WORD) -1;
            data->cols[column].dx = (WORD) 4;
            FreeArgs(rdargs);
          }

          if (data->cols[column].width < 1)
          {
/*
            if ((column == colmax-1) && (data->cols[column].width >= -1))
              data->cols[column].width = (WORD) 100;
            else
*/
              data->cols[column].width = (WORD) -1;
            data->cols[column].width_type = (BYTE) CI_PERCENT;
          }
          if (data->cols[column].delta < 0) data->cols[column].delta = 0;
          if ((data->cols[column].bar != 0) &&
              (data->cols[column].delta < 2)) data->cols[column].delta += 2;
          if (data->cols[column].minwidth < 1)
          {
/*
            if ((column == colmax-1) && (data->cols[column].minwidth >= -1))
              data->cols[column].minwidth = 5;
            else
*/
              data->cols[column].minwidth = -1;
          }
          if (data->cols[column].maxwidth <= 4) data->cols[column].maxwidth = 1000;
          if (data->cols[column].mincolwidth < 0) data->cols[column].mincolwidth = 0;
          if (data->cols[column].maxcolwidth <= 0) data->cols[column].maxcolwidth = 5000;
          if (data->cols[column].minpixwidth < 6) data->cols[column].minpixwidth = 6;
          if (data->cols[column].maxpixwidth <= 5) data->cols[column].maxpixwidth = 20000;
          if (data->cols[column].col > col) col = data->cols[column].col;
          column++;
        }
/*        data->cols[colmax-1].width = (WORD) 100;*/

        FreeDosObject(DOS_RDARGS, ptr);
        NL_Free(data,sf,"ReadFormat_sf");

        col = colmax;
        column = 1;
        while (column < data->numcols)
        { col = 0;
          colmax = 0;
          while (colmax < column)
          { if (data->cols[column].col == data->cols[colmax].col)
            { colmax = 0;
              while (colmax < data->numcols)
              { if (col == data->cols[colmax].col)
                { col++;
                  if (col >= DISPLAY_ARRAY_MAX)
                    col = 0;
                  colmax = 0;
                }
                else
                  colmax++;
              }
              data->cols[column].col = col++;
              if (col >= DISPLAY_ARRAY_MAX)
                col = 0;
              colmax = 0;
            }
            else
              colmax++;
          }
          column++;
        }

        column = 0;
        while (column < data->numcols)
        { data->column[column] = data->cols[column].col;
          data->cols[column].c = &(data->cols[column]);
          column++;
        }
        data->numcols2 = data->numcols;

        data->format_chge = 1;
        data->do_setcols = data->do_updatesb = data->do_wwrap = TRUE;
        data->display_ptr = NULL;
/*D(bug("%lx|NL_Read_Format >%s<\n",obj,strformat));*/
        if (data->SHOW)
        { if (!data->DRAW)
            NL_SetObjInfos(obj,data,TRUE);
          NL_SetCols(obj,data);
        }
        return (TRUE);
      }
      FreeDosObject(DOS_RDARGS, ptr);
    }
    NL_Free(data,sf,"ReadFormat_sf2");
  }
  return (FALSE);
}



static BOOL CCB_string(struct NLData *data,char **cbstr,char *str,LONG len,char lc,BOOL skipesc)
{
  char *tmpcb;
  char *tmp;
  LONG tmpcblen,newlen;

  if (str)
  {
    tmpcblen = len + 2;
    if (*cbstr)
      tmpcblen += strlen(*cbstr);
    if (tmpcb = NL_Malloc(data,tmpcblen,"CCB_string"))
    {
      tmp = tmpcb;
      if (*cbstr)
      {
        tmp = stpcpy(tmp,*cbstr);
        NL_Free(data,*cbstr,"CCB_string");
      }
      if (skipesc)
        tmp = stpncpy_noesc(tmp,str,len);
      else
        tmp = stpncpy(tmp,str,len);
      tmp[0] = lc;
      tmp[1] = '\0';
      *cbstr = tmpcb;
      return (TRUE);
    }
    return (FALSE);
  }
  return (TRUE);
}


static BOOL CCB_entry(Object *obj,struct NLData *data,char **cbstr,APTR entptr,LONG ent,struct Hook *hook,LONG c1,LONG p1,LONG c2,LONG p2)
{
  char **display_array = &data->DisplayArray[2];
  char *str;
  LONG ent1 = ent,pos1,pos2,len,prep1,prep2;
  char lc;
  BOOL ln = FALSE;
  WORD colwrap,wrap = 0;
  if ((ent >= 0) && data->EntriesArray[ent]->Wrap)
  { LONG ent1 = ent;
    if (data->EntriesArray[ent]->Wrap & TE_Wrap_TmpLine)
      ent1 -= data->EntriesArray[ent]->dnum;
    entptr = data->EntriesArray[ent1]->Entry;
    wrap = data->EntriesArray[ent]->Wrap;
  }
  if (p1 < -PREPARSE_OFFSET_ENTRY)   /* begin in general column preparse string */
  { p1 += PREPARSE_OFFSET_COL;
    prep1 = 2;
  }
  else if (p1 < -2)
  { p1 += PREPARSE_OFFSET_ENTRY;     /* begin in display hook column preparse string */
    prep1 = 1;
  }
  else if (p1 == -1)                 /* begin at beginning of column */
    prep1 = 2;
  else
    prep1 = 0;
  if (p2 < -PREPARSE_OFFSET_ENTRY)   /* end in general column preparse string */
  { p2 += PREPARSE_OFFSET_COL;
    prep2 = 2;
  }
  else if (p2 < -2)
  { p2 += PREPARSE_OFFSET_ENTRY;     /* end in display hook column preparse string */
    prep2 = 1;
  }
  else if (p2 == -1)                 /* end at beginning of column */
    prep2 = 2;
  else
    prep2 = 0;

  if (entptr)
  {
    if (!hook)
      hook = data->NList_CopyEntryToClipHook;
    if (hook)
    { display_array[0] = (char *) entptr;
      data->DisplayArray[0] = (char *) data->NList_PrivateData;
      data->DisplayArray[1] = (char *) ent;
      display_array[1] = (char *) c1;
      display_array[2] = (char *) p1;
      display_array[3] = (char *) c2;
      display_array[4] = (char *) p2;
      display_array[5] = (char *) prep1;
      display_array[6] = (char *) prep2;
      if (c1 <= 0) display_array[1] = (char *) -1;
      if (c1 >= data->numcols-1) display_array[1] = (char *) -2;
      if (c2 <= 0) display_array[3] = (char *) -1;
      if (c2 >= data->numcols-1) display_array[3] = (char *) -2;
      if (data->NList_CopyEntryToClipHook2)
      { data->DisplayArray[0] = (char *) entptr;
        MyCallHookPkt(obj,FALSE,hook,obj,data->DisplayArray);
      }
      else
        MyCallHookPkt(obj,TRUE,hook,display_array,entptr);
      data->display_ptr = NULL;
      return (CCB_string(data,cbstr,display_array[0],strlen(display_array[0]),'\0',FALSE));
    }
    else
    {
      WORD column,col,col1,col2;
      NL_GetDisplayArray(obj,data,ent);
      if ((c1 == -2) || (c1 >= data->numcols-1))
        col1 = data->numcols-1;
      else if (c1 < 0)
        col1 = 0;
      else
        col1 = c1;
      if ((c2 == -2) || (c2 >= data->numcols-1))
      { col2 = data->numcols-1;
        if (p2==-2)
          ln = TRUE;
      }
      else if (c2 < 0)
        col2 = 0;
      else
        col2 = c2;
      for (column = col1;column <= col2;column++)
      {
        colwrap = (1 << data->cols[column].c->col) & TE_Wrap_TmpMask;
        str = display_array[data->cols[column].c->col];
        pos1 = -1;
        pos2 = -2;
        if (column == col1)
          pos1 = p1;
        if (column == col2)
          pos2 = p2;
        if (column == col2)
        { if (ln)
            lc = '\n';
          else
            lc = '\0';
        }
        else
          lc = '\t';
        if ((wrap & TE_Wrap_TmpLine) && !(wrap & colwrap))
        { if (!CCB_string(data,cbstr,str,0,lc,TRUE))
            return (FALSE);
          continue;
        }
        if (wrap && (wrap & colwrap))
        { if (pos1 < 0)
            pos1 = data->EntriesArray[ent]->pos;
          if (pos2 < 0)
            pos2 = data->EntriesArray[ent]->pos + data->EntriesArray[ent]->len;
        }
        if (data->NList_CopyColumnToClipHook)
        { display_array[0] = (char *) str;
          data->DisplayArray[0] = (char *) data->NList_PrivateData;
          data->DisplayArray[1] = (char *) ent;
          display_array[1] = (char *) pos1;
          display_array[2] = (char *) pos2;
          if (data->NList_CopyColumnToClipHook2)
          { data->DisplayArray[0] = (char *) str;
            MyCallHookPkt(obj,FALSE,data->NList_CopyColumnToClipHook,obj,data->DisplayArray);
          }
          else
            MyCallHookPkt(obj,TRUE,data->NList_CopyColumnToClipHook,display_array,(APTR) str);
          data->display_ptr = NULL;
          len = (LONG) display_array[1];
          if (len < 0)
            len = 0;
          if (!CCB_string(data,cbstr,display_array[0],len,lc,FALSE))
            return (FALSE);
        }
        else
        { len = 0;
          while ((str[len] != '\0') && (str[len] != '\n'))
            len++;
          if (pos2 > len)
            pos2 = len;
          if (pos1 > len)
          { pos1 = len;
            len = 0;
          }
          if (pos1 <= -2)
            pos1 = len = 0;
          else if (pos1 == -1)
            pos1 = 0;
          if (pos2 >= pos1)
            len = pos2-pos1;
          else if (pos2 == -1)
            len = 0;
          if (!CCB_string(data,cbstr,&str[pos1],len,lc,TRUE /* FALSE */ ))
            return (FALSE);
        }
      }
      return (TRUE);
    }
    return (FALSE);
  }
  return (TRUE);
}


#define CCB_ENTRY_PTR_HOOK(ep,h) \
  ok = CCB_entry(obj,data,&clipstr,ep,-1,h,-1,-1,-2,-2);

#define CCB_ENTRY_PTR(ep) \
  ok = CCB_entry(obj,data,&clipstr,ep,-1,NULL,-1,-1,-2,-2);

#define CCB_ENTRY(e) \
  { if ((e >= 0) && (e < data->NList_Entries)) \
    ok = CCB_entry(obj,data,&clipstr,data->EntriesArray[e]->Entry,e,NULL,-1,-1,-2,-2); \
  }

#define CCB_ENTRY_START_END(e,c1,p1,c2,p2) \
  { if ((e >= 0) && (e < data->NList_Entries)) \
    ok = CCB_entry(obj,data,&clipstr,data->EntriesArray[e]->Entry,e,NULL,c1,p1,c2,p2); \
  }


LONG NL_CopyTo(Object *obj,struct NLData *data,LONG pos,char *filename,ULONG clipnum,APTR *entries,struct Hook *hook)
{
  struct IOClipReq *ior = NULL;
  BPTR file = NULL;
  char *retstr = NULL;
  char emptyline[] = "";
  char retline[] = "\n";
  char *clipstr = NULL;
  LONG ok = TRUE;
  LONG ent;

  if (filename)
  { if (!(file = Open( filename, MODE_NEWFILE )))
      return (MUIV_NLCT_OpenErr);
  }
  else if (clipnum >= 0)
  { if (!(ior = CBOpen(clipnum)))
      return (0);
  }

  if (data->SETUP)
  { set(_app(obj),MUIA_Application_Sleep,TRUE );
  }

  switch (pos)
  {
    case MUIV_NList_CopyToClip_Active :
      CCB_ENTRY(data->NList_Active);
      break;
    case MUIV_NList_CopyToClip_Selected :
      if (!data->NList_TypeSelect)
      { ent = 0;
        while (ok && (ent < data->NList_Entries))
        { if (data->EntriesArray[ent]->Select != TE_Select_None)
          { CCB_ENTRY(ent);
          }
          ent++;
        }
      }
      else
      { LONG c1,p1,c2,p2;
        c1 = data->sel_pt[data->min_sel].column;
        p1 = data->sel_pt[data->min_sel].colpos;
        c2 = data->sel_pt[data->max_sel].column;
        p2 = data->sel_pt[data->max_sel].colpos;
        ent = data->sel_pt[data->min_sel].ent;
        if (ok && (ent >= 0) && (ent < data->NList_Entries))
        { if (ent == data->sel_pt[data->max_sel].ent)
          { CCB_ENTRY_START_END(ent,c1,p1,c2,p2);
            break;
          }
          else
          { CCB_ENTRY_START_END(ent,c1,p1,-2,-2);
            ent++;
          }
        }
        while (ok && (ent >= 0) && (ent < data->sel_pt[data->max_sel].ent) && (ent < data->NList_Entries))
        { CCB_ENTRY(ent);
          ent++;
        }
        if (ok && (ent >= 0) && (ent == data->sel_pt[data->max_sel].ent) && (ent < data->NList_Entries))
        { CCB_ENTRY_START_END(ent,-1,-1,c2,p2);
          ent++;
        }
      }
      break;
    case MUIV_NList_CopyToClip_All :
      { ent = 0;
        while (ok && (ent < data->NList_Entries))
        { CCB_ENTRY(ent);
          ent++;
        }
      }
      break;
    case MUIV_NList_CopyToClip_Entries :
      { APTR *array = (APTR *) entries;
        ent = 0;
        while (ok && array[ent])
        { CCB_ENTRY_PTR(array[ent]);
          ent++;
        }
      }
      break;
    case MUIV_NList_CopyToClip_Entry :
      CCB_ENTRY_PTR(entries);
      break;
    case MUIV_NList_CopyToClip_Strings :
      { APTR *array = (APTR *) entries;
        ent = 0;
        while (ok && array[ent])
        { CCB_ENTRY_PTR_HOOK(array[ent],hook);
          ent++;
        }
      }
      break;
    case MUIV_NList_CopyToClip_String :
      CCB_ENTRY_PTR_HOOK(entries,hook);
      break;
    default :
      CCB_ENTRY(pos);
      break;
  }
  if (file)
  { if (clipstr)
    { LONG pstr = 0;
      LONG lstr = strlen(clipstr);
      LONG ret = 0;
      while ((lstr > 0) && ((ret = Write(file,&clipstr[pstr],lstr)) >= 0))
      { lstr -= ret;
        pstr += ret;
      }
      if (ret < 0)
        ok = MUIV_NLCT_WriteErr;
      else
        ok = MUIV_NLCT_Success;
    }
    else
      ok = MUIV_NLCT_Failed;
    Close(file);
  }
  else if (ior)
  { if (clipstr)
      CBWriteFTXT(ior,clipstr);
    else
      CBWriteFTXT(ior,"");
    CBClose(ior);
  }
  else
  { retstr = (char *) AllocVec(strlen(clipstr)+1,0L);
    if (retstr)
      strcpy(retstr,clipstr);
    ok = (LONG) retstr;
  }
  if (clipstr)
    NL_Free(data,clipstr,"CopyTo");
  if (data->SETUP)
  { set(_app(obj),MUIA_Application_Sleep,FALSE );
  }
  return(ok);
}


ULONG mNL_CopyToClip(struct IClass *cl,Object *obj,struct MUIP_NList_CopyToClip *msg)
{
  struct NLData *data = INST_DATA(cl,obj);
  /*DoSuperMethodA(cl,obj,(Msg) msg);*/
  if (msg->clipnum < 0)
    return (0);
  return ((ULONG) NL_CopyTo(obj,data,msg->pos,NULL,msg->clipnum,msg->entries,msg->hook));
}


ULONG mNL_CopyTo(struct IClass *cl,Object *obj,struct MUIP_NList_CopyTo *msg)
{
  struct NLData *data = INST_DATA(cl,obj);
  LONG res;
  /*DoSuperMethodA(cl,obj,(Msg) msg);*/
  res = NL_CopyTo(obj,data,msg->pos,msg->filename,-1,msg->entries,NULL);
  *msg->result = (APTR) res;
  return ((ULONG)res);
}
