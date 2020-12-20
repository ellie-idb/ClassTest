#ifndef _STRUCTS_H_
#define _STRUCTS_H_

// ripped out structs from Tork

class SimObject;
typedef unsigned int U32;
typedef float F32;
typedef signed int S32;
typedef const char* StringTableEntry;

extern DWORD ImageBase;
extern DWORD ImageSize;

// yuck
#define Offset(x, cls) ((size_t)((const char *)&(((cls *)0)->x)-(const char *)0))
#include "tvector.h"

enum
{
	TypeChar,
	TypeU8,
	TypeS16,
	TypeU16,
	TypeInt,
	TypeIntList,
	TypeBool,
	TypeBoolList,
	TypeFloat,
	TypeFloatList,
	TypeString,
	TypeCaseString,
	TypeFileName,
	TypeEnumVal,
	TypeFlag,
	TypeColorI,
	TypeColorF,
	TypeSimObjectPtr,
	TypePoint2I,
	TypePoint2F,
	TypePoint3F,
	TypePoint4F,
	TypeRectI,
	TypeRectF,
	TypeMatrixPosition,
	TypeMatrixRotation,
	TypeBox3F,
	TypeGuiProfile,
};

enum
{
	SizeTypeChar = 1,
	SizeTypeU8 = 1,
	SizeTypeS16 = 2,
	SizeTypeU16 = 2,
	SizeTypeInt = 4,
	SizeTypeIntList = 12,
	SizeTypeBool = 1,
	SizeTypeBoolList = 12,
	SizeTypeFloat = 4,
	SizeTypeFloatList = 12,
	SizeTypeString = 4,
	SizeTypeCaseString = 4,
	SizeTypeFileName = 4,
	SizeTypeEnumVal = 4,
	SizeTypeFlag = 4,
	SizeTypeColorI = 4,
	SizeTypeColorF = 16,
	SizeTypeSimObjectPtr = 4,
	SizeTypePoint2I = 8,
	SizeTypePoint2F = 8,
	SizeTypePoint3F = 12,
	SizeTypePoint4F = 16,
	SizeTypeRectI = 16,
	SizeTypeRectF = 16,
	SizeTypeMatrixPosition = 4,
	SizeTypeMatrixRotation = 64,
	SizeTypeBox3F = 24,
	SizeTypeGuiProfile = 4,
};

class Namespace
{
public:
	typedef const char* (*StringCallback)(SimObject* obj, int argc, const char* argv[]);
	typedef int(*IntCallback)(SimObject* obj, int argc, const char* argv[]);
	typedef float(*FloatCallback)(SimObject* obj, int argc, const char* argv[]);
	typedef void(*VoidCallback)(SimObject* obj, int argc, const char* argv[]);
	typedef bool(*BoolCallback)(SimObject* obj, int argc, const char* argv[]);

	const char* mName;
	const char* mPackage;

	Namespace* mParent;
	Namespace* mNext;
	void* mClassRep;
	U32 mRefCountToParent;
	struct Entry
	{
		enum
		{
			GroupMarker = -3,
			OverloadMarker = -2,
			InvalidFunctionType = -1,
			ScriptFunctionType,
			StringCallbackType,
			IntCallbackType,
			FloatCallbackType,
			VoidCallbackType,
			BoolCallbackType
		};

		Namespace* mNamespace;
		//char _padding1[4];
		Entry* mNext;
		const char* mFunctionName;
		S32 mType;
		S32 mMinArgs;
		S32 mMaxArgs;
		const char* mUsage;
		const char* mPackage;
		void* mCode; // CodeBlock *mCode;
		U32 mFunctionOffset;
		union {
			StringCallback mStringCallbackFunc;
			IntCallback mIntCallbackFunc;
			VoidCallback mVoidCallbackFunc;
			FloatCallback mFloatCallbackFunc;
			BoolCallback mBoolCallbackFunc;
			const char* mGroupName;
		} cb;
	};
	Entry* mEntryList;
	Entry** mHashTable;
	U32 mHashSize;
	U32 mHashSequence;  ///< @note The hash sequence is used by the autodoc console facility
						///        as a means of testing reference state.
	char* lastUsage;


	bool linkTo(Namespace* parent)
	{
		Namespace* parent_1 = this->mParent;
		Namespace* walk;
		for (walk = this; parent_1; parent_1 = parent_1->mParent)
		{
			if (parent_1->mName != this->mName)
				break;
			walk = parent_1;
		}

		Namespace* v4 = walk->mParent;
		if (!v4 || v4 == parent)
		{
			this->mRefCountToParent++;
			walk->mParent = parent;
			return true;
		}
		else
		{
			return false;
		}
	}
};

static Namespace* mGlobalNamespace;


#define BIT(x) (1 << (x))

enum NetClassTypes {
	NetClassTypeObject = 0,
	NetClassTypeDataBlock,
	NetClassTypeEvent,
	NetClassTypesCount,
};

enum NetClassGroups {
	NetClassGroupGame = 0,
	NetClassGroupCommunity,
	NetClassGroup3,
	NetClassGroup4,
	NetClassGroupsCount,
};

enum NetClassMasks {
	NetClassGroupGameMask = BIT(NetClassGroupGame),
	NetClassGroupCommunityMask = BIT(NetClassGroupCommunity),
};

enum NetDirection
{
	NetEventDirAny,
	NetEventDirServerToClient,
	NetEventDirClientToServer,
};

class ConsoleObject;

class AbstractClassRep
{
	friend class ConsoleObject;
public:
	AbstractClassRep(): mClassGroupMask(0), mClassType(0), mNetEventDir(0), mClassId(), mClassName(nullptr),
						nextClass(nullptr), mNamespace(nullptr), mFieldList(Vector<Field>()), mDynamicGroupExpand(false) {}

	virtual ~AbstractClassRep() { }
	
	S32 mClassGroupMask;
	S32 mClassType;
	S32 mNetEventDir;
	S32 mClassId[4];

	S32                        getClassId(U32 netClassGroup)   const;
	static U32                 getClassCRC(U32 netClassGroup);
	const char* getClassName() const;
	static AbstractClassRep* getClassList();
	Namespace* getNameSpace();
	AbstractClassRep* getNextClass();
	
	virtual ConsoleObject* create() const = 0;
public:
	virtual void init() const = 0;
	
	const char* mClassName;
	AbstractClassRep* nextClass;
	Namespace* mNamespace;
public:
	typedef bool (*SetDataNotify)( void *obj, const char *data );
	typedef const char *(*GetDataNotify)( void *obj, const char *data );

	struct Field
	{
		const char* pFieldname;
		const char* pGroupname;
		const char* pFieldDocs;
		bool groupExpand;
		U32 type;
		U32 offset;
		U32 elementCount;
		void* table;
		int flag;
		void* validator;
	};
	typedef Vector<Field> FieldList;

	FieldList mFieldList;
	bool mDynamicGroupExpand;
	const Field* findField(StringTableEntry fieldName) const;
protected:
	static ConsoleObject* create(const char* in_pClassName);
	static ConsoleObject* create(const U32 groupId, const U32 typeId, const U32 in_classId);
public:

	static AbstractClassRep***** classTable;
	static AbstractClassRep** classLinkList;
	static U32*                classCRC;
	static U32**  NetClassCount;
	static U32**  NetClassBitSize;

	static void registerClassRep(AbstractClassRep*);
	static void initialize(); // Called from Con::init once on startup
};

inline AbstractClassRep* AbstractClassRep::getClassList()
{
	return *classLinkList;
}

inline U32 AbstractClassRep::getClassCRC(U32 group)
{
	return classCRC[group];
}

inline AbstractClassRep* AbstractClassRep::getNextClass()
{
	return nextClass;
}

inline S32 AbstractClassRep::getClassId(U32 group) const
{
	return mClassId[group];
}

inline const char* AbstractClassRep::getClassName() const
{
	return mClassName;
}

inline Namespace* AbstractClassRep::getNameSpace()
{
	return mNamespace;
}

template <class T>
class ConcreteClassRep : public AbstractClassRep
{
public:
	ConcreteClassRep(const char* name, S32 netClassGroupMask, S32 netClassType, S32 netEventDir)
	{
		// name is a static compiler string so no need to worry about copying or deleting
		mClassName = name;

		// Clean up mClassId
		for (U32 i = 0; i < NetClassGroupsCount; i++)
			mClassId[i] = -1;

		// Set properties for this ACR
		mClassType = netClassType;
		mClassGroupMask = netClassGroupMask;
		mNetEventDir = netEventDir;
	}

	/// Perform class specific initialization tasks.
	///
	/// Link namespaces, call initPersistFields() and consoleInit().
	void init() const
	{
		// Get handle to our parent class, if any, and ourselves (we are our parent's child).
		AbstractClassRep* parent = T::getParentStaticClassRep();
		AbstractClassRep* child = T::getStaticClassRep();
		
		// If we got reps, then link those namespaces! (To get proper inheritance.)
		if (parent && child)
			child->getNameSpace()->linkTo(parent->getNameSpace());

		// Finally, do any class specific initialization...
		T::initPersistFields();
		T::consoleInit();
	}

	/// Wrap constructor.
	ConsoleObject* create() const
	{
		T* obj = new T;
		return (ConsoleObject*)obj;
	}
};


struct EnumTable
{
	/// Number of enumerated items in the table.
	S32 size;

	/// This represents a specific item in the enumeration.
	struct Enums
	{
		S32 index;        ///< Index label maps to.
		const char* label;///< Label for this index.
	};

	Enums* table;

	/// Constructor.
	///
	/// This sets up the EnumTable with predefined data.
	///
	/// @param sSize  Size of the table.
	/// @param sTable Pointer to table of Enums.
	///
	/// @see gLiquidTypeTable
	/// @see gAlignTable
	EnumTable(S32 sSize, Enums* sTable)
	{
		size = sSize; table = sTable;
	}
};

class TypeValidator
{
	signed int fieldIndex;
	virtual void ValidateType(SimObject* obj, void* typePtr);
};


class ConsoleObject
{
protected:
	/// @deprecated This is disallowed.
	ConsoleObject() { /* disallowed */ }
	/// @deprecated This is disallowed.
	ConsoleObject(const ConsoleObject&);

protected:
	/// Get a reference to a field by name.
	const AbstractClassRep::Field* findField(StringTableEntry fieldName) const;

public:

	/// Gets the ClassRep.
	virtual AbstractClassRep* getClassRep() const;

	/// Set the value of a field.
	bool setField(const char* fieldName, const char* value);
	virtual ~ConsoleObject();

public:
	/// @name Object Creation
	/// @{
	static ConsoleObject* create(const char* in_pClassName);
	static ConsoleObject* create(const U32 groupId, const U32 typeId, const U32 in_classId);
	/// @}

public:
	/// Get the classname from a class tag.
	static const char* lookupClassName(const U32 in_classTag);

protected:
	/// @name Fields
	/// @{

	/// Mark the beginning of a group of fields.
	///
	/// This is used in the consoleDoc system.
	/// @see console_autodoc
	static void __fastcall addGroup(const char* in_pGroupname, const char* in_pGroupDocs = NULL);

	/// Mark the end of a group of fields.
	///
	/// This is used in the consoleDoc system.
	/// @see console_autodoc
	static void __fastcall endGroup(const char* in_pGroupname);

	/// Register a complex field.
	///
	/// @param  in_pFieldname     Name of the field.
	/// @param  in_fieldType      Type of the field. @see ConsoleDynamicTypes
	/// @param  in_fieldOffset    Offset to  the field from the start of the class; calculated using the Offset() macro.
	/// @param  in_elementCount   Number of elements in this field. Arrays of elements are assumed to be contiguous in memory.
	/// @param  in_table          An EnumTable, if this is an enumerated field.
	/// @param  in_pFieldDocs     Usage string for this field. @see console_autodoc
	static void __fastcall addField(const char* in_pFieldname,
		const U32     in_fieldType,
		const size_t in_fieldOffset,
		const U32     in_elementCount = 1,
		EnumTable* in_table = NULL,
		const char* in_pFieldDocs = NULL);

	/// Register a simple field.
	///
	/// @param  in_pFieldname  Name of the field.
	/// @param  in_fieldType   Type of the field. @see ConsoleDynamicTypes
	/// @param  in_fieldOffset Offset to  the field from the start of the class; calculated using the Offset() macro.
	/// @param  in_pFieldDocs  Usage string for this field. @see console_autodoc
    static void __fastcall addField(const char* in_pFieldname,
		const U32     in_fieldType,
		const size_t in_fieldOffset,
		const char* in_pFieldDocs)
    {
		addField(in_pFieldname, in_fieldType, in_fieldOffset, in_fieldOffset, NULL, in_pFieldDocs);
    }

	/// Register a validated field.
	///
	/// A validated field is just like a normal field except that you can't
	/// have it be an array, and that you give it a pointer to a TypeValidator
	/// subclass, which is then used to validate any value placed in it. Invalid
	/// values are ignored and an error is printed to the console.
	///
	/// @see addField
	/// @see typeValidators.h
	static void __fastcall addFieldV(const char* in_pFieldname,
		const U32      in_fieldType,
		const size_t  in_fieldOffset,
		TypeValidator* v,
		const char* in_pFieldDocs = NULL);
	
	/// Add a deprecated field.
	///
	/// A deprecated field will always be undefined, even if you assign a value to it. This
	/// is useful when you need to make sure that a field is not being used anymore.
	static void __fastcall addDeprecatedField(const char* fieldName);

	/// Remove a field.
	///
	/// Sometimes, you just have to remove a field!
	/// @returns True on success.
	static bool removeField(const char* in_pFieldname);

	/// @}
public:
	/// Register dynamic fields in a subclass of ConsoleObject.
	///
	/// @see addField(), addFieldV(), addDepricatedField(), addGroup(), endGroup()
	static void initPersistFields();

	/// Register global constant variables and do other one-time initialization tasks in
	/// a subclass of ConsoleObject.
	///
	/// @deprecated You should use ConsoleMethod and ConsoleFunction, not this, to
	///             register methods or commands.
	/// @see console
	static void consoleInit();

	/// @name Field List
	/// @{

	/// Get a list of all the fields. This information cannot be modified.
	const AbstractClassRep::FieldList& getFieldList() const;

	/// Get a list of all the fields, set up so we can modify them.
	///
	/// @note This is a bad trick to pull if you aren't very careful,
	///       since you can blast field data!
	AbstractClassRep::FieldList& getModifiableFieldList();

	/// Get a handle to a boolean telling us if we expanded the dynamic group.
	///
	/// @see GuiInspector::Inspect()
	bool& getDynamicGroupExpand();
	/// @}

	/// @name ConsoleObject Implementation
	///
	/// These functions are implemented in every subclass of
	/// ConsoleObject by an IMPLEMENT_CONOBJECT or IMPLEMENT_CO_* macro.
	/// @{

	/// Get the abstract class information for this class.
	static AbstractClassRep* getStaticClassRep() { return NULL; }

	/// Get the abstract class information for this class's superclass.
	static AbstractClassRep* getParentStaticClassRep() { return NULL; }

	/// Get our network-layer class id.
	///
	/// @param  netClassGroup  The net class for which we want our ID.
	/// @see
	S32 getClassId(U32 netClassGroup) const;

	/// Get our compiler and platform independent class name.
	///
	/// @note This name can be used to instantiate another instance using create()
	const char* getClassName() const;
};


#define DECLARE_CONOBJECT(className)                    \
   static ConcreteClassRep<className> dynClassRep;      \
   static AbstractClassRep* getParentStaticClassRep();  \
   static AbstractClassRep* getStaticClassRep();        \
   virtual AbstractClassRep* getClassRep() const

#define IMPLEMENT_CONOBJECT(className)                                                            \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; }           \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; }                      \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); } \
   ConcreteClassRep<className> className::dynClassRep(#className, 0, -1, 0)

#define IMPLEMENT_CO_NETOBJECT_V1(className)                    \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; }                 \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; }                            \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); }       \
   ConcreteClassRep<className> className::dynClassRep(#className, NetClassGroupGameMask, NetClassTypeObject, 0)

#define IMPLEMENT_CO_DATABLOCK_V1(className)                                                            \
   AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; }                 \
   AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; }                            \
   AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); }       \
   ConcreteClassRep<className> className::dynClassRep(#className, NetClassGroupGameMask, NetClassTypeDataBlock, 0)


struct Stream;
struct Stream__vft
{
	void(__thiscall* dtor)(Stream* this_);
	bool(__thiscall* read)(Stream* this_, int bytes, void* buf);
	bool(__thiscall* write)(Stream* this_, int bytes, void* buf);
	bool(__thiscall* hasCap)(Stream* this_, int cap);
	int(__thiscall* getPos)(Stream* this_);
	bool(__thiscall* setPos)(Stream* this_, int newPos);
	int(__thiscall* getStreamSize)(Stream* this_);
	void(__thiscall* readString)(Stream* this_, char stringBuf[256]);
	void(__thiscall* writeString)(Stream* this_, const char* str, signed int maxLen);
};

/* 176 */
struct Stream
{
	Stream__vft* __vftable;
	int m_streamStatus;
};

extern void* pSimObjectVTable;

class SimObject : public ConsoleObject
{
	typedef ConsoleObject Parent;
public:
	const char* objectName;
	SimObject* nextNameObject;
	SimObject* nextManagerNameObject;
	SimObject* nextIdObject;
	void* mGroup;
	unsigned int mFlags;
	void* mNotifyList;
	unsigned int id;
	Namespace* mNamespace;
	unsigned int mTypeMask;
	void* filler;
	void* mFieldDictionary;

	static AbstractClassRep* getStaticClassRep()
	{
		return (AbstractClassRep*)(ImageBase + 0x387A70);
	}

	virtual AbstractClassRep* getClassRep() const override
	{
		return getStaticClassRep();
	}

	virtual ~SimObject() override
	{
		
	}

	/* stubs */
	virtual bool unk_1(bool arg) { return true; }
	virtual bool onAdd() { return true; }
	virtual void onRemove() {}
	virtual void stub_0() {}
	virtual void stub_1() {}
	virtual void stub_2() {}
	virtual void stub_3() {}
	virtual void stub_4() {}
	virtual void stub_5() {}
	virtual void stub_6() {}
	virtual void stub_7() {}
	virtual void stub_8() {}
	virtual bool stub_9() { return false; }
	virtual bool write(Stream* stream, U32 tabStop, U32 flags) { return true; }
	virtual bool stub_10() { return false; }
};

struct CodeBlock
{
	const char* name;
	int unk_1;
	void* globalStrings;
	void* functionStrings;
	void* globalFloats;
	void* functionFloats;
	int codeSize;
	int unk_2;
	char* code;
	int refCount;
	int lineBreakPairCount;
	int* lineBreakPairs;
	int breakListSize;
	int* breakList;
	CodeBlock* nextFile;
	const char* mRoot;
};

struct Dictionary
{
	struct Entry {
		enum
		{
			TypeInternalInt = -3,
			TypeInternalFloat = -2,
			TypeInternalString = -1,
		};

		const char* name;
		Entry* next;
		S32 type;
		char* sval;
		U32 ival; // doubles as strlen
		F32 fval;
		U32 bufferLen;
		void* dataPtr;
	};

	struct HashTableData
	{
		int size;
		int count;
		Entry** data;
		void* owner;
	};

	HashTableData table;
	const char* scopeName;
	const char* scopeNamespace;
	CodeBlock* block;
	int ip;
};

struct ExprEvalState
{
	void* thisObject;
	Dictionary::Entry* currentVariable;
	bool traceOn;
	Dictionary globalVars;
	Vector<Dictionary> stack;
};

static ExprEvalState* gEvalState;

struct DataChunker
{
	struct DataBlock
	{
		DataBlock* next;
		char* data;
		int curIndex;
	};

	DataBlock* curBlock;
	signed int chunkSize;
};

struct StringStack
{
	char* mBuffer;
	int mBufferSize;
	const char* mArgv[20];
	int mFrameOffsets[1024];
	int mStartOffsets[1024];
	int mNumFrames;
	int mArgc;
	int mStart;
	int mLen;
	int mStartStackSize;
	int mFunctionOffset;
	int mArgBufferSize;
	char* mArgBuffer;
};

struct StringTable {
	struct Node {
		char* val;
		Node* next;
	};

	Node** buckets;
	U32 numBuckets;
	U32 itemCount;
	DataChunker memPool;
};

struct TextureObject
{
	TextureObject* next;
	TextureObject* prev;
	TextureObject* hashNext;
	unsigned int texGLName;
	unsigned int smallTexGLName;
	const char* texFileName;
	unsigned int* type_GBitmap_bitmap;
	unsigned int texWidth;
	unsigned int texHeight;
	unsigned int bitmapWidth;
	unsigned int bitmapHeight;
	unsigned int downloadedWidth;
	unsigned int downloadedHeight;
	unsigned int enum_TextureHandleType_type;
	bool filterNearest;
	bool clamp;
	bool holding;
	int refCount;
};

struct Point2I {
	int x, y;
};

struct GameInterface
{
	void** vtable;
	int mJournalMode;
	bool mRunning;
	bool mJournalBreak;
	bool mRequiresRestart;
};

#endif