
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>
#include <aros/machine.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


/***********************************************************************************/

struct IntuitionBase 	*IntuitionBase;
struct GfxBase 		*GfxBase;
struct Library 		*CyberGfxBase;
struct Screen 		*scr;
struct Window 		*win;
struct RastPort 	*rp;
ULONG 			cgfx_coltab[256];
UBYTE 			Keys[128];
char			s[256];

static void cleanup(char *msg);

/***********************************************************************************/

#define balls 5

static WORD x, y, i;

static struct
{
    APTR p;
    WORD dia;
    BYTE rand;
} ball[balls];

#define W 320
#define H 200

static UBYTE chunkybuffer[W * H];

/***********************************************************************************/

static void refresh(void)
{
    WriteLUTPixelArray(chunkybuffer,
		       0,
		       0,
		       W,
		       rp,
		       cgfx_coltab,
		       win->BorderLeft,
		       win->BorderTop,
		       W,
		       H,
		       CTABFMT_XRGB8);
}

/***********************************************************************************/

#define SQR(x) ((x) * (x))
#define pi 3.14159265

static void Calc_Ball(WORD hohe, WORD breite, APTR po)
{
    WORD x, y, col;
    float e;
    
    for(y = 0; y < hohe; y++)
    {
    	for(x = 0; x < breite; x++)
	{
	    e = sqrt(SQR(x-(breite+1)/2) + 1.5*SQR(y-(breite+1)/2));
	    col = (WORD)(130.0+130.0*cos(e*pi/(hohe/2)));
	    if (col > 255) col=255;
	    if (e < (hohe / 2))
	    {
	    	((UBYTE *)po)[breite * y + x] = col;
	    }
	    else
	    {
	    	((UBYTE *)po)[breite * y + x] = 0;
	    }
	}
    }
    
}

/***********************************************************************************/

static void Do_Ball(WORD x, WORD y, WORD h, WORD b, APTR po)
{
    WORD loopy, loopx;
    UWORD destoffset = (y * W) + x;
    UWORD col;
    
#define src ((UBYTE *)po)
    
    for(loopy = 0; loopy < h; loopy++)
    {
    	for(loopx = 0; loopx < b; loopx++)
	{
	    col = *src++;
	    col += chunkybuffer[destoffset];
	    if (col > 255) col = 255;
	    chunkybuffer[destoffset++] = col;
	}
	destoffset += (W - b);
    }
}

/***********************************************************************************/

static void initpalette(void)
{
    WORD palindex = 0;
    
    for(palindex = 0; palindex < 128; palindex++)
    {
    	ULONG red   = 0;
	ULONG green = palindex * 2;
	ULONG blue  = 0;

	cgfx_coltab[palindex] = (red << 16) + (green << 8) + blue;

    	red = palindex * 2;
	green = 255;
	blue = palindex * 2;

	cgfx_coltab[palindex + 128] = (red << 16) + (green << 8) + blue;	
    }
    
    cgfx_coltab[255] = 0xFFFFFF;    
}

/***********************************************************************************/

static void initstuff(void)
{    
    for(i = 1; i <= balls; i++)
    {
    	ball[i - 1].dia = 20 + abs(70 - 10 * (i-1));
	ball[i - 1].rand = (BYTE)(20.0+20.0*sin(i*pi/balls*13));
	ball[i - 1].p = malloc(SQR(ball[i - 1].dia));
	if (ball[i].p) cleanup("out of memory!");
	Calc_Ball(ball[i-1].dia, ball[i-1].dia, ball[i-1].p);
    }
}

/***********************************************************************************/

static void ticker(void)
{
    memset(chunkybuffer, 0, sizeof(chunkybuffer));
    
    for(i = 0; i < balls; i++)
    {
    	Do_Ball((WORD)((160 - ball[i].dia / 2) +
	    	sin(x*pi/400+i*6)*(90-8*i)*sin((x+ball[i].rand)*pi/150.0) +
		(20+6*i)*cos((x-ball[i].rand+940.0)*pi/150.0)),
		(WORD)((100 - ball[i].dia / 2) +
		cos(x*pi/400+i*6)*(20+6*i)*cos((x+ball[i].rand)*pi/120.0) +
		(20+4*i)*sin((x-ball[i].rand+1000.0)*pi/120.0)),
		ball[i].dia, ball[i].dia, ball[i].p);    
    }
    x++;
}

/***********************************************************************************/

static void cleanup(char *msg)
{
    
    if (msg)
    {
        printf("metaballs: %s\n",msg);
    }
    
    if (win) CloseWindow(win);
    
    if (scr) UnlockPubScreen(0, scr);
    
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    
    exit(0);
}

/***********************************************************************************/

static void openlibs(void)
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

static void getvisual(void)
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

static void makewin(void)
{
    win = OpenWindowTags(NULL, WA_CustomScreen	, (IPTR)scr,
    			       WA_Left		, (scr->Width - W - scr->WBorLeft - scr->WBorRight) / 2,
			       WA_Top		, (scr->Height - H - scr->WBorTop - scr->WBorTop - scr->Font->ta_YSize - 1) / 2,
    			       WA_InnerWidth	, W,
    			       WA_InnerHeight	, H,
			       WA_AutoAdjust	, TRUE,
			       WA_Title		, (IPTR)"Metaballs",
			       WA_DragBar	, TRUE,
			       WA_DepthGadget	, TRUE,
			       WA_CloseGadget	, TRUE,
			       WA_Activate	, TRUE,
			       WA_IDCMP		, IDCMP_CLOSEWINDOW |
			       			  IDCMP_RAWKEY,
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

static void getevents(void)
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

	}
        ReplyMsg((struct Message *)msg);
    }

}

/***********************************************************************************/

static void action(void)
{
    initpalette();
    initstuff();
    
    while (!Keys[KC_ESC])
    {
        getevents();

    	ticker();
        refresh();
    	WaitTOF();
    }

}

/***********************************************************************************/

int main(void)
{
    openlibs();
    getvisual();
    makewin();
    action();
    cleanup(0);
    
    return 0;
}

/***********************************************************************************/

