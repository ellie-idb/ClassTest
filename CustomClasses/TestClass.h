#ifndef _TESTCLASS_H_
#define _TESTCLASS_H_
#include "Torque.h"
class TestClass : SimObject
{
	typedef SimObject Parent;
public:

	TestClass();
	~TestClass();
	static void initPersistFields();
	static void consoleInit();
	void fixVTable();
	DECLARE_CONOBJECT(TestClass);
};


#endif