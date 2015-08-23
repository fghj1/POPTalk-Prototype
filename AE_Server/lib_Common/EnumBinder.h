#pragma once
// =============================================================
// #include "EnumBinder.h"

// =============================================================
// Notes:
// 1) enum values MUST be unique, however:
//     - the enum values do NOT have to start at zero (e.g. using =4 on the first enum)
//     - the enum values do NOT have to be in any particular order (e.g. 2,5,1,6,8)
//     - the enum values can contain gaps (e.g. 0,1,2,7,8,9)
//     - BUT enum values that have automatic ordering (start at 0, increment by 1)
//       will have faster lookups from enum to values (constant time vs. O(n))
//
// 2) Exceptions
//    to use exceptions, #define CENUMBINDER_USE_EXCEPTIONS
//

/*
// =============================================================
// Example:
// Step 1) create your standard C++ enum definition:

enum eShapes {
	eSquare,
	eCircle,
	eRectangle,
	eTriangle,
};


// Step 2) BIND_ENUM parameter 1 is the enum type
//         BIND_ENUM parameter 2 is the name of the new class, i.e. the "enum binding" class (any name is okay)

BIND_ENUM(eShapes, CEnumShapes)
{
	{ eSquare    , _T("Square")   , _T("Square (4 equal length edges)") },
	{ eCircle    , _T("Circle")   , _T("Circle (1 continuous edge)"     },
	{ eRectangle , _T("Rectangle"), _T("Rectangle (4 edges)")           },
	{ eTriangle  , _T("Triangle") , _T("Triangle (3 edges)")            },
};

// Step 3) Define the mapping between enum and the two strings (above)
           item 1 is the enum
		   item 2 is the "code string" (for writing to files/registry) i.e. strings the user will not ever see
		   item 3 is the "user string" (for displaying to the user)
		   (these items are listed in the same order as the CEnumBinderData class, listed below)
*/

// =============================================================

#ifdef CENUMBINDER_USE_EXCEPTIONS
#include <stdexcept>
#endif //CENUMBINDER_USE_EXCEPTIONS

#ifdef _DEBUG
#define CENUMBINDER_AUTO_UNIT_TEST AutoUnitTest()
#else _DEBUG
#define CENUMBINDER_AUTO_UNIT_TEST ((void)0)
#endif // _DEBUG




	#include "./AEString.h"
	


	template <typename Tenum, typename Tproperties>
	class CEnumBinder
	{
	public:
		static int GetSize()
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			static const int num = sizeof(m_properties) / sizeof(m_properties[0]);
			return num;
		}
		
		// const access to elements by index (preferred to ElementAt)
		static const Tproperties& GetAt(int zeroBasedIndex) 
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			ASSERT(IndexInRange(zeroBasedIndex));
			return m_properties[zeroBasedIndex];
		}
		// const access to elements by enum (preferred to ElementAt)
		static const Tproperties& GetAt(const Tenum enumItem) 
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			int zeroBasedIndex(0);
			EnumToIndex(enumItem, zeroBasedIndex);
			return m_properties[zeroBasedIndex];
		}

		// non-const access by index (i.e. if changes required to the data)
		static Tproperties& ElementAt(int zeroBasedIndex)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			ASSERT(IndexInRange(zeroBasedIndex));
			return m_properties[zeroBasedIndex];
		}
		// non-const access by enum (i.e. if changes required to the data)
		static Tproperties& ElementAt(const Tenum enumItem)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			int zeroBasedIndex(0);
			EnumToIndex(enumItem, zeroBasedIndex);
			return m_properties[zeroBasedIndex];
		}

		// converts the enum to a code-string (i.e. an internal string for file/registry use)
		// returns false if there was a problem (which indicates a bug - see UnitTest() function)
		static bool EnumToCodeString(const Tenum enumItem, sCHAR& codeString_OUT)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			int zeroBasedIndex(0);
			if(EnumToIndex(enumItem, zeroBasedIndex)){
				codeString_OUT = m_properties[zeroBasedIndex].m_stringCode;
				return true;
			}else{
				return false;
			}
		}

		// converts the enum to a code-string (i.e. an internal string for file/registry use)
		static const sCHAR& EnumToCodeString(const Tenum enumItem)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			int zeroBasedIndex(0);
			EnumToIndex(enumItem, zeroBasedIndex);
			return m_properties[zeroBasedIndex].m_stringCode;
		}

		// converts the enum to a user-string (i.e. for display to the user)
		static const sCHAR& EnumToUserString(const Tenum enumItem)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			int zeroBasedIndex(0);
			EnumToIndex(enumItem, zeroBasedIndex);
			return m_properties[zeroBasedIndex].m_stringUser;
		}

		// converts the code-string to the enum
		// returns false if it could not match the string
		static bool CodeStringToEnum(LPCTSTR stringToCheck, Tenum& enumValue_OUT)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			const int num = GetSize();
			for(int i=0; i<num; ++i){
				if(m_properties[i].m_stringCode == stringToCheck){
					enumValue_OUT = m_properties[i].m_enum;
					return true;
				}
			}		
			return false; // not an ASSERT (may be checking strings e.g. file reading)
		}

		// typically for use with registry classes
		template<typename CStringOrSimilar>
		static bool CodeStringEnumExchange(CStringOrSimilar& stringINOUT, Tenum& enumINOUT, bool stringToEnum)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			if(stringToEnum)
			{
				const sCHAR conversion(stringINOUT); // helps compiler figure out
				return CodeStringToEnum(conversion.c_str(), enumINOUT);
			}
			else
			{
				stringINOUT = EnumToCodeString(enumINOUT);
				return true;
			}
		}

		// returns true if all tests pass
		static bool UnitTest(const bool testUniqueCodeStringsOnly = false)
		{
			//
			// do NOT put CENUMBINDER_AUTO_UNIT_TEST macro here !!!
			//
			const int num = GetSize();
			for(int i=0; i<num; ++i)
			{
				// ensures that code strings are unique (not terribly efficient method)
				for(int j=0; j<i; ++j){ // next two lines expanded to help in the debugger
					const sCHAR strOne(GetAt(j).m_stringCode);
					const sCHAR strTwo(GetAt(i).m_stringCode);
					if(UnitTestFailed(strOne != strTwo)){
						return false;
					}
				}

				// this can be turned off if the enums are NOT unique
				// special case where conversion is only one way and contains duplicate enums
				// (typically file reading/conversion of a partially supported list)
				// (e.g. to convert "apple" and "orange" strings both to your eFruit enum)
				if( ! testUniqueCodeStringsOnly)
				{
					// ensures that enum <- -> code string conversions are okay both ways
					const Tenum   thisItem   = GetAt(i).m_enum;
					const sCHAR thisString = GetAt(i).m_stringCode;
					
					sCHAR stringOut;
					Tenum   itemOut;
					if(  UnitTestFailed(EnumToCodeString(thisItem, stringOut))
					   ||UnitTestFailed(CodeStringToEnum(stringOut.c_str(),  itemOut))
					   ||UnitTestFailed(thisItem == itemOut)){
						return false;
					}

					if(  UnitTestFailed(CodeStringToEnum(thisString.c_str(), itemOut))
					   ||UnitTestFailed(EnumToCodeString(itemOut,  stringOut))
					   ||UnitTestFailed(thisString == stringOut)){
						return false;
					}
				}
			}
			return true; // passed tests
		}

		// =============================================================
		// CComboBox/Listbox functions 
		// NOTE: the typename CListBox_or_CComboBox should be an MFC:
		// 1) CComboBox 2) CListBox 3) something derived from these two
		// =============================================================
	#if defined(USE_MFC)
		template<typename CListBox_or_CComboBox>
		static bool DDX_Control(bool saveAndValidate, CListBox_or_CComboBox& listOrComboBox, Tenum& memberVariable)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			if(saveAndValidate){
				return GetCurSel(listOrComboBox, memberVariable);
			}
			else
			{
				if(0==listOrComboBox.GetCount()){ // first time through
					if( ! Populate(listOrComboBox)){
						return false; // problem adding the strings
					}
				}
				return SetSel(listOrComboBox, memberVariable);
			}
		}

		// just calls the other function
		template<typename CListBox_or_CComboBox>
		static bool DDX_Control(CDataExchange* pDX, CListBox_or_CComboBox& listOrComboBox, Tenum& memberVariable)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			return DDX_Control( (0!=pDX->m_bSaveAndValidate), listOrComboBox, memberVariable);
		}
	#endif 	
		// populates a CComboBox or CListBox (or derived classes)
		// returns false if not able to add a string (fairly rare case)
		template<typename CListBox_or_CComboBox>
		static bool Populate(CListBox_or_CComboBox& listOrComboBox)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			const int num = GetSize();

			listOrComboBox.ResetContent(); // clear out
			const int typicalMaxStrLength(70); // typical max string lengths in a combo or list box
			listOrComboBox.InitStorage(num, typicalMaxStrLength); // for faster filling
		
			ASSERT((LB_ERR==CB_ERR) && (LB_ERRSPACE==CB_ERRSPACE) && (LB_ERR==CB_ERR));
			for(int i=0; i<num; ++i)
			{
				const int addLocation = listOrComboBox.AddString(m_properties[i].m_stringUser);				
				if( (CB_ERR==addLocation) || (CB_ERRSPACE==addLocation) ){
					return false;
				}else{
					if(CB_ERR==listOrComboBox.SetItemData(addLocation, m_properties[i].m_enum)){
						return false;
					}
				}
			}
			return true;
		}

		// sets the selection in the CComboBox or CListBox (or derived classes) using the enum
		// returns false if not able to find the item (fairly rare case)
		template<typename CListBox_or_CComboBox>
		static bool SetSel(CListBox_or_CComboBox& listOrComboBox, const Tenum selectThisItem)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			const DWORD_PTR castEnum(selectThisItem);
			const int numItems(listOrComboBox.GetCount());
			for(int i=0; i<numItems; ++i){
				if(listOrComboBox.GetItemData(i) == castEnum){
					listOrComboBox.SetCurSel(i);
					return true;
				}
			}
			ASSERT(false); // did you call Populate first ?!?
			return false;  // did you add the strings to the combo using the Populate function?		
		}

		// combination function (to write one line instead of two in final application)
		template<typename CListBox_or_CComboBox>
		static bool PopulateSetSel(CListBox_or_CComboBox& listOrComboBox, const Tenum selectThisItem)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			return(Populate(listOrComboBox) && SetSel(listOrComboBox, selectThisItem) );
		}
		
		// converts the selection in the CComboBox or CListBox (or derived classes) to the enum
		// returns false if it could not transfer the selection (e.g. if no item is selected)
		template<typename CListBox_or_CComboBox>
		static bool GetCurSel(CListBox_or_CComboBox& listOrComboBox, Tenum& itemSelected)
		{
			CENUMBINDER_AUTO_UNIT_TEST;
			const int curSel = listOrComboBox.GetCurSel();
			if(CB_ERR!=curSel)
			{
				const DWORD_PTR curData(listOrComboBox.GetItemData(curSel));
				const int num = GetSize();
				for(int i=0; i<num; ++i)
				{
					const DWORD_PTR castEnum(m_properties[i].m_enum);
					if(castEnum == curData)
					{
						itemSelected = m_properties[i].m_enum;
						return true;
					}
				}
				ASSERT(false); // should have found it
			}
			return false; // no selection (which is not an error)
		}

	// =============================================================
	private:
		static bool m_autoUnitTestDone;
		static bool m_autoUnitTestUniqueCSOnly;
				
		// performs an automatic unit test for each bind-enum that is defined
		// 
		static void AutoUnitTest()
		{
			if( ! m_autoUnitTestDone)
			{
				m_autoUnitTestDone = true;
				UnitTest(m_autoUnitTestUniqueCSOnly);
			}
		}

		CEnumBinder() { } // only static functions - no construction for you!
		static Tproperties m_properties[]; // the array of data

		static bool IndexInRange(int& zeroBasedIndex)
		{
			if( (zeroBasedIndex>=0) && (zeroBasedIndex<GetSize()) ){
				return true; // okay
			}else{
				ASSERT(false); // trying to access out of range

	#ifdef CENUMBINDER_USE_EXCEPTIONS
				throw std::out_of_range("invalid subscript in CEnumBinder");
	#endif //CENUMBINDER_USE_EXCEPTIONS
				
				zeroBasedIndex = 0; // fix-up if not throwing exceptions
				// 
				// using wrong index (i.e. index 0) probably easier bug to track down
				// track down compared to corrupting random memory (i.e. prefer 
				// debug-build reproducible bug vs. often-unreproducible memory overruns)
				return false;
			}
		}

		// function should NOT fail if the enum is properly defined
		// (i.e. if UnitTest() passes, and all of enums passed to this function are in the list)
		static bool EnumToIndex(const Tenum enumItem, int& zeroBasedIndex_OUT)
		{
			// 1) assume enum is setup with first starting at zero, with all other values automatic
			// --> lookup will be constant time for this case
			const int castIndex(enumItem);
			if( (castIndex>=0) && (castIndex<GetSize()) ){
				if(enumItem == m_properties[castIndex].m_enum){
					zeroBasedIndex_OUT = castIndex;
					return true;
				}
			}
			// 2) enum is setup with non-standard index order
			// --> lookup will be O(n) for this case
			// --> could use a map (etc) to make it O(log(n)) but case 1, above, is preferred anyway
			const int num = GetSize();
			for(int i=0; i<num; ++i)
			{
				if(m_properties[i].m_enum == enumItem)
				{
					zeroBasedIndex_OUT = i;
					return true;
				}
			}

			int invalidIndexToFlagProblem(-1);
			IndexInRange(invalidIndexToFlagProblem);
			return false; // could not find the enum ?!?
		}

		static bool UnitTestFailed(const bool testPassed)
		{
			// all in a function to change behaviour in one spot (i.e. ASSERT or not)
			// Note: typically better to leave this assert - so you can examine the call stack
			// flip of pass/fail to avoid lots of ! in UnitTest code
			const bool testFailed( ! testPassed);
			ASSERT(testPassed); // test failed
			return testFailed;
		}
	};

	// use this template (along with the BIND_ENUM macro)
	// if you do not need to add extra members
	template <typename Tenum>
	class CEnumBinderData
	{
	public:
		Tenum m_enum;         // the enum
		sCHAR m_stringCode; // for writing to registry or files (i.e. does not change)
		sCHAR m_stringUser; // for the user (okay to change at any time)
	};


	// use this macro with BIND_ENUM_CUSTOM
	// to insert the required members into a custom class
	#define INSERT_ENUM_BINDER_ITEMS(originalEnumName) \
		public: \
			originalEnumName m_enum; \
			sCHAR m_stringCode; \
			sCHAR m_stringUser; \


	// first  parameter is the original enum name (already defined)
	// second parameter is the new class name (not yet defined - make it up)
	#define BIND_ENUM(originalEnumName, boundEnumClassName) \
		typedef CEnumBinderData< originalEnumName > Properties##originalEnumName; \
		typedef CEnumBinder< originalEnumName, Properties##originalEnumName > boundEnumClassName; \
		bool boundEnumClassName::m_autoUnitTestDone(false); \
		bool boundEnumClassName::m_autoUnitTestUniqueCSOnly(false); \
		Properties##originalEnumName boundEnumClassName::m_properties[] =


	// first  parameter is the original enum name (already defined)
	// second parameter is the new class name (not yet defined - make it up)
	// third  parameter is the custom binder class (already defined - with INSERT_ENUM_BINDER_ITEMS in it)
	#define BIND_ENUM_CUSTOM(originalEnumName,boundEnumClassName,binderDataClassName) \
		typedef CEnumBinder< originalEnumName, binderDataClassName > boundEnumClassName; \
		bool boundEnumClassName::m_autoUnitTestDone(false); \
		bool boundEnumClassName::m_autoUnitTestUniqueCSOnly(false); \
		binderDataClassName boundEnumClassName::m_properties[] =


	// these previous macros are an expansion of the following
	//
	// #define BIND_ENUM_CUSTOM(x,y,z) \
	// typedef CEnumBinderData< x > y; \
	// typedef CEnumBinder< x, y > z; \
	// y z::m_properties[] = 
	//
	// for example:
	// BIND_ENUM_CUSTOM(eShapeMode, ShapeModeProperties, ShapeModeEnum)
	//
	// which is, in turn, an expansion of the following:
	//
	// typedef CEnumBinderData< eShapeMode > ShapeModeProperties;
	// typedef CEnumBinder< eShapeMode, ShapeModeProperties > ShapeModeEnum;
	// ShapeModeProperties ShapeModeEnum::m_properties[] =

#undef DECL_ENUM_ELEMENT
#undef BEGIN_ENUM
#undef END_ENUM

#ifndef GENERATE_ENUM_STRINGS
	#define DECL_ENUM_ELEMENT( element ) element
	#define BEGIN_ENUM( ENUM_NAME ) typedef enum tag##ENUM_NAME
	#define END_ENUM( ENUM_NAME ) ENUM_NAME; TCHAR* GetString##ENUM_NAME(enum tag##ENUM_NAME index);
#else
	#define DECL_ENUM_ELEMENT( element ) #element
	#define BEGIN_ENUM( ENUM_NAME ) TCHAR* gs_##ENUM_NAME [] =
	#define END_ENUM( ENUM_NAME ) ; TCHAR* GetString##ENUM_NAME(enum tag##ENUM_NAME index){ return gs_##ENUM_NAME [index]; }
#endif


