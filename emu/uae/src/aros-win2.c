 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Amiga interface
  * 
  * Copyright 1996,1997,1998 Samuel Devulder.
  */

/*
 * NOTE:
 *      - Im quite unhappy with the color allocation scheme in case of
 *        EXTRA_HALFBRITE...
 */

/*
 * History: (Day/Month/Year)
 *    long ago - Work begins; many things done; I should have kept the history
 *               log up-to-date.
 *    16/10/96 - Rewrote cybergfx support. (there is still a bug if the memory
 *               is not linear).
 *    15/11/96 - Inhibit the drive in readdevice().
 *    23/11/96 - Added call to rexx_init() and gui_handle_event().
 *    14/12/96 - Added support for graffiti screens.
 *    14/02/97 - Now uses SetRGB32() for cybergfx screens.
 *    15/02/97 - Dump iff file when ENV:UAEIFF is defined.
 *    25/02/97 - Get rid of cybergfx screen direct access.. Now uses
 *               BltBitMapRastPort(). Thanks to Mats Eirik Hansen for
 *               his patch from which I'm widely inspired.
 *    10/05/97 - Use OpenWindowTag() for public screen. Uses Zoom,
 *               SimpleRefresh.
 *    20/08/97 - Removed halfv stuff (use correct_aspect instead).
 *    21/08/97 - Added HAM support (yeah!).
 *    05/10/97 - Added drag&drop support.
 *    13/10/97 - Fixed CyberGFX crash (a missing "return" in flush_line()).
 *    31/10/97 - Added support for gray output (-T). Added missing "Cyb" 
 *               prefix to BitMap=NULL in graphics_leave().
 *    07/12/97 - New calc_err(). UAE screen (-H2) is now a public screen.
 *    25/12/97 - Now use DELTAMOVE for mouse code. Use ENV:UAESM to use
 *               pre-selected screenmode (-H2). Happy Chrismas!
 *    12/02/98 - Now use cybergfx bitmap only for 15+bit screens.
 *    22/02/98 - Added screen-flahses, exit-key and POWERUP code from
 *               Holger Jakob.
 *    04/03/98 - Added do_inhibit_frame() for GUI feedback.
 *    06/03/98 - Added GetRGB4 cache in get_nearest_color() for POWERUP.
 */

#include "sysconfig.h"
#include "sysdeps.h"

/****************************************************************************/

#include <exec/execbase.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>

#include <libraries/asl.h>
#include <intuition/pointerclass.h>

/****************************************************************************/


#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/asl.h>
#include <proto/dos.h>

/****************************************************************************/

#undef Exception  /* Undef Exception because otherwise would conflict with the function
                   * with the same name used in UAE
				   */
/****************************************************************************/


#include <ctype.h>
#include "config.h"
#include "options.h"
#include "threaddep/penguin.h"
#include "uae.h"
#include "memory.h"
#include "custom.h"
#include "newcpu.h"
#include "xwin.h"
#include "drawing.h"
#include "keyboard.h"
#include "keybuf.h"
#include "gui.h"
#include "debug.h"
#include "disk.h"

#define IHF_WINDOWZOOM 6

#define BitMap Picasso96BitMap  /* Argh! */
#include "picasso96.h"
#undef BitMap

/****************************************************************************/

#define use_dither      (!currprefs.no_xhair)
#define use_gray	(0)

#define UAEIFF "UAEIFF"        /* env: var to trigger iff dump */
#define UAESM  "UAESM"         /* env: var for screen mode */

static int need_dither;        /* well.. guess :-) */
static int use_delta_buffer;   /* this will redraw only needed places */
static int use_approx_color;
int dump_iff;

extern xcolnr xcolors[4096];

 /* Keyboard and mouse */

static int keystate[256];

extern int buttonstate[3];
extern int lastmx, lastmy;
extern int newmousecounters;

static int inwindow;

static char *oldpixbuf;

/****************************************************************************/
/*
 * prototypes & global vars
 */
struct IntuitionBase    *IntuitionBase;
struct GfxBase          *GfxBase;
struct Library          *AslBase;
struct Library          *CyberGfxBase;

unsigned long            frame_num; /* for arexx */

static UBYTE            *Line;
static struct RastPort  *RP;
static struct Screen    *S;
static struct Window    *W;
static struct RastPort  *TempRPort;
static struct BitMap    *BitMap;
static struct ColorMap  *CM;
static Object           *Pointer; /* for os 39 */
static UWORD            *Sprite;
static int              XOffset,YOffset;

static int usepub;      /* use public screen */
static int usecyb;      /* use cybergraphics.library */
static int is_halfbrite;
static int is_ham;

static int   get_color_failed;
static int   maxpen;
static UBYTE pen[256];

static void set_title(void);
static LONG ObtainColor(ULONG, ULONG, ULONG);
static void ReleaseColors(void);
static int  DoSizeWindow(struct Window *,int,int);
static void disk_hotkeys(void);
static int  SaveIFF(char *filename, struct Screen *scr);
static int  init_ham(void);
static void ham_conv(UWORD *src, UBYTE *buf, UWORD len);
static int  RPDepth(struct RastPort *RP);

/****************************************************************************/

void main_window_led(int led, int on);

extern char *to_unix_path(char *s);
extern char *from_unix_path(char *s);
extern void split_dir_file(char *src, char **dir, char **file);

extern int ievent_alive;

/****************************************************************************/

extern UBYTE cidx[4][8*4096];

__inline__ void flush_line(int y)
{
    int xs = 0, len;
    int yoffset = y*gfxvidinfo.rowbytes;
    char *linebuf = gfxvidinfo.bufmem + yoffset;
    char *src, *dst;

    if(y<0 || y>=gfxvidinfo.height) {
//       printf("flush_line out of window: %d\n", y); */
       return;
    }

    len = gfxvidinfo.width;

    if(is_ham) {
        ham_conv((void*)linebuf,Line,len);
        WritePixelLine8(RP, 0, y, len, Line, TempRPort);
        return;
    }

    if(!use_delta_buffer) {
      dst = linebuf;
    } else switch(gfxvidinfo.pixbytes) {
      case 4:
        {   /* sam: we should not arrive here on the amiga */
            fprintf(stderr, "Bug in flush_line() !\n");
            fprintf(stderr, "need_dither=%d\n",need_dither);
            fprintf(stderr, "depth=%d\n",RPDepth(RP));
            fprintf(stderr, "Please return those values to maintainer.\n");
            abort();
            return;
        } break;
      case 2:
        {   short *newp = (short *)linebuf;
            short *oldp = (short *)(oldpixbuf + yoffset);
            while (*newp++ == *oldp++) if(!--len) return;
            src   = (char *)--newp;
            dst   = (char *)--oldp;
            newp += len;
            oldp += len;
            while (*--newp == *--oldp);
            len   = 1 + (oldp - (short *)dst);
            xs    = (src - linebuf)/2;
            CopyMem (src, dst, len * 2);
        } break;
      case 1:
        {   char *newp = (char *)linebuf;
            char *oldp = (char *)(oldpixbuf + yoffset);
            while (*newp++ == *oldp++) if(!--len) return;
            src   = (char *)--newp;
            dst   = (char *)--oldp;
            newp += len;
            oldp += len;
            while (*--newp == *--oldp);
            len   = 1 + (oldp - (char *)dst);
            xs    = (src - linebuf);
            CopyMem (src, dst, len);
         } break;
      default:
        abort();
        break;
      }

    if (need_dither) {
        DitherLine(Line, (UWORD *)dst, xs, y, (len+3)&~3, 8);
    } else CopyMem(dst, Line, len);

    WritePixelLine8(RP, xs + XOffset, y + YOffset, len, Line, TempRPort);
}

/****************************************************************************/

void flush_block (int ystart, int ystop)
{
    int y;
    for(y=ystart; y<=ystop; ++y) flush_line(y);
}

/****************************************************************************/

static void save_frame(void)
{
    char *file;
    static int cpt = 0;
    char name[80];

    if(!dump_iff) return;
    if(!(file = getenv(UAEIFF))) return;
    if(strchr(file,'%')) sprintf(name,file,cpt++);
    else sprintf(name,"%s.%05d",file,cpt++);
    if(W->WScreen) SaveIFF(name,W->WScreen);
}

/****************************************************************************/

void flush_screen (int ystart, int ystop)
{
/* WaitBOVP() ? */
}

/****************************************************************************/

static int RPDepth(struct RastPort *RP)
{
   int depth = RP->BitMap->Depth;
   if (depth>8) depth = 8;

   return depth;
}

/****************************************************************************/

static int get_color(int r, int g, int b, xcolnr *cnp)
{
    int col;

    if(use_gray) r = g = b = (77*r + 151*g + 29*b) / 16;
    else {r *= 0x11;g *= 0x11;b *= 0x11;}

    r *= 0x01010101;
    g *= 0x01010101;
    b *= 0x01010101;
    col = ObtainColor(r, g, b);

    if(col == -1) {
        get_color_failed = 1;
        return 0;
    }

    *cnp = col;
    return 1;
}

/****************************************************************************/
/*
 * FIXME: find a better way to determine closeness of colors (closer to
 * human perception).
 */
static __inline__ void rgb2xyz(int r, int g, int b,
                               int *x, int *y, int *z)
{
	*x = r*1024 - (g+b)*512;
	*y = 886*(g-b);
	*z = (r+g+b)*341;
}
static __inline__ int calc_err(int r1, int g1, int b1,
                               int r2, int g2, int b2)
{
    int x1,y1,z1,x2,y2,z2;

    rgb2xyz(r1,g1,b1,&x1,&y1,&z1);
    rgb2xyz(r2,g2,b2,&x2,&y2,&z2);
    x1 -= x2; y1 -= y2; z1 -= z2;
    return x1*x1 + y1*y1 + z1*z1;
}

/****************************************************************************/

static int get_nearest_color(int r, int g, int b)
{
    int i, best, err, besterr;
    int colors;
    int br=0,bg=0,bb=0;

   if(use_gray) r = g = b = (77*r + 151*g + 29*b) / 256;

    best    = 0;
    besterr = calc_err(0,0,0, 15,15,15);
    colors  = is_halfbrite?32:(1<<RPDepth(RP));
  

    for(i=0; i<colors; i++ ) {
        long rgb;
        int cr, cg, cb;

        rgb = GetRGB4(CM, i);
        cr = (rgb >> 8) & 15;
        cg = (rgb >> 4) & 15;
        cb = (rgb >> 0) & 15;

        err = calc_err(r,g,b, cr,cg,cb);

        if(err < besterr) {
            best = i;
            besterr = err;
            br=cr; bg=cg; bb=cb;
        }

        if(is_halfbrite) {
            cr /= 2; cg /= 2; cb /= 2;
            err = calc_err(r,g,b, cr,cg,cb);
            if(err < besterr) {
                best = i + 32;
                besterr = err;
                br=cr; bg=cg; bb=cb;
            }
        }
    }
    return best;
}

/****************************************************************************/

static int init_colors(void)
{
    gfxvidinfo.can_double = 0;
    if (need_dither) {
        /* first try color allocation */
        int bitdepth = usepub ? 8 : RPDepth(RP);
        int maxcol;

        if(!use_gray && bitdepth>=3)
        do {
            get_color_failed = 0;
            setup_dither(bitdepth, get_color);
            if(get_color_failed) ReleaseColors();
        } while(get_color_failed && --bitdepth>=3);
        
        if(!use_gray && bitdepth>=3) {
            printf("Color dithering with %d bits\n",bitdepth);
            return 1;
        }

        /* if that fail then try grey allocation */
        maxcol = 1<<(usepub ? 8 : RPDepth(RP));

        do {
            get_color_failed = 0;
            setup_greydither_maxcol(maxcol, get_color);
            if(get_color_failed) ReleaseColors();
        } while(get_color_failed && --maxcol>=2);

        /* extra pass with approximated colors */
        if(get_color_failed) do {
            maxcol=2;
            use_approx_color = 1;
            get_color_failed = 0;
            setup_greydither_maxcol(maxcol, get_color);
            if(get_color_failed) ReleaseColors();
        } while(get_color_failed && --maxcol>=2);

        if (maxcol >= 2) {
            printf("Gray dither with %d shades.\n",maxcol);
            return 1;
        }

        return 0; /* everything failed :-( */
    }

    /* No dither */
    switch(RPDepth(RP)) {
      case 6: if (is_halfbrite) {
        static int tab[]={
        0x000, 0x00f, 0x0f0, 0x0ff, 0x08f, 0x0f8, 0xf00, 0xf0f,
        0x80f, 0xff0, 0xfff, 0x88f, 0x8f0, 0x8f8, 0x8ff, 0xf08,
        0xf80, 0xf88, 0xf8f, 0xff8, /* end of regular pattern */
        0xa00, 0x0a0, 0xaa0, 0x00a, 0xa0a, 0x0aa, 0xaaa,
        0xfaa, 0xf6a, 0xa80, 0x06a, 0x6af };
        int i;
        for(i=0;i<32;++i) get_color(tab[i]>>8, (tab[i]>>4)&15, tab[i]&15, xcolors);
        for(i=0;i<4096;++i) xcolors[i] = get_nearest_color(i>>8, (i>>4)&15, i&15);
        printf("Using %d colors + halfbrite\n",32);
        break;
      } else if(is_ham) {
        int i;
        for(i=0;i<16;++i) get_color(i,i,i, xcolors);
        printf("Using %d bits pseudo-truecolor (HAM).\n",12);
        alloc_colors64k(4,4,4,10,5,0);
        return init_ham();
      }

      case 1: case 2: case 3: case 4: case 5: case 7: case 8: {
        int maxcol = 1<<RPDepth(RP);

        if(maxcol>=8 && !use_gray) do {
            get_color_failed = 0;
            setup_maxcol(maxcol);
            alloc_colors256(get_color);
            if(get_color_failed) ReleaseColors();
        } while(get_color_failed && --maxcol>=8);
        else {
            int i;
            for(i=0;i<maxcol;++i) {
                get_color((i*15)/(maxcol-1), (i*15)/(maxcol-1), 
                          (i*15)/(maxcol-1), xcolors);
            }
        }
        printf("Using %d colors.\n",maxcol);
        for(maxcol=0; maxcol<4096; ++maxcol)
            xcolors[maxcol] = get_nearest_color(maxcol>>8, (maxcol>>4)&15, 
                                                maxcol&15);
        } break;

      case 15:
        printf("Using %d bits truecolor.\n",15);
        alloc_colors64k(5,5,5,10,5,0);
        break;

      case 16:
        printf("Using %d bits truecolor.\n",16);
        alloc_colors64k(5,6,5,11,5,0);
        break;

      case 24: 
        printf("Using %d bits truecolor.\n",24);
        alloc_colors64k(8,8,8,16,8,0);
        break;

      case 32:
        printf("Using %d bits truecolor.\n",32);
        alloc_colors64k(8,8,8,16,8,0);
        break;
    }
    return 1;
}

/****************************************************************************/

static void setup_sprite(struct Window *W, int visible)
{
    Sprite = AllocVec((4+2)*sizeof(*Sprite), MEMF_CHIP|MEMF_CLEAR);
    if(!Sprite) {
       fprintf(stderr, "Warning: Can't alloc sprite buffer !\n");
       return;
    }
    if(visible) {Sprite[2] = 0x8000; Sprite[3] = 0x8000;}
    SetPointer(W, Sprite, 1, 16, -1, 0);
}

/****************************************************************************/

static int setup_customscreen(void)
{
    /* hum some of old programming style here :) */
    static struct NewScreen NewScreenStructure = {
        0,0, 800,600, 3, 0,1,
        LACE+HIRES, CUSTOMSCREEN|SCREENQUIET|SCREENBEHIND,
        NULL, (void*)"UAE", NULL, NULL};
    static struct NewWindow NewWindowStructure = {
        0,0, 800,600, 0,1,
        IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_DISKINSERTED|IDCMP_DISKREMOVED|
        IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_MOUSEMOVE|
        IDCMP_DELTAMOVE,
        WFLG_SMART_REFRESH|WFLG_BACKDROP|WFLG_RMBTRAP|WFLG_NOCAREREFRESH|
        WFLG_BORDERLESS|WFLG_ACTIVATE|WFLG_REPORTMOUSE,
        NULL, NULL, (void*)"UAE", NULL, NULL, 5,5, 800,600,
        CUSTOMSCREEN};

    NewScreenStructure.Width     = currprefs.gfx_width;
    NewScreenStructure.Height    = currprefs.gfx_height;
    NewScreenStructure.Depth     = 8;
    NewScreenStructure.ViewModes = SPRITES | (currprefs.gfx_lores?NULL:HIRES) |
                                   (currprefs.gfx_height>256?LACE:NULL);
  
    do S = (void*)OpenScreen(&NewScreenStructure);
    while(!S && --NewScreenStructure.Depth);
    if(!S) {
        fprintf(stderr, "Can't open custom screen !\n");
        return 0;
    }

    CM = S->ViewPort.ColorMap;
    RP = &S->RastPort;

    NewWindowStructure.Width  = S->Width;
    NewWindowStructure.Height = S->Height;
    NewWindowStructure.Screen = S;
    W = (void*)OpenWindow(&NewWindowStructure);
    if(!W) {
        fprintf(stderr, "Can't open window on custom screen !\n");
        return 0;
    }
    setup_sprite(W,0);
    return 1;
}

/****************************************************************************/

static int setup_publicscreen(void)
{
    UWORD ZoomArray[4] = {0,0,0,0};

    S = LockPubScreen(NULL);
    if(!S) {
        fprintf(stderr,"No public screen !\n");
        return 0;
    }

    ZoomArray[2] = 128;
    ZoomArray[3] = S->BarHeight+1;

    CM = S->ViewPort.ColorMap;

    if((S->ViewPort.Modes & (HIRES|LACE))==HIRES) {
        if(currprefs.gfx_height + S->BarHeight + 1 >= S->Height) {
            currprefs.gfx_height >>= 1;
            currprefs.gfx_correct_aspect = 1;
        }
    }

    W = OpenWindowTags(NULL,
                       WA_Title,        (ULONG)"UAE",
                       WA_AutoAdjust,   TRUE,
                       WA_InnerWidth,   currprefs.gfx_width,
                       WA_InnerHeight,  currprefs.gfx_height,
                       WA_PubScreen,    (ULONG)S,
                       WA_Zoom,         (ULONG)ZoomArray,
                       WA_IDCMP,        IDCMP_MOUSEBUTTONS|
                                        IDCMP_RAWKEY|
                                        IDCMP_ACTIVEWINDOW|
                                        IDCMP_INACTIVEWINDOW|
                                        IDCMP_MOUSEMOVE|
                                        IDCMP_DELTAMOVE|
                                        IDCMP_CLOSEWINDOW|
                                        IDCMP_REFRESHWINDOW|
                                        IDCMP_NEWSIZE|
                                        0,
                       WA_Flags,        WFLG_DRAGBAR|
                                        WFLG_DEPTHGADGET|
                                        WFLG_REPORTMOUSE|
                                        WFLG_RMBTRAP|
                                        WFLG_ACTIVATE|
                                        WFLG_CLOSEGADGET|
                                        WFLG_SMART_REFRESH|
                                        0,
                       TAG_DONE);

    UnlockPubScreen(NULL, S);

    if(!W) {
        fprintf(stderr,"Can't open window on public screen !\n");
        CM = NULL;
        return 0;
    }

    gfxvidinfo.width  = (W->Width  - W->BorderRight - W->BorderLeft);
    gfxvidinfo.height = (W->Height - W->BorderTop   - W->BorderBottom);
    XOffset = W->BorderLeft;
    YOffset = W->BorderTop;

    RP = W->RPort;
    setup_sprite(W,1);

    return 1;
}

/****************************************************************************/

static char *get_num(char *s, int *n)
{
   int i=0;
   while(isspace(*s)) ++s;
   if(*s=='0') {
     ++s;
     if(*s=='x' || *s=='X') {
       do {char c=*++s; 
           if(c>='0' && c<='9') {i*=16; i+= c-'0';}    else
           if(c>='a' && c<='f') {i*=16; i+= c-'a'+10;} else
           if(c>='A' && c<='F') {i*=16; i+= c-'A'+10;} else break;
       } while(1);
     } else while(*s>='0' && *s<='7') {i*=8; i+= *s++ - '0';}
   } else {
     while(*s>='0' && *s<='9') {i*=10; i+= *s++ - '0';}
   }
   *n=i;
   while(isspace(*s)) ++s;
   return s;
}

/****************************************************************************/

static void get_displayid(ULONG *DI, LONG *DE)
{
   char *s;
   int di,de;

   *DI=INVALID_ID;
   s=getenv(UAESM);if(!s) return;
   s=get_num(s,&di);
   if(*s!=':') return;
   s=get_num(s+1,&de);
   if(!de) return;
   *DI=di; *DE=de;
}

/****************************************************************************/

static int setup_userscreen(void)
{
    struct ScreenModeRequester *ScreenRequest;
    ULONG DisplayID;
    static struct BitMap PointerBitMap;
    UWORD *PointerLine;
    LONG ScreenWidth=0,ScreenHeight=0,Depth=0;
    UWORD OverscanType=OSCAN_STANDARD;
    BOOL AutoScroll=TRUE;

    if(!AslBase) AslBase = OpenLibrary("asl.library",38);
    if(!AslBase) {
        fprintf(stderr,"Can't open asl.library v38 !");
        return 0;
    }

    ScreenRequest = AllocAslRequest(ASL_ScreenModeRequest,NULL);
    if(!ScreenRequest) {
        fprintf(stderr,"Unable to allocate screen mode requester.\n");
        return 0;
    }

    get_displayid(&DisplayID, &Depth);

    if(DisplayID==(ULONG)INVALID_ID) {
    if(AslRequestTags(ScreenRequest,
                      ASLSM_TitleText, (ULONG)"Select screen display mode",
                      ASLSM_InitialDisplayID,    NULL,
                      ASLSM_InitialDisplayDepth, 8,
                      ASLSM_InitialDisplayWidth, currprefs.gfx_width,
                      ASLSM_InitialDisplayHeight,currprefs.gfx_height,
                      ASLSM_MinWidth,            currprefs.gfx_width,
                      ASLSM_MinHeight,           currprefs.gfx_height,
                      ASLSM_DoWidth,             TRUE,
                      ASLSM_DoHeight,            TRUE,
                      ASLSM_DoDepth,             TRUE,
                      ASLSM_DoOverscanType,      TRUE,
                      ASLSM_PropertyFlags,       0,
                      ASLSM_PropertyMask,        DIPF_IS_DUALPF | 
                                                 DIPF_IS_PF2PRI | 
                                                 0,
                      TAG_DONE)) {
        ScreenWidth  = ScreenRequest->sm_DisplayWidth;
        ScreenHeight = ScreenRequest->sm_DisplayHeight;
        Depth        = ScreenRequest->sm_DisplayDepth;
        DisplayID    = ScreenRequest->sm_DisplayID;
        OverscanType = ScreenRequest->sm_OverscanType;
        AutoScroll   = ScreenRequest->sm_AutoScroll;
        }
    else DisplayID = INVALID_ID;
    }
    FreeAslRequest(ScreenRequest);
    
    if(DisplayID == (ULONG)INVALID_ID) return 0;

    if(DisplayID & HAM_KEY) Depth = 6; /* only ham6 for the moment */
    if(ScreenWidth  < currprefs.gfx_width)  ScreenWidth  = currprefs.gfx_width;
    if(ScreenHeight < currprefs.gfx_height) ScreenHeight = currprefs.gfx_height;

    S = OpenScreenTags(NULL,
                       SA_DisplayID,                  DisplayID,
                       SA_Width,                      ScreenWidth,
                       SA_Height,                     ScreenHeight,
                       SA_Depth,                      Depth,
                       SA_Overscan,                   OverscanType,
                       SA_AutoScroll,                 AutoScroll,
                       SA_ShowTitle,                  FALSE,
                       SA_Quiet,                      TRUE,
                       SA_Behind,                     TRUE,
                       SA_PubName, 		      (ULONG)"UAE",
                       /* v39 stuff here: */
                       SA_BackFill,                   (ULONG)LAYERS_NOBACKFILL,
                       SA_SharePens,                  TRUE,
                       SA_Exclusive,                  FALSE,
                       SA_Draggable,                  TRUE,
                       SA_Interleaved,                TRUE,
                       TAG_DONE);
    if(!S) {
        fprintf(stderr,"Unable to open the screen.\n");
        return 0;
    }
    
    CM           = S->ViewPort.ColorMap;
    is_halfbrite = (S->ViewPort.Modes & EXTRA_HALFBRITE);
    is_ham       = (S->ViewPort.Modes & HAM);

    PointerLine  = malloc(4);/* autodocs says it needs not be in chip memory */
    if(PointerLine) PointerLine[0]=PointerLine[1]=0;
    InitBitMap(&PointerBitMap,2,16,1);
    PointerBitMap.Planes[0] = (PLANEPTR)&PointerLine[0];
    PointerBitMap.Planes[1] = (PLANEPTR)&PointerLine[1];

    Pointer = NewObject(NULL,POINTERCLASS,
                        POINTERA_BitMap,        (ULONG)&PointerBitMap,
                        POINTERA_WordWidth,     1,
                        TAG_DONE);
    if(!Pointer)
        fprintf(stderr,"Warning: Unable to allocate blank mouse pointer.\n");

    W = OpenWindowTags(NULL,
                       WA_Width,                S->Width,
                       WA_Height,               S->Height,
                       WA_CustomScreen,         (ULONG)S,
                       WA_Backdrop,             TRUE,
                       WA_Borderless,           TRUE,
                       WA_RMBTrap,              TRUE,
                       WA_Activate,		TRUE,
                       WA_ReportMouse,          TRUE,
                       WA_IDCMP,                IDCMP_MOUSEBUTTONS|
                                                IDCMP_RAWKEY|
                                                IDCMP_DISKINSERTED|
                                                IDCMP_DISKREMOVED|
                                                IDCMP_ACTIVEWINDOW|
                                                IDCMP_INACTIVEWINDOW|
                                                IDCMP_MOUSEMOVE|
                                                IDCMP_DELTAMOVE,
                       WA_BackFill,             (ULONG)LAYERS_NOBACKFILL,
                       (Pointer?WA_Pointer:TAG_IGNORE), (ULONG)Pointer,
                       TAG_DONE);

    if(!W) {
        fprintf(stderr,"Unable to open the window.\n");
        CloseScreen(S);S=NULL;RP=NULL;CM=NULL;
        return 0;
    }
    if(!Pointer) setup_sprite(W,0);

    RP = W->RPort; /* &S->Rastport if screen is not public */
    PubScreenStatus(S, 0);
    printf("Using screenmode: 0x%x:%d (%u:%d)\n",
           DisplayID,Depth, DisplayID,Depth);

    return 1;
}

/****************************************************************************/

int graphics_setup(void)
{
#ifdef OS_IS_AMIGAOS
    if(ix_os != OS_IS_AMIGAOS) {
        ix_req(NULL, "Abort", NULL, "That version of %s is only for AmigaOS!", __progname);
        exit(20);
    }
#endif
    if(((struct ExecBase *)SysBase)->LibNode.lib_Version < 36) {
        fprintf(stderr, "UAE needs OS 2.0+ !\n");
        return 0;
    }

//    atexit(graphics_leave);

    IntuitionBase = (void*)OpenLibrary("intuition.library",0L);
    if(!IntuitionBase) {
        fprintf(stderr,"No intuition.library ?\n");
        return 0;
    }
    GfxBase = (void*)OpenLibrary("graphics.library",0L);
    if(!GfxBase) {
        fprintf(stderr,"No graphics.library ?\n");
        return 0;
    }

    return 1;
}

/****************************************************************************/

int graphics_init(void)
{
    int i,bitdepth;

    use_delta_buffer = 0;
    need_dither = 0;
    
    if (currprefs.gfx_width < 320)
        currprefs.gfx_width = 320;
    if (!currprefs.gfx_correct_aspect && (currprefs.gfx_height < 64/*200*/))
        currprefs.gfx_height = 200;
    currprefs.gfx_width += 7;
    currprefs.gfx_width &= ~7;

    if (currprefs.color_mode > 5)
        fprintf(stderr, "Bad color mode selected. Using default.\n"), currprefs.color_mode = 0;
    
    gfxvidinfo.width  = currprefs.gfx_width;
    gfxvidinfo.height = currprefs.gfx_height;

	  /*
    switch(currprefs.color_mode) {
      case 2:
        if(setup_userscreen()) break;
        fprintf(stderr,"Trying on public screen...\n");
      case 1:
        is_halfbrite = 0;
        if(setup_publicscreen()) {usepub = 1;break;}
        fprintf(stderr,"Trying on custom screen...\n");
      case 0:
      default:
        if(!setup_customscreen()) return 0;
        break;
    }
	*/
      fprintf(stderr,"Trying on public screen...\n");
      is_halfbrite = 0;
      if(setup_publicscreen()) usepub = 1;
      else return 0;

    Line = AllocVec((currprefs.gfx_width + 15) & ~15,MEMF_ANY|MEMF_PUBLIC);
    if(!Line) {
        fprintf(stderr,"Unable to allocate raster buffer.\n");
        return 0;
    }
    BitMap = AllocBitMap(currprefs.gfx_width,1,8,BMF_CLEAR|BMF_MINPLANES,RP->BitMap);
    if(!BitMap) {
        fprintf(stderr,"Unable to allocate BitMap.\n");
        return 0;
    }
    TempRPort = AllocVec(sizeof(struct RastPort),MEMF_ANY|MEMF_PUBLIC);
    if(!TempRPort) {
        fprintf(stderr,"Unable to allocate RastPort.\n");
        return 0;
    }
    CopyMem(RP,TempRPort,sizeof(struct RastPort));
    TempRPort->Layer  = NULL;
    TempRPort->BitMap = BitMap;

    if(usepub) set_title();

    bitdepth = RPDepth(RP);
    if(is_ham) {
        /* ham 6 */
        use_delta_buffer    = 0; /* needless as the line must be fully */
        need_dither         = 0; /* recomputed */
        gfxvidinfo.pixbytes = 2;
    } else if(bitdepth <= 8) {
        /* chunk2planar is slow so we define use_delta_buffer for all modes */
        use_delta_buffer    = 1; 
        need_dither         = use_dither || (bitdepth<=1);
        gfxvidinfo.pixbytes = need_dither?2:1;
    } else {
        /* Cybergfx mode */
        use_delta_buffer    = 0;
        need_dither         = 0;
        gfxvidinfo.pixbytes = (bitdepth >= 24) ? 4 : (bitdepth >= 12) ? 2 : 1;
    }

    gfxvidinfo.emergmem = 0;
    gfxvidinfo.linemem = 0;

    gfxvidinfo.rowbytes = gfxvidinfo.pixbytes * currprefs.gfx_width;
    gfxvidinfo.bufmem = (char *)calloc(gfxvidinfo.rowbytes, currprefs.gfx_height+1);
        /*                                                           ^^ */
        /*            This is because DitherLine may read one extra row */

    if(!gfxvidinfo.bufmem) {
        fprintf(stderr,"Not enough memory for video bufmem.\n");
        return 0;
    } 

    if (use_delta_buffer) {
        gfxvidinfo.maxblocklines = currprefs.gfx_height-1; /* it seems to increase the speed */
        oldpixbuf = (char *)calloc(gfxvidinfo.rowbytes, currprefs.gfx_height);
        if(!oldpixbuf) {
            fprintf(stderr,"Not enough memory for oldpixbuf.\n");
            return 0;
        }
    } else {
        gfxvidinfo.maxblocklines = 0; /* whatever... */
    }

    if (!init_colors()) {
        fprintf(stderr,"Failed to init colors.\n");
        return 0;
    }
    switch (gfxvidinfo.pixbytes) {
     case 2:
        for (i = 0; i < 4096; i++) xcolors[i] *= 0x00010001;
        gfxvidinfo.can_double = 1;
        break;
     case 1:
        for (i = 0; i < 4096; i++) xcolors[i] *= 0x01010101;
        gfxvidinfo.can_double = 1;
        break;
     default:
        gfxvidinfo.can_double = 0;
        break;
    }

    if(!usepub) ScreenToFront(S);

    buttonstate[0] = buttonstate[1] = buttonstate[2] = 0;
    for(i=0; i<256; i++)
        keystate[i] = 0;
    
    lastmx = lastmy = 0; 
    newmousecounters = 0;
    inwindow = 0;

    if(getenv(UAEIFF)) {
        dump_iff = 1;
        fprintf(stderr,"Saving to \"%s\"\n",getenv(UAEIFF));
    }

    return 1;
}

/****************************************************************************/

int check_prefs_changed_gfx (void)
{
	return 0;
}

/****************************************************************************/

void graphics_leave(void)
{
    if(BitMap) {
        WaitBlit();
        FreeBitMap(BitMap);
        BitMap = NULL;
    }
    if(TempRPort) {
        FreeVec(TempRPort);
        TempRPort = NULL;
    }
    if(Line) {
        FreeVec(Line);
        Line = NULL;
    } 
    if(CM) {
        ReleaseColors();
        CM = NULL;
    }
    if(W) {
        CloseWindow(W);
        W = NULL;
    }
    if(Sprite) {
        FreeVec(Sprite);
        Sprite = NULL;
    }
    if(Pointer) {
        DisposeObject(Pointer);
        Pointer = NULL;
    }
    if(!usepub && S) {
        if(!CloseScreen(S)) {
           fprintf(stderr,"Please close all opened windows on UAE's "
		          "screen.\n");
	   do Delay(50); while(!CloseScreen(S));
	}
        S = NULL;
    }
    if(AslBase) {
        CloseLibrary((void*)AslBase);
        AslBase = NULL;
    }
    if(GfxBase) {
        CloseLibrary((void*)GfxBase);
        GfxBase = NULL;
    }
    if(IntuitionBase) {
        CloseLibrary((void*)IntuitionBase);
        IntuitionBase = NULL;
    }
    if(CyberGfxBase) {
        CloseLibrary((void*)CyberGfxBase);
        CyberGfxBase = NULL;
    }
}

/****************************************************************************/

void handle_events(void)
{
    struct IntuiMessage *msg;
    int dmx,dmy,class,code;

    newmousecounters = 0;

   /* this function is called at each frame, so: */
    ++frame_num;       /* increase frame counter */
    //save_frame();      /* possibly save frame    */

    while((msg=(struct IntuiMessage*)GetMsg(W->UserPort))) {
        class = msg->Class;
        code  = msg->Code;
        dmx   = msg->MouseX;
        dmy   = msg->MouseY;
        ReplyMsg((struct Message*)msg);

        switch(class) {
          case IDCMP_NEWSIZE:
            toggle_inhibit_frame((W->Flags & WFLG_ZOOMED)?IHF_WINDOWZOOM:0);
            break;

          case IDCMP_REFRESHWINDOW:
            if(use_delta_buffer) {
                /* hack: this forces refresh */
                char *ptr = oldpixbuf;
                int i, len = gfxvidinfo.width;
                len *= gfxvidinfo.pixbytes;
                for(i=0;i<currprefs.gfx_height;++i) {
                    ptr[00000] ^= 255;
                    ptr[len-1] ^= 255;
                    ptr += gfxvidinfo.rowbytes;
                }
            }
            BeginRefresh(W);
            flush_block(0,currprefs.gfx_height-1);
            EndRefresh(W, TRUE);
	    break;

          case IDCMP_CLOSEWINDOW:
            activate_debugger();
            break;

          case IDCMP_RAWKEY: {
              int kc       = code&127;
              int released = code&128?1:0;

              if(released) {
                  keystate[kc] = 0;
                  record_key ((kc << 1) | 1);
              } else if (!keystate[kc])  {
                  keystate[kc] = 1;
                  record_key (kc << 1);
              }
          } break;

          case IDCMP_MOUSEMOVE:
            lastmx += dmx;
            lastmy += dmy;
	    break;

          case IDCMP_MOUSEBUTTONS:
            if(code==SELECTDOWN) buttonstate[0]=1;
            if(code==SELECTUP)   buttonstate[0]=0;
            if(code==MIDDLEDOWN) buttonstate[2]=1;
            if(code==MIDDLEUP)   buttonstate[2]=0;
            if(code==MENUDOWN)   buttonstate[2]=1;
            if(code==MENUUP)     buttonstate[2]=0;
            break;
        
          /* Those 2 could be of some use later. */
          case IDCMP_DISKINSERTED:
            /*printf("diskinserted(%d)\n",code);*/
            break;
          
          case IDCMP_DISKREMOVED:
            /*printf("diskremoved(%d)\n",code);*/
            break;
                        
          case IDCMP_ACTIVEWINDOW:
            inwindow = 1;
            newmousecounters = 1;
            break;

          case IDCMP_INACTIVEWINDOW:
            inwindow = 0;
            break;
          
          default:
            fprintf(stderr, "Unknown class: %d\n",class);
            break;
        }
    }
    /* "Affengriff" */
    if(keystate[AK_CTRL] && keystate[AK_LAMI] && keystate[AK_RAMI]) 
        m68k_reset();

    /* PC-like :-) CTRL-ALT-DEL => reboot */
    if(keystate[AK_CTRL] && (keystate[AK_LALT] || keystate[AK_RALT]) && 
       keystate[AK_DEL]) 
        m68k_reset();

    /* CTRL+LSHIFT+LALT+F10 on amiga => F12 on X11 */
    /*                  F9           => ScrollLock on X11 (inhibit_frame) */
    if(keystate[AK_CTRL] && keystate[AK_LSH] && keystate[AK_LALT]) {
/*        if(keystate[AK_F10]) togglemouse(); */
        if(keystate[AK_F8]) {
            uae_quit();
        }
	/*
	if(keystate[AK_F9]) {
            do_inhibit_frame(inhibit_frame != 2 ? inhibit_frame ^ 1 :
                            inhibit_frame);
            DisplayBeep(NULL);
        } */
    }

    disk_hotkeys();
    gui_handle_events();
}

/***************************************************************************/

int debuggable(void)
{
    return 1;
}

/***************************************************************************/

int needmousehack(void)
{
    return 0;
}

/***************************************************************************/

void LED(int on)
{
}

/***************************************************************************/

/* sam: need to put all this in a separate module */

#ifdef PICASSO96

void DX_Invalidate (int first, int last)
{
}

int DX_BitsPerCannon (void)
{
    return 8;
}

void DX_SetPalette(int start, int count)
{
}

int DX_FillResolutions (uae_u16 *ppixel_format)
{
    return 0;
}

void gfx_set_picasso_modeinfo (int w, int h, int depth)
{
}

void gfx_set_picasso_baseaddr (uaecptr a)
{
}

void gfx_set_picasso_state (int on)
{
}

void begindrawing (void)
{
}

void enddrawing (void)
{
}
#endif


int lockscr (void)
{
return 1;
}

void unlockscr (void)
{
}


/***************************************************************************/

static int led_state[5];

static void set_title(void)
{
    static char title[80];
    static char ScreenTitle[200];

    if(!usepub) return;

    sprintf(title,"%sPower: [%c] Drives: [%c] [%c] [%c] [%c]",
            inhibit_frame?"UAE (PAUSED) - ":"UAE � ",
            led_state[0]?'X':' ',
            led_state[1]?'0':' ',
            led_state[2]?'1':' ',
            led_state[3]?'2':' ',
            led_state[4]?'3':' ');
    
    if(!*ScreenTitle) {
         sprintf(ScreenTitle,
                 "UAE-%d.%d.%d (%s%s%s) � by Bernd Schmidt & contributors, "
                 "Amiga Port by Samuel Devulder.",
		  UAEMAJOR, UAEMINOR, UAESUBREV,
		  currprefs.cpu_level==0?"68000":
		  currprefs.cpu_level==1?"68010":
		  currprefs.cpu_level==2?"68020":"68020/68881",
		  currprefs.address_space_24?" 24bits":"",
		  currprefs.cpu_compatible?" compat":"");
        SetWindowTitles(W, title, ScreenTitle);
    } else SetWindowTitles(W, title, (char*)-1);
}

/****************************************************************************/

void main_window_led(int led, int on)                /* is used in amigui.c */
{
  if(led>=0 && led<=4) led_state[led] = on;
  set_title();
}

/****************************************************************************/

static void unrecord(int kc)
{
    keystate[kc] = 0;
    record_key ((kc << 1) | 1);
}

/****************************************************************************/

static void disk_hotkeys(void)
{
    struct FileRequester *FileRequest;
    char buff[80];
    int drive;
    char *last_file,*last_dir,*s;

    if(!(keystate[AK_CTRL] && keystate[AK_LALT])) return;

    /* CTRL-LSHIFT-LALT F1-F4 => eject_disk */
    if(keystate[AK_LSH]) {
        int ok = 0;

        if(keystate[AK_F1]) {ok=1;disk_eject(0);
                             printf("drive DF0: ejected\n");}
        if(keystate[AK_F2]) {ok=1;disk_eject(1);
                             printf("drive DF1: ejected\n");}
        if(keystate[AK_F3]) {ok=1;disk_eject(2);
                             printf("drive DF2: ejected\n");}
        if(keystate[AK_F4]) {ok=1;disk_eject(3);
                             printf("drive DF3: ejected\n");}

        if(ok) {
            DisplayBeep(NULL);
            unrecord(AK_CTRL);unrecord(AK_LALT);unrecord(AK_LSH); 
            unrecord(AK_F1);unrecord(AK_F2);  
            unrecord(AK_F3);unrecord(AK_F4);
        }
        return;
    }

    /* CTRL-LALT F1-F4 => insert_disk */
    if(keystate[AK_F1])      {drive = 0;unrecord(AK_F1);}
    else if(keystate[AK_F2]) {drive = 1;unrecord(AK_F2);}
    else if(keystate[AK_F3]) {drive = 2;unrecord(AK_F3);}
    else if(keystate[AK_F4]) {drive = 3;unrecord(AK_F4);}
    else return;
    DisplayBeep(NULL);
    unrecord(AK_CTRL);unrecord(AK_LALT); 

    switch(drive) {
      case 0: case 1: case 2: case 3: last_file = currprefs.df[drive]; break;
      default: return;
    }

    split_dir_file(from_unix_path(last_file), &last_dir, &last_file);
    if(!last_file) return;
    if(!last_dir)  return;

    if(!AslBase) AslBase = OpenLibrary("asl.library",36);
    if(!AslBase) {
        fprintf(stderr,"Can't open asl.library v36 !");
        return;
    }

    FileRequest = AllocAslRequest(ASL_FileRequest,NULL);
    if(!FileRequest) {
        fprintf(stderr,"Unable to allocate file requester.\n");
        return;
    }

    sprintf(buff,"Select file to use for drive DF%d:",drive);
    if(AslRequestTags(FileRequest,
                      ASLFR_Window,         (ULONG)W,
                      ASLFR_TitleText,      (ULONG)buff,
                      ASLFR_InitialDrawer,  (ULONG)last_dir,
                      ASLFR_InitialFile,    (ULONG)last_file,
                      ASLFR_InitialPattern, (ULONG)"(#?.ad(f|z)#?|df?|?)",
                      ASLFR_DoPatterns,     TRUE,
                      ASLFR_RejectIcons,    TRUE,
                      TAG_DONE)) {
        free(last_file);
        last_file = malloc(3 + strlen(FileRequest->fr_Drawer) +
                           strlen(FileRequest->fr_File));
        if((last_file)) {
            s = last_file;
            strcpy(s,FileRequest->fr_Drawer);
            if(*s && !(s[strlen(s)-1]==':' || s[strlen(s)-1]=='/')) 
                strcat(s,"/");
            strcat(s,FileRequest->fr_File);
            last_file = to_unix_path(s);free(s);
        }
    } else {
        free(last_file);
        last_file = NULL;
    }
    FreeAslRequest(FileRequest);
    free(last_dir);

    if(last_file) {
        disk_insert(drive,last_file);
        free(last_file);
    }
}

/****************************************************************************/
/*
 * find the best appropriate color. return -1 if none is available
 */
static LONG ObtainColor(ULONG r,ULONG g,ULONG b)
{
    int i, crgb;
    int colors;

    if(usepub && CM) {
        i = ObtainBestPen(CM,r,g,b,
                          OBP_Precision, (use_approx_color?PRECISION_GUI:
                                                           PRECISION_EXACT),
                          OBP_FailIfBad, TRUE,
                          TAG_DONE);
        if(i != -1) {
            if(maxpen<256) pen[maxpen++] = i;
            else i = -1;
        }
        return i;
    }

    colors = is_halfbrite?32:(1<<RPDepth(RP));

    /* private screen => standard allocation */
    if(!usepub) {
        if(maxpen >= colors) return -1; /* no more colors available */
        SetRGB32(&S->ViewPort, maxpen, r, g, b);
	return maxpen++;
    }

    /* public => find exact match */
    r >>= 28; g >>= 28; b >>= 28;
    if(use_approx_color) return get_nearest_color(r,g,b);
    crgb = (r<<8)|(g<<4)|b;
    for(i=0; i<colors; i++ ) {
        int rgb = GetRGB4(CM, i);
        if(rgb == crgb) return i;
    }
    return -1;
}

/****************************************************************************/
/*
 * free a color entry
 */
static void ReleaseColors(void)
{
    if(usepub && CM)
        while(maxpen>0) ReleasePen(CM, pen[--maxpen]);
    else maxpen = 0;
}

/****************************************************************************/

static int DoSizeWindow(struct Window *W,int wi,int he)
{
    register int x,y;
    int ret = 1;

    wi += W->BorderRight + W->BorderLeft;
    he += W->BorderBottom + W->BorderTop;
    x   = W->LeftEdge;
    y   = W->TopEdge;

    if(x + wi >= W->WScreen->Width)  x = W->WScreen->Width - wi;
    if(y + he >= W->WScreen->Height) y = W->WScreen->Height - he;

    if(x<0 || y<0) {
        fprintf(stderr, "Working screen too small to open window (%dx%d)!\n",
                wi, he);
        if(x<0) {x = 0; wi = W->WScreen->Width;}
        if(y<0) {y = 0; he = W->WScreen->Height;}
        ret = 0;
    }

    x  -= W->LeftEdge;
    y  -= W->TopEdge;
    wi -= W->Width;
    he -= W->Height;

    if(x|y)   MoveWindow(W,x,y);
    if(wi|he) SizeWindow(W,wi,he);
    return ret;
}


/****************************************************************************/

#define MAKEID(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|d)

static int SaveIFF(char *filename, struct Screen *scr)
{
    struct DisplayInfo DI;
    FILE *file;
    ULONG BODYsize;
    ULONG modeid;
    ULONG count;
    ULONG i;
    
    struct {ULONG iff_type, iff_length;} chunk;
    struct {ULONG fc_type, fc_length, fc_subtype;} FORM;
    struct {
        UWORD w,h,x,y;
        UBYTE depth,masking,compression,pad1;
        UWORD transparentColor;
        UBYTE xAspect,yAspect;
        WORD  pagewidth,pageheight;
    } BMHD;
    
    BODYsize = scr->BitMap.Depth * scr->BitMap.Rows * 2 * ((scr->Width+15)/16);
    modeid   = GetVPModeID(&S->ViewPort);
    count    = scr->ViewPort.ColorMap->Count;
    
    FORM.fc_type    = MAKEID('F','O','R','M');
    FORM.fc_length  = 4 +                 /* ILBM */
                      8 + sizeof(BMHD) +  /* BMHD */
                      8 + 4 +             /* CAMG */
                      8 + 3*count +       /* CMAP */
                      8 + BODYsize;       /* BODY */
    FORM.fc_subtype = MAKEID('I','L','B','M');
    
    if(!(file = fopen(filename,"w"))) return 0;
    if(fwrite(&FORM,sizeof(FORM),1,file)!=1) goto err;

    BMHD.w           = 
    BMHD.pagewidth   = scr->Width;
    BMHD.h           = 
    BMHD.pageheight  = scr->Height;
    BMHD.x           = 0;
    BMHD.y           = 0;
    BMHD.depth       = scr->BitMap.Depth;
    BMHD.masking     = 0;
    BMHD.compression = 0;
    BMHD.pad1        = 0;
    BMHD.transparentColor = 0;
    BMHD.xAspect     = 22;
    BMHD.yAspect     = 11;

    if(GetDisplayInfoData(NULL, (UBYTE *)&DI, sizeof(struct DisplayInfo), 
                          DTAG_DISP, modeid)) {
    BMHD.xAspect     = DI.Resolution.x;
    BMHD.yAspect     = DI.Resolution.y;
    }

    chunk.iff_type   = MAKEID('B','M','H','D');
    chunk.iff_length = sizeof(BMHD);
    if(fwrite(&chunk,sizeof(chunk),1,file)!=1 
    || fwrite(&BMHD, sizeof(BMHD), 1,file)!=1) goto err;

    chunk.iff_type   = MAKEID('C','A','M','G');
    chunk.iff_length = sizeof(modeid);
    if(fwrite(&chunk, sizeof(chunk),   1,file)!=1
    || fwrite(&modeid,chunk.iff_length,1,file)!=1) goto err;

   chunk.iff_type    = MAKEID('C','M','A','P');
   chunk.iff_length  = 3 * count;
   if(fwrite(&chunk,sizeof(chunk),1,file)!=1) goto err;
   for(i=0; i<count; ++i) {
      ULONG c = GetRGB4(scr->ViewPort.ColorMap, i);
      UBYTE d;
      d = (c>>8)&15;d |= d<<4;if(fwrite(&d,1,1,file)!=1) goto err;
      d = (c>>4)&15;d |= d<<4;if(fwrite(&d,1,1,file)!=1) goto err;
      d = (c>>0)&15;d |= d<<4;if(fwrite(&d,1,1,file)!=1) goto err;
   }

   chunk.iff_type    = MAKEID('B','O','D','Y');
   chunk.iff_length  = BODYsize;
   if(fwrite(&chunk,sizeof(chunk),1,file)!=1) goto err;
   {
   int r,p;
   struct BitMap *bm = S->RastPort.BitMap;
   for(r=0; r<bm->Rows; ++r) for(p=0; p<bm->Depth; ++p)
   if(fwrite(bm->Planes[p] + r*bm->BytesPerRow, 2*((S->Width+15)/16), 1, file)!=1) goto err;
   }
   
   fclose(file);
   return 1;
err: 
   fprintf(stderr,"Error writing to \"%s\"\n",filename);
   fclose(file);
   return 0;
   }

/****************************************************************************/
/* Here lies an algorithm to convert a 12bits truecolor buffer into a HAM
 * buffer. That algorithm is quite fast and if you study it closely, you'll
 * understand why there is no need for MMX cpu to subtract three numbers in
 * the same time. I can think of a quicker algorithm but it'll need 4096*4096
 * = 1<<24 = 16Mb of memory. That's why I'm quite proud of this one which
 * only need roughly 64Kb (could be reduced down to 40Kb, but it's not
 * worth as I use cidx as a buffer which is 128Kb long)..
 ****************************************************************************/

static int dist4(LONG rgb1, LONG rgb2) /* computes distance very quickly */
{
    int d = 0, t;
    t = (rgb1&0xF00)-(rgb2&0xF00); t>>=8; if (t<0) d -= t; else d += t;
    t = (rgb1&0x0F0)-(rgb2&0x0F0); t>>=4; if (t<0) d -= t; else d += t;
    t = (rgb1&0x00F)-(rgb2&0x00F); t>>=0; if (t<0) d -= t; else d += t;
#if 0
    t = rgb1^rgb2; 
    if(t&15) ++d; t>>=4;
    if(t&15) ++d; t>>=4;
    if(t&15) ++d;
#endif
    return d;
}

#define d_dst (00000+(UBYTE*)cidx) /* let's use cidx as a buffer */
#define d_cmd (16384+(UBYTE*)cidx)
#define h_buf (32768+(UBYTE*)cidx)

static int init_ham(void)
{
    int i,t,RGB;

    /* try direct color first */
    for(RGB=0;RGB<4096;++RGB) {
        int c,d;
        c = d = 50;
        for(i=0;i<16;++i) {
            t = dist4(i*0x111, RGB);
            if(t<d) {
                d = t;
                c = i;
            }
        }
        i = (RGB&0x00F) | ((RGB&0x0F0)<<1) | ((RGB&0xF00)<<2);
        d_dst[i] = (d<<2)|3; /* the "|3" is a trick to speedup comparison */
        d_cmd[i] = c;        /* in the conversion process */
    }
    /* then hold & modify */
    for(i=0;i<32768;++i) {
        int dr, dg, db, d, c;
        dr = (i>>10) & 0x1F; dr -= 0x10;if(dr<0) dr = -dr;
        dg = (i>>5)  & 0x1F; dg -= 0x10;if(dg<0) dg = -dg;
        db = (i>>0)  & 0x1F; db -= 0x10;if(db<0) db = -db;
        c  = 0; d = 50;
        t = dist4(0,  0*256 + dg*16 + db); if(t < d) {d = t;c = 0;}
        t = dist4(0, dr*256 +  0*16 + db); if(t < d) {d = t;c = 1;}
        t = dist4(0, dr*256 + dg*16 +  0); if(t < d) {d = t;c = 2;}
        h_buf[i] = (d<<2) | c;
    }
    return 1;
}

/* great algorithm: convert trucolor into ham using precalc buffers */
#undef USE_BITFIELDS
static void ham_conv(UWORD *src, UBYTE *buf, UWORD len)
{
    /* A good compiler (ie. gcc :) will use bfext/bfins instructions */
    union { struct { ULONG _:17,r:5,g:5,b:5;} _; ULONG all;} rgb, RGB;

    rgb.all = 0;
    while(len--) {
        UBYTE c,t;
        RGB.all = *src++;
        c = d_cmd[RGB.all];
        /* cowabonga! */
        t = h_buf[16912 + RGB.all - rgb.all];
#ifndef USE_BITFIELDS
        if(t<=d_dst[RGB.all]) {
	    static int ht[]={32+10,48+5,16+0}; ULONG m;
	    t &= 3; m = 0x1F<<(ht[t]&15);
            m = ~m; rgb.all &= m;
            m = ~m; m &= RGB.all;rgb.all |= m;
	    m >>= ht[t]&15;
	    c = (ht[t]&~15) | m;
        } else {
	    rgb.all = c;
	    rgb.all <<= 5; rgb.all |= c;
	    rgb.all <<= 5; rgb.all |= c;
        }
#else
        if(t<=d_dst[RGB.all]) {
            t&=3;
            if(!t)        {c = 32; c |= (rgb._.r = RGB._.r);}
            else {--t; if(!t) {c = 48; c |= (rgb._.g = RGB._.g);}
            else              {c = 16; c |= (rgb._.b = RGB._.b);} }
        } else rgb._.r = rgb._.g = rgb._.b = c;
#endif
        *buf++ = c;
    }
}

/****************************************************************************/
/* support routines to handle unix filename convention
 */
char *to_unix_path(char *s)
{
    char *t,*r,*u;
    int l;

#ifndef __GNUC
    return my_strdup(s);
#endif

    for(u=s,l=0;*u;++u,++l) if(*u=='/' || *u==':') l+=2;

    r = t = malloc(1+l);
    if(!r) return NULL;

    for(u=s;*u && *u!=':';++u);
    if(*u) {
        *t++='/';
        while(*s!=':') *t++=*s++;
        *t++='/';++s;
    }
    while(*s=='/') {*t++='.';*t++='.';*t++=*s++;}
    while(*s) {
        if(s[0]=='/' && s[1]=='/') {*t++=*s++;*t++='.';*t++='.';*t++=*s++;}
        else *t++=*s++;
    }
    *t='\0';
    return r;
}

/****************************************************************************/

char *from_unix_path(char *s)
{
    char *t,*r;

    r = t = malloc(strlen(s)+1);
    if(!r) return NULL;
#ifndef __GNUC__
    strcpy(r,s);
    return r;
#endif

    if(*s=='/') {
        ++s;
        while(*s && *s!='/') *t++=*s++;
        if(*s=='/') {*t++=':';++s;}
    }

    while(*s) {
        if(s[0]=='.' && s[1]=='.') s+=2;
        else *t++=*s++;
    }

    *t='\0';

    return r;
}

/****************************************************************************/

void split_dir_file(char *src, char **dir, char **file)
{   /* note: src is freed() */
    char *s=src;

    while(*s) ++s;
    while(s>src && (*s!=':' && *s!='/')) --s;
    if(*s==':' || *s=='/') {
        *file = my_strdup(s+1);
        s[1]  = '\0';
        *dir  = my_strdup(src);
        free(src);
    } else {
        *file = src;
        *dir  = my_strdup("");
    }
}


/****************************************************************************/

void target_save_options (FILE *f, struct uae_prefs *p)
{
}

int target_parse_option (struct uae_prefs *p, char *option, char *value)
{
	return 0;
}

void setup_brkhandler(void)
{}
