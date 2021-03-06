#ifndef ICONOBJECT_INTERFACE_DEF_H
#define ICONOBJECT_INTERFACE_DEF_H

/*
** This file was machine generated by idltool 52.1.
** Do not edit
*/ 

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_EXEC_H
#include <exec/exec.h>
#endif
#ifndef EXEC_INTERFACES_H
#include <exec/interfaces.h>
#endif

#ifndef INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif
#ifndef WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif

struct IconobjectIFace
{
	struct InterfaceData Data;

	uint32 APICALL (*Obtain)(struct IconobjectIFace *Self);
	uint32 APICALL (*Release)(struct IconobjectIFace *Self);
	void APICALL (*Expunge)(struct IconobjectIFace *Self);
	struct Interface * APICALL (*Clone)(struct IconobjectIFace *Self);
	Object * APICALL (*NewIconObject)(struct IconobjectIFace *Self, CONST_STRPTR name, CONST struct TagItem * tagList);
	Object * APICALL (*NewIconObjectTags)(struct IconobjectIFace *Self, CONST_STRPTR name, ...);
	VOID APICALL (*DisposeIconObject)(struct IconobjectIFace *Self, Object * iconObject);
	Object * APICALL (*GetDefIconObject)(struct IconobjectIFace *Self, ULONG iconType, CONST struct TagItem * tagList);
	Object * APICALL (*GetDefIconObjectTags)(struct IconobjectIFace *Self, ULONG iconType, ...);
	LONG APICALL (*PutIconObject)(struct IconobjectIFace *Self, Object * iconObject, CONST_STRPTR path, CONST struct TagItem * tagList);
	LONG APICALL (*PutIconObjectTags)(struct IconobjectIFace *Self, Object * iconObject, CONST_STRPTR path, ...);
	ULONG APICALL (*IsIconName)(struct IconobjectIFace *Self, CONST_STRPTR fileName);
	Object * APICALL (*Convert2IconObject)(struct IconobjectIFace *Self, struct DiskObject * diskObject);
	Object * APICALL (*Convert2IconObjectA)(struct IconobjectIFace *Self, struct DiskObject * diskObject, CONST struct TagItem * tagList);
	Object * APICALL (*Convert2IconObjectTags)(struct IconobjectIFace *Self, struct DiskObject * diskObject, ...);
};

#endif /* ICONOBJECT_INTERFACE_DEF_H */
