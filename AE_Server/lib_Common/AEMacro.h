//---------------------------------------------------------
//						A4Macro.h
//---------------------------------------------------------

#ifndef __A4MACRO__H__
#define __A4MACRO__H__




//------------------------------------------------------------------------------
// 비트 마스킹 
//------------------------------------------------------------------------------
#ifndef BIT
	#define		BIT(x) (1 << (x))                       ///< Returns value with bit x set (2^x)
#endif

#ifndef BITMASK_CK
	#define BITMASK_CK( target, value )			(	(target) & (value)		)	// 비트 검사
#endif

#ifndef BITMASK_SET
	#define BITMASK_SET( target, value )		(	(target) |= (value)		)	// 비트 설정
#endif

#ifndef BITMASK_REMOVE
	#define BITMASK_REMOVE( target, value )		(	(target) &= ~(value)	)	// 비트 해제
#endif

#ifndef BITMASK_ZERO
	#define BITMASK_ZERO( target )				(	(target) = 0			)	// 전체비트 해제
#endif


#ifndef _countof
#	define _countof(a) (sizeof(a) / sizeof((a)[0]))
#endif // _countof

#ifndef _size_to_dword
#	define _size_to_dword(n) (((n) - 1) / 32 + 1)
#endif // _size_to_dword

#ifndef _size_to_byte
#	define _size_to_byte(n) (((n) - 1) / 8 + 1)
//#	define _size_to_byte(n) (((n) - 1) << 1)

#endif // _size_to_byte

//------------------------------------------------------------------------------
// 메모리 삭제 
//------------------------------------------------------------------------------
#ifndef SAFE_DELETE_ARRAY
	#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    

//---------------------------------------------------------
//		
//---------------------------------------------------------
#define GET_OUTER_OBJ(outer_class, this_var)								\
		outer_class& GetOuter() { return * (outer_class*)					\
        (((PBYTE) this) - (PBYTE) (&((outer_class*) NULL)->this_var)); }	\
		outer_class* GetOuterSafe() { return this ? &GetOuter() : NULL; }

#define getDWORD(buf,bit,mask) ((*(DWORD*)(((BYTE*)buf)+((bit)>>3)))>>((bit)&7)&(mask))
#define getWORD(buf,bit,mask) ((*(WORD*)(((BYTE*)buf)+((bit)>>3)))>>((bit)&7)&(mask))

#endif 
