/**
 * Scout - The Amiga System Monitor
 *
 *------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * You must not use this source code to gain profit of any kind!
 *
 *------------------------------------------------------------------
 *
 * @author Andreas Gelhausen
 * @author Richard K�rber <rkoerber@gmx.de>
 */

#include "system_headers.h"

struct MemoryDetailWinData {
    char mdwd_Title[WINDOW_TITLE_LENGTH];
    APTR mdwd_Texts[12];
    APTR mdwd_ChunkList;
    struct MemoryEntry *mdwd_Memory;
    APTR mdwd_MemoryPool;
};

STATIC CONST struct LongFlag memFlags[] = {
    { MEMF_PUBLIC,     "MEMF_PUBLIC" },
    { MEMF_CHIP,       "MEMF_CHIP" },
    { MEMF_FAST,       "MEMF_FAST" },
#if defined(__amigaos4__)
    { MEMF_VIRTUAL,    "MEMF_VIRTUAL" },
    { MEMF_EXECUTABLE, "MEMF_EXECUTABLE" },
#else
    { (1 << 3),        "< ??? >" },
    { (1 << 4),        "< ??? >" },
#endif
    { (1 << 5),        "< ??? >" },
    { (1 << 6),        "< ??? >" },
    { (1 << 7),        "< ??? >" },
    { MEMF_LOCAL,      "MEMF_LOCAL" },
    { MEMF_24BITDMA,   "MEMF_24BITDMA" },
    { MEMF_KICK,       "MEMF_KICK" },
#if defined(__amigaos4__)
    { MEMF_PRIVATE,    "MEMF_PRIVATE" },
    { MEMF_SHARED,     "MEMF_SHARED" },
#elif defined(__MORPHOS__)
    { (1 << 11),       "MEMF_SWAP" },
    { (1 << 12),       "MEMF_31BIT" }, 
#else
    { (1 << 11),       "< ??? >" },
    { (1 << 12),       "< ??? >" },
#endif
    { (1 << 13),       "< ??? >" },
    { (1 << 14),       "< ??? >" },
    { (1 << 15),       "< ??? >" },
    { MEMF_CLEAR,      "MEMF_CLEAR" },
    { MEMF_LARGEST,    "MEMF_LARGEST" },
    { MEMF_REVERSE,    "MEMF_REVERSE" },
    { MEMF_TOTAL,      "MEMF_TOTAL" },
#if defined(__amigaos4__)
    { MEMF_HWALIGNED,  "MEMF_HWALIGNED" },
    { MEMF_DELAYED,    "MEMF_DELAYED" },
#else
    { (1 << 20),       "< ??? >" },
    { (1 << 21),       "< ??? >" },
#endif
    { (1 << 22),       "< ??? >" },
    { (1 << 23),       "< ??? >" },
    { (1 << 24),       "< ??? >" },
    { (1 << 25),       "< ??? >" },
    { (1 << 26),       "< ??? >" },
    { (1 << 27),       "< ??? >" },
    { (1 << 28),       "< ??? >" },
    { (1 << 29),       "< ??? >" },
    { (1 << 30),       "< ??? >" },
#if defined(__amigaos4__)
    { MEMF_NO_EXPUNGE, "MEMF_NO_EXPUNGE" },
#else
    { (1 << 31),       "< ??? >" },
#endif
    { MEMF_ANY,        "MEMF_ANY" },
    { 0,               NULL }
};

STATIC SAVEDS LONG memmorelist_dsp2func( struct Hook *hook, Object *obj, struct NList_DisplayMessage *msg )
{
    struct MemoryFreeEntry *mfe = (struct MemoryFreeEntry *)msg->entry;

    if (mfe) {
        msg->strings[0] = mfe->mfe_Lower;
        msg->strings[1] = mfe->mfe_Upper;
        msg->strings[2] = mfe->mfe_Size;
    } else {
        msg->strings[0] = txtMemoryLower3;
        msg->strings[1] = txtMemoryUpper3;
        msg->strings[2] = txtMemorySize;
        msg->preparses[0] = MUIX_B;
        msg->preparses[1] = MUIX_B;
        msg->preparses[2] = MUIX_B;
    }

    return 0;
}
MakeStaticHook(memmorelist_dsp2hook, memmorelist_dsp2func);

STATIC LONG memmorelist_cmp2colfunc( struct MemoryFreeEntry *mfe1,
                                     struct MemoryFreeEntry *mfe2,
                                     ULONG column )
{
    switch (column) {
        default:
        case 0: return HexCompare(mfe1->mfe_Lower, mfe2->mfe_Lower);
        case 1: return HexCompare(mfe1->mfe_Upper, mfe2->mfe_Upper);
        case 2: return -stricmp(mfe1->mfe_Size, mfe2->mfe_Size);
    }
}

STATIC SAVEDS LONG memmorelist_cmp2func( struct Hook *hook, Object *obj, struct NList_CompareMessage *msg )
{
    LONG cmp;
    struct MemoryFreeEntry *mfe1, *mfe2;
    ULONG col1, col2;

    mfe1 = (struct MemoryFreeEntry *)msg->entry1;
    mfe2 = (struct MemoryFreeEntry *)msg->entry2;
    col1 = msg->sort_type & MUIV_NList_TitleMark_ColMask;
    col2 = msg->sort_type2 & MUIV_NList_TitleMark2_ColMask;

    if (msg->sort_type == MUIV_NList_SortType_None) return 0;

    if (msg->sort_type & MUIV_NList_TitleMark_TypeMask) {
        cmp = memmorelist_cmp2colfunc(mfe2, mfe1, col1);
    } else {
        cmp = memmorelist_cmp2colfunc(mfe1, mfe2, col1);
    }

    if (cmp != 0 || col1 == col2) return cmp;

    if (msg->sort_type2 & MUIV_NList_TitleMark2_TypeMask) {
        cmp = memmorelist_cmp2colfunc(mfe2, mfe1, col2);
    } else {
        cmp = memmorelist_cmp2colfunc(mfe1, mfe2, col2);
    }

    return cmp;
}
MakeStaticHook(memmorelist_cmp2hook, memmorelist_cmp2func);

STATIC void SetDetails( struct IClass *cl,
                        Object *obj )
{
    struct MemoryDetailWinData *mdwd = INST_DATA(cl, obj);
    struct MemoryEntry *me = mdwd->mdwd_Memory;
    struct MemHeader *mh = me->me_Header;
    ULONG total, inuse;

    total = (ULONG)mh->mh_Upper - (ULONG)mh->mh_Lower;
    inuse = total - (ULONG)mh->mh_Free;

    MySetContents(mdwd->mdwd_Texts[ 0], "%s", me->me_Name);
    MySetContents(mdwd->mdwd_Texts[ 1], "$%08lx", mh);
    MySetContents(mdwd->mdwd_Texts[ 2], "%s", GetNodeType(mh->mh_Node.ln_Type));
    MySetContents(mdwd->mdwd_Texts[ 3], "%ld", mh->mh_Node.ln_Pri);
    MySetContents(mdwd->mdwd_Texts[ 4], "$%08lx", mh->mh_Lower);
    MySetContents(mdwd->mdwd_Texts[ 5], "$%08lx", mh->mh_Upper);
    MySetContents(mdwd->mdwd_Texts[ 6], "$%08lx", mh->mh_First);
    MySetContents(mdwd->mdwd_Texts[ 7], "%12lD", inuse);
    MySetContents(mdwd->mdwd_Texts[ 8], "%12lD", mh->mh_Free);
    MySetContents(mdwd->mdwd_Texts[ 9], "%12lD", total);
    MySetContents(mdwd->mdwd_Texts[10], "%12lD", AvailMem(MEMF_LARGEST | mh->mh_Attributes));
    set(mdwd->mdwd_Texts[11], MUIA_FlagsButton_Flags, mh->mh_Attributes);

    DoMethod(mdwd->mdwd_ChunkList, MUIM_NList_Clear);

    if (me->me_Header->mh_First) {
        struct MemChunk *mc;
        struct MemoryFreeEntry *mfel;
        ULONG mccnt;

        mc = me->me_Header->mh_First;
        mccnt = 0;
        while (mc) {
            mccnt++;
            mc = mc->mc_Next;
        }
        mccnt += (mccnt / 10 < 0) ? 10 : (mccnt / 10);

        if ((mfel = tbAllocVecPooled(mdwd->mdwd_MemoryPool, mccnt * sizeof(struct MemoryFreeEntry))) != NULL) {
            struct MemoryFreeEntry *mfe;
            set(mdwd->mdwd_ChunkList, MUIA_NList_Quiet, TRUE);

            mc = me->me_Header->mh_First;
            mfe = mfel;
            while (mc != NULL && mccnt > 0) {
                _snprintf(mfe->mfe_Lower, sizeof(mfe->mfe_Lower), "$%08lx", mc);
                _snprintf(mfe->mfe_Upper, sizeof(mfe->mfe_Upper), "$%08lx", (ULONG)mc + mc->mc_Bytes);
                _snprintf(mfe->mfe_Size, sizeof(mfe->mfe_Size), "%12lD", mc->mc_Bytes);

                InsertBottomEntry(mdwd->mdwd_ChunkList, mfe);

                mfe++;
                mccnt--;

                mc = mc->mc_Next;
            }

            set(mdwd->mdwd_ChunkList, MUIA_NList_Quiet, FALSE);
        }
    }

    set(obj, MUIA_Window_Title, MyGetChildWindowTitle(txtMemoryDetailTitle, me->me_Name, mdwd->mdwd_Title, sizeof(mdwd->mdwd_Title)));
}

STATIC ULONG mNew( struct IClass *cl,
                   Object *obj,
                   struct opSet *msg )
{
    APTR group, texts[12], exitButton, chunklist;

    if ((obj = (Object *)DoSuperNew(cl, obj,
        MUIA_HelpNode, "Memory",
        MUIA_Window_ID, MakeID('.','M','E','M'),
        WindowContents, VGroup,

            Child, group = ScrollgroupObject,
                MUIA_CycleChain, TRUE,
                MUIA_Scrollgroup_FreeHoriz, FALSE,
                MUIA_Scrollgroup_Contents, VGroupV,
                    MUIA_Background, MUII_GroupBack,
                    Child, VGroup,
                        GroupFrame,
                        Child, ColGroup(2),
                            Child, MyLabel2(txtNodeName2),
                            Child, texts[ 0] = MyTextObject6(),
                            Child, MyLabel2(txtAddress2),
                            Child, texts[ 1] = MyTextObject6(),
                            Child, MyLabel2(txtNodeType2),
                            Child, texts[ 2] = MyTextObject6(),
                            Child, MyLabel2(txtNodePri2),
                            Child, texts[ 3] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryLower2),
                            Child, texts[ 4] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryUpper2),
                            Child, texts[ 5] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryFirst),
                            Child, texts[ 6] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryInUse),
                            Child, texts[ 7] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryFree),
                            Child, texts[ 8] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryTotal),
                            Child, texts[ 9] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryLargest),
                            Child, texts[10] = MyTextObject6(),
                            Child, MyLabel2(txtMemoryAttr2),
                            Child, texts[11] = FlagsButtonObject,
                                MUIA_FlagsButton_Type, MUIV_FlagsButton_Type_Word,
                                MUIA_FlagsButton_Title, txtMemoryAttr,
                                MUIA_FlagsButton_BitArray, memFlags,
                                MUIA_FlagsButton_WindowTitle, txtMemoryFlagsTitle,
                            End,
                        End,
                        Child, VGroup,
                            GroupFrameT(txtMemoryChunks),
                            Child, MyNListviewObject(&chunklist, MakeID('.','M','L','V'), "BAR,BAR,BAR P=" MUIX_R, NULL, NULL, &memmorelist_dsp2hook, &memmorelist_cmp2hook, FALSE),
                        End,
                    End,
                End,
            End,
            Child, MyVSpace(4),
            Child, exitButton = MakeButton(txtExit),
        End,
        TAG_MORE, msg->ops_AttrList)) != NULL)
    {
        struct MemoryDetailWinData *mdwd = INST_DATA(cl, obj);
        APTR parent;

        CopyMemQuick(texts, mdwd->mdwd_Texts, sizeof(mdwd->mdwd_Texts));
        mdwd->mdwd_ChunkList = chunklist;
        mdwd->mdwd_MemoryPool = tbCreatePool(MEMF_CLEAR, 4096, 4096);

        parent = (APTR)GetTagData(MUIA_Window_ParentWindow, (ULONG)NULL, msg->ops_AttrList);

        set(obj, MUIA_Window_DefaultObject, group);

        DoMethod(parent,         MUIM_Window_AddChildWindow, obj);
        DoMethod(obj,            MUIM_Notify, MUIA_Window_CloseRequest, TRUE,  MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_Window_RemChildWindow, obj);
        DoMethod(exitButton,     MUIM_Notify, MUIA_Pressed,             FALSE, obj,                     3, MUIM_Set, MUIA_Window_CloseRequest, TRUE);
    }

    return (ULONG)obj;
}

STATIC ULONG mDispose( struct IClass *cl,
                       Object *obj,
                       Msg msg )
{
    struct MemoryDetailWinData *mdwd = INST_DATA(cl, obj);

    set(obj, MUIA_Window_Open, FALSE);
    DoMethod(mdwd->mdwd_ChunkList, MUIM_NList_Clear);

    tbDeletePool(mdwd->mdwd_MemoryPool);

    return DoSuperMethodA(cl, obj, msg);
}

STATIC ULONG mSet( struct IClass *cl,
                   Object *obj,
                   struct opSet *msg )
{
    struct MemoryDetailWinData *mdwd = INST_DATA(cl, obj);
    struct TagItem *tags, *tag;

    tags = msg->ops_AttrList;
    while ((tag = NextTagItem(&tags)) != NULL) {
        switch (tag->ti_Tag) {
            case MUIA_Window_ParentWindow:
                DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, (APTR)tag->ti_Data, 2, MUIM_Window_RemChildWindow, obj);
                break;

            case MUIA_MemoryDetailWin_Memory:
                mdwd->mdwd_Memory = (struct MemoryEntry *)tag->ti_Data;
                SetDetails(cl, obj);
                break;
        }
    }

    return DoSuperMethodA(cl, obj, (Msg)msg);
}

DISPATCHER(MemoryDetailWinDispatcher)
{
    switch (msg->MethodID) {
        case OM_NEW:     return (mNew(cl, obj, (APTR)msg));
        case OM_DISPOSE: return (mDispose(cl, obj, (APTR)msg));
        case OM_SET:     return (mSet(cl, obj, (APTR)msg));
    }

    return DoSuperMethodA(cl, obj, msg);
}
DISPATCHER_END

APTR MakeMemoryDetailWinClass( void )
{
    return MUI_CreateCustomClass(NULL, NULL, ParentWinClass, sizeof(struct MemoryDetailWinData), DISPATCHER_REF(MemoryDetailWinDispatcher));
}
