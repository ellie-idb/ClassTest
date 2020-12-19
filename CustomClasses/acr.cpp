#include <cstring>
#include "Torque.h"
#include "structs.h"

AbstractClassRep**** AbstractClassRep::classTable;
AbstractClassRep* AbstractClassRep::classLinkList;
U32** AbstractClassRep::NetClassCount = NULL;

AbstractClassRep* ConsoleObject::getClassRep() const
{
    return 0;
}

ConsoleObject::~ConsoleObject()
{
    typedef void(__thiscall* Fn)(ConsoleObject* a, bool b);
    Fn v = (Fn)(ImageBase + 0x1D0B0);
    v(this, true);
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
    AbstractClassRep** a = (AbstractClassRep**)(ImageBase + 0x3C5D18);
    for (AbstractClassRep* walk = *a; walk; walk = walk->nextClass)
    {
        if (walk == in_pRep)
        {
            Printf("Ignoring registration of duplicate ACR %s", in_pRep->getClassName());
            return;
        }
    }
	// otherwise, patch it
    DWORD oldProtection;
    VirtualProtect(a, 4, PAGE_EXECUTE_READWRITE, &oldProtection);
    in_pRep->nextClass = *a;
    *a = in_pRep;
    VirtualProtect(a, 4, oldProtection, &oldProtection);
}

ConsoleObject* AbstractClassRep::create(const char* in_pClassName)
{
    for (AbstractClassRep* walk = classLinkList; walk; walk = walk->nextClass)
        if (!strcmp(walk->getClassName(), in_pClassName))
            return walk->create();

    return NULL;
}

ConsoleObject* AbstractClassRep::create(const U32 groupId, const U32 typeId, const U32 in_classId)
{
    if (!(in_classId < NetClassCount[groupId][typeId])) return NULL;
	
    // Look up the specified class and create it.
    if (classTable[groupId][typeId][in_classId])
        return classTable[groupId][typeId][in_classId]->create();

    return NULL;
}

