#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define SCREENWIDTH  320
#define SCREENHEIGHT 200

/***********************************************************************************/

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *CyberGfxBase;
struct Screen *scr;
struct Window *win;
struct RastPort *rp;

ULONG cgfx_coltab[256];
BYTE Keys[256];

/***********************************************************************************/

UWORD pal=2;
UWORD a,c,d,x,y,z;
UWORD s[256];
UBYTE p[65536];
UBYTE buf[SCREENWIDTH * SCREENHEIGHT];

/***********************************************************************************/

void cleanup(char *msg)
{
    if (msg)
    {
        printf("parallax: %s\n",msg);
    }
    
    if (win) CloseWindow(win);
    
    if (scr) UnlockPubScreen(0, scr);
    
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    
    exit(0);
}

/***********************************************************************************/

void openlibs(void)
{
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)))
    {
        cleanup("Can't open intuition.library V39!");
    }
    
    if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39)))
    {
        cleanup("Can't open graphics.library V39!");
    }
    
    if (!(CyberGfxBase = OpenLibrary("cybergraphics.library",0)))
    {
        cleanup("Can't open cybergraphics.library!");
    }
}

/***********************************************************************************/

void getvisual(void)
{
    if (!(scr = LockPubScreen(NULL)))
    {
        cleanup("Can't lock pub screen!");
    }
    
    if (GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH) <= 8)
    {
        cleanup("Need hi or true color screen!");
    }
}

/***********************************************************************************/

void setuppal(WORD pal)
{
    WORD a;
    
    for(a = 0; a < 256; a ++)
    {
    	cgfx_coltab[a] = a << (pal * 8);
    }
}

/***********************************************************************************/

void setupdemo(void)
{
    setuppal(pal);
    
    for(a = 0; a < 256; a++)
    {
    	s[a] = (WORD)(32 - sin(3.14159265 * a / 128) * 31);
    }
    
    d = 200;
}

/***********************************************************************************/

void render(void)
{
    static int firsttime = 1;
    static int oldmousex, oldmousey;
    
    if (firsttime)
    {
    	firsttime = 0;
    }
    else
    {
    	c += (win->MouseX - oldmousex);
	d += (win->MouseY - oldmousey);
	
	oldmousex = win->MouseX;
	oldmousey = win->MouseY;
    }
    a = 0;
    c -= 2;
    
    for(y = 0; y < SCREENHEIGHT; y++)
    {
    	for(x = 0; x < SCREENWIDTH; x++)
	{
	    buf[a++] = p[((UBYTE)((y - 100) & 255)) * 256 + ((UBYTE)(x & 255))];
	}
    }

#if 1

    for(y = 0; y < 128; y++)
    {
    	for(x = 0; x < 128; x++)
	{
	    p[y * 256 + x] = p[y * 256 * 2 + x * 2] / 2 + 40;
	}
    }

#endif

#if 1    
    for(y = 0; y < 128; y++)
    {
    	memmove(p + y * 256 + 128, p + y * 256, 128);
    }
    
    memmove(p + 128 * 256, p, 32768);
#endif
    
    for(y = 0; y < 256; y++)
    {
    	for(x = 0; x < 256; x++)
	{
	    a = (UBYTE)((s[x] ^ s[y] & s[(UBYTE)((x ^ y) & 255)]) & 255);
	    if (a > 37) p[((UBYTE)((d + y) & 255)) * 256 + (UBYTE)((c + x) & 255)] = a * 8 - 30;
	}
    }
    
    WriteLUTPixelArray(buf,
		       0,
		       0,
		       SCREENWIDTH,
		       rp,
		       cgfx_coltab,
		       win->BorderLeft,
		       win->BorderTop,
		       SCREENWIDTH,
		       SCREENHEIGHT,
		       CTABFMT_XRGB8);
    
}

/***********************************************************************************/

void makewin(void)
{
    win = OpenWindowTags(NULL, WA_CustomScreen, (IPTR)scr, 
    			       WA_InnerWidth, SCREENWIDTH,
    			       WA_InnerHeight, SCREENHEIGHT,
			       WA_Title, (IPTR)"Parallax",
			       WA_DragBar, TRUE,
			       WA_DepthGadget, TRUE,
			       WA_CloseGadget, TRUE,
			       WA_Activate, TRUE,
			       WA_IDCMP, IDCMP_CLOSEWINDOW |
			       		 IDCMP_RAWKEY      |
					 IDCMP_MOUSEBUTTONS,
			       TAG_DONE);
			       
   if (!win) cleanup("Can't open window");

   rp = win->RPort; 
}

/***********************************************************************************/

#define KC_LEFT         0x4F
#define KC_RIGHT     	0x4E
#define KC_UP        	0x4C
#define KC_DOWN      	0x4D
#define KC_ESC       	0x45

/***********************************************************************************/

void getevents(void)
{
    struct IntuiMessage *msg;
    
    while ((msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
    {
        switch(msg->Class)
	{
	    case IDCMP_CLOSEWINDOW:
	        Keys[KC_ESC] = 1;
		break;
		
	    case IDCMP_RAWKEY:
	        {
		    WORD code = msg->Code & ~IECODE_UP_PREFIX;
		    
		    Keys[code] = (code == msg->Code) ? 1 : 0;

		}
	        break;

    	    case IDCMP_MOUSEBUTTONS:
	    	if (msg->Code == SELECTDOWN)
		{
	    	    pal = (pal + 1) % 3;
		    setuppal(pal);
		}
	    	break;
	}
        ReplyMsg((struct Message *)msg);
    }

}

/***********************************************************************************/

main(int argc, char *argv[])
{
    BOOL done = FALSE;
    
    openlibs();
    getvisual();
    setupdemo(); 
    makewin();

    while(!done)
    {
	getevents();
	render();	
	if (Keys[KC_ESC])
	{
	    done = TRUE;
	}
    }

    cleanup(0);
}

/***********************************************************************************/

