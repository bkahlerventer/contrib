#include <mcc_common.h>

extern struct MUI_CustomClass *NLI2_Class;

#define NImage2Object NewObject(NLI2_Class->mcc_Class,NULL

struct NLI2Data
{
  APTR obj;
  LONG DoDraw;
};


#ifdef MORPHOS
struct EmulLibEntry NLI2_Dispatcher;
#else
extern ULONG ASM SAVEDS NLI2_Dispatcher( REG(a0) struct IClass *cl GNUCREG(a0), REG(a2) Object *obj GNUCREG(a2), REG(a1) Msg msg GNUCREG(a1) );
#endif

extern struct MUI_CustomClass *NLI2_Create(void);
extern void NLI2_Delete(void);

