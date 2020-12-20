#include <cstring>
#include "Torque.h"
#include "structs.h"

// this is dirty. classTable is actually
// AbstractClassRep ** AbstractClassRep::classTable[NetClassGroupsCount][NetClassTypesCount];
// but this is as close as we're getting (minus the deref that you need to do)
AbstractClassRep***** AbstractClassRep::classTable;
AbstractClassRep** AbstractClassRep::classLinkList;
U32** AbstractClassRep::NetClassCount = NULL;

/* ConsoleObject does not have an ACR, don't refer to the vTable for this one */
AbstractClassRep* ConsoleObject::getClassRep() const
{
    return 0;
}

/* Dirty hack to call the dtor */
ConsoleObject::~ConsoleObject()
{
    typedef void(__thiscall* Fn)(ConsoleObject* a, bool b);
    Fn v = (Fn)(ImageBase + 0x1D0B0);
    v(this, false);
}

const AbstractClassRep::Field* AbstractClassRep::findField(StringTableEntry name) const
{
    for (U32 i = 0; i < mFieldList.size(); i++)
        if (mFieldList[i].pFieldname == name)
            return &mFieldList[i];

    return NULL;
}

//--------------------------------------
void AbstractClassRep::registerClassRep(AbstractClassRep* in_pRep)
{
    for (AbstractClassRep* walk = getClassList(); walk; walk = walk->nextClass)
    {
        if (walk == in_pRep)
        {
            Printf("Ignoring registration of duplicate ACR %s", in_pRep->getClassName());
            return;
        }
    }

    in_pRep->nextClass = *classLinkList;
    *classLinkList = in_pRep;
}

ConsoleObject* AbstractClassRep::create(const char* in_pClassName)
{
    for (AbstractClassRep* walk = *classLinkList; walk; walk = walk->nextClass)
        if (!strcmp(walk->getClassName(), in_pClassName))
            return walk->create();

    return NULL;
}

ConsoleObject* AbstractClassRep::create(const U32 groupId, const U32 typeId, const U32 in_classId)
{
    if (!(in_classId < NetClassCount[groupId][typeId])) return NULL;
	
    // Look up the specified class and create it.
    AbstractClassRep* actualRep = *classTable[groupId][typeId][in_classId];
    if (actualRep)
        return actualRep->create();

    return NULL;
}

void __fastcall ConsoleObject::addGroup(const char* in_pGroupname, const char* in_pGroupDocs)
{
    ConsoleObjectAddGroup(in_pGroupname, in_pGroupDocs);
}

void __fastcall ConsoleObject::endGroup(const char* in_pGroupname)
{
    ConsoleObjectEndGroup(in_pGroupname);
}

void __fastcall ConsoleObject::addField(const char* in_pFieldname,
    const U32     in_fieldType,
    const size_t in_fieldOffset,
    const U32     in_elementCount,
    EnumTable* in_table,
    const char* in_pFieldDocs)
{
	/*
	 * This function doesn't fix the stack (naked call?)
	 * We need to fix ESP so we don't mess anything up   
	 */
	__asm {
        push in_pFieldDocs;
        push in_table;
        push in_elementCount;
        push in_fieldOffset;
        mov edx, in_fieldType;
        mov ecx, in_pFieldname;
        call ConsoleObjectAddField;
        add esp, 10h;
	}
}

void __fastcall ConsoleObject::addFieldV(const char* in_pFieldname,
    const U32      in_fieldType,
    const size_t  in_fieldOffset,
    TypeValidator* v,
    const char* in_pFieldDocs)
{
    ConsoleObjectAddFieldV(in_pFieldname, in_fieldType, in_fieldOffset, v);
}

void __fastcall ConsoleObject::addDeprecatedField(const char* fieldName)
{
    ConsoleObjectAddDeprecatedField(fieldName);
}