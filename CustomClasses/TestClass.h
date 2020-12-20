#ifndef _TESTCLASS_H_
#define _TESTCLASS_H_
#include "Torque.h"

/*
 *
 *	This header file is an example implementation of a SimObject-compatible class.
 *  However - be aware, overriding any virtual functions will not work unless you restore them
 *  after the vtable patching phase.
 *  We need to copy in the SimObject vft to avoid crashing (and as such, SimObject's declaration is mostly stubbed out)
 *  
 */

class TestClass : SimObject
{
	typedef SimObject Parent;
public:
	TestClass();
	~TestClass();
	
	int testInt;
	
	static void initPersistFields();
	static void consoleInit();
	DECLARE_CONOBJECT(TestClass);
protected:
	void fixVTable();
};



#endif