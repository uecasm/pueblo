/*----------------------------------------------------------------------------
                        _                              _ _       
        /\             | |                            | (_)      
       /  \   _ __   __| |_ __ ___  _ __ ___   ___  __| |_  __ _ 
      / /\ \ | '_ \ / _` | '__/ _ \| '_ ` _ \ / _ \/ _` | |/ _` |
     / ____ \| | | | (_| | | | (_) | | | | | |  __/ (_| | | (_| |
    /_/    \_\_| |_|\__,_|_|  \___/|_| |_| |_|\___|\__,_|_|\__,_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo client code, released November 4, 1998.

    The Initial Developer of the Original Code is Andromedia Incorporated.
	Portions created by Andromedia are Copyright (C) 1998 Andromedia
	Incorporated.  All Rights Reserved.

	Andromedia Incorporated                         415.365.6700
	818 Mission Street - 2nd Floor                  415.365.6701 fax
	San Francisco, CA 94103

    Contributor(s):
	--------------------------------------------------------------------------
	   Chaco team:  Dan Greening, Glenn Crocker, Jim Doubek,
	                Coyote Lussier, Pritham Shetty.

					Wrote and designed original codebase.

------------------------------------------------------------------------------

	This file contains the definitions for Chaco common data types.

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHTYPES_H )
#define _CHTYPES_H

#if defined( _WINDOWS )
	#if !defined( CH_MSW )
		#define CH_MSW						// Automatically define CH_MSW
	#endif
#endif

#if defined( CH_MSW )
	#include <afxwin.h>         			// MFC core and standard components

	#if !defined( CH_DEBUG ) && defined( _DEBUG )
		#define CH_DEBUG
	#endif

#elif defined( CH_UNIX )
#ifdef CH_CLIENT
	#include <X11/Xlib.h>
#endif // CH_CLIENT
	#include <String.h>
	#include <stdio.h>
	#include <sys/param.h>
	#include <string.h>
	#include <stdarg.h>
#ifndef __GNUC__
	typedef int bool;
	#define true 1
	#define false 0
#endif

#endif

#if defined( CH_MSW )
											// Windows platforms
	#if defined( WIN32 )
		#if !defined( CH_ARCH_32 )
			#define CH_ARCH_32
		#endif
	#else
		#if !defined( CH_ARCH_16 )
			#define CH_ARCH_16
		#endif                

		#if !defined( NO_TEMPLATES )
			#define NO_TEMPLATES
		#endif
	#endif
#elif defined( CH_UNIX )
											// Unix platforms
	#if !defined( CH_ARCH_32 )
		#define CH_ARCH_32
	#endif
#endif


#if defined( CH_MSW )
    
    #if defined( CH_ARCH_32 )
	#define DLL_EXPORT		__declspec( dllexport )
	#define DLL_IMPORT		__declspec( dllimport )
	#else
	#define DLL_EXPORT			
	#define DLL_IMPORT		
	#endif

											// Windows platforms
	#if !defined( RC_INVOKED )

		#if defined(CH_ARCH_16)

			#if !defined( NEAR )
				#define NEAR   __near
			#endif

			#if !defined( FAR )
				#define FAR    __far
			#endif

			#if !defined( HUGE )
				#define HUGE   _huge
			#endif

			#if !defined( CDECL )
				#define CDECL  __cdecl
			#endif

			#if !defined( EXPORT )
				#define EXPORT _export
			#endif

			#define API              FAR __pascal
			#define API_VARARGS      FAR CDECL
			#if !defined( CALLBACK )
				#define CALLBACK		FAR __pascal
			#endif
			#define CALLBACK_VARARGS FAR CDECL

			#undef  INT_ENDIAN_HI_LO
			#undef  INT_STRICT_ALIGNMENT

		#elif defined(CH_ARCH_32)

			#if !defined( WIN32 )
				#define WIN32
			#endif

			#if !defined( NEAR )
				#define NEAR
			#endif
			#if !defined( FAR )
				#define FAR
			#endif
			#if !defined( HUGE )
				#define HUGE
			#endif
			#if !defined( CDECL )
				#define CDECL			__cdecl
			#endif
			#if !defined( EXPORT )
				#define EXPORT
			#endif

			#define API					__stdcall
			#define API_VARARGS			CDECL
			#define VARARGS				CDECL
			#define CALLBACK			__stdcall
			#define CALLBACK_VARARGS	CDECL

		#endif   /* platforms */

	#endif  /* !defined( RC_INVOKED ) */

#elif defined( CH_UNIX )

	#define DLL_EXPORT
	#define DLL_IMPORT
											// Unix platforms
	#if !defined( NEAR )
		#define NEAR
	#endif

	#if !defined( FAR )
		#define FAR
	#endif

	#if !defined( CDECL )
		#define CDECL
	#endif

	#if !defined( EXPORT )
		#define EXPORT
	#endif

	#define API
	#define API_VARARGS

	#if !defined( CALLBACK )
		#define CALLBACK
	#endif

	#define CALLBACK_VARARGS

	#undef INT_ENDIAN_HI_LO
	#undef INT_STRICT_ALIGNMENT

	#define AFXAPI

#endif

/*----------------------------------------------------------------------------
	Constants:

	Define the standard constant values
----------------------------------------------------------------------------*/

#define CH_SYS_UNKNOWN			0
#define CH_SYS_WIN3X			1
#define CH_SYS_WIN32S			2
#define CH_SYS_WIN95			3
#define CH_SYS_WINNT			4

#define CH_SYS_UNIX				20

#define CH_PROP_WIN95			(1L << 0)
#define CH_PROP_LONG_FILENAMES	(1L << 1)
#define CH_PROP_MULTITHREADED	(1L << 2)


/*----------------------------------------------------------------------------
	Basic Machine Dependent Types:

	Define the basic machine dependent type definitions
----------------------------------------------------------------------------*/

#if !defined( RC_INVOKED )

typedef signed char     chint8;
typedef unsigned char   chuint8;

typedef signed short    chint16;
typedef unsigned short  chuint16;

typedef signed long     chint32;
typedef unsigned long   chuint32;

typedef unsigned int    chuint;

typedef char*			pstr;

typedef void FAR*		ptr;



/*----------------------------------------------------------------------------
	Basic Derived Types:

	Define the basic derived type definitions
----------------------------------------------------------------------------*/

typedef chuint32		chparam;

typedef chuint16		chflag16;
typedef chuint32		chflag32;

typedef chuint32		ChModuleID;

#if defined( CH_MSW )

	typedef CWnd			ChWnd;

#endif	// defined( CH_MSW )

#endif	// !defined( RC_INVOKED )


/*----------------------------------------------------------------------------
	Macros:
----------------------------------------------------------------------------*/

#if !defined( RC_INVOKED )

#if defined( __cplusplus )
											/* Special 'extern' for C++ to
												avoid name mangling... */
	#ifdef CH_MSW
//		#define EXTERN			extern "C"
		#define CH_EXTERN			extern
	#else
		#define CH_EXTERN			extern
	#endif
	#define C_NAMING		extern "C"

#else	// defined( __cplusplus )

	#define CH_EXTERN			extern
	#define C_NAMING

#endif	// defined( __cplusplus )


#define CH_STATIC_VAR		static
#define CH_INTERN_VAR		static
#define CH_GLOBAL_VAR
#define CH_EXTERN_VAR		extern

#if !defined( TEXT )

	#define	TEXT( a )		a

#endif	// !defined( TEXT )

#define CH_INTERN_FUNC( retType )			static retType NEAR
#define CH_INTERN_FUNC_C( retType )			static retType NEAR

#define CH_GLOBAL_FUNC( retType )			retType
#define CH_EXTERN_FUNC( retType )			CH_EXTERN retType

#define CH_GLOBAL_LIBRARY_C( retType )		C_NAMING DLL_EXPORT retType API EXPORT
#define CH_EXTERN_LIBRARY_C( retType )		CH_EXTERN DLL_IMPORT retType API EXPORT

#define CH_GLOBAL_CALLBACK( retType )		retType CALLBACK EXPORT
#define CH_EXTERN_CALLBACK( retType )		CH_EXTERN retType CALLBACK EXPORT

#if defined( CH_MSW )
	#define CH_GLOBAL_CALLBACK_C( retType )		retType FAR __cdecl EXPORT
	#define CH_EXTERN_CALLBACK_C( retType )		CH_EXTERN retType FAR __cdecl EXPORT  
// UE: the logic seems to have mysteriously been reversed in the following test;
//     which caused it to erroneously think the entrypoint was __stdcall in the
//     main program and __cdecl in the modules themselves, causing the stack to
//     accumulate too much crap and corrupting local variables.
	#if !defined( CH_ARCH_32 )
#error Unexpected non-arch32!
	#define CH_GLOBAL_LIBRARY( retType )		C_NAMING DLL_EXPORT retType CDECL EXPORT
	#define CH_EXTERN_LIBRARY( retType )		CH_EXTERN DLL_IMPORT retType CDECL EXPORT  
	#else
	#define CH_GLOBAL_LIBRARY( retType )		C_NAMING DLL_EXPORT retType API EXPORT
	#define CH_EXTERN_LIBRARY( retType )		C_NAMING CH_EXTERN DLL_IMPORT retType API EXPORT  
	#endif
#else
	#define CH_GLOBAL_CALLBACK_C( retType )		retType FAR EXPORT
	#define CH_EXTERN_CALLBACK_C( retType )		CH_EXTERN retType FAR EXPORT
	#define CH_GLOBAL_LIBRARY( retType )		C_NAMING DLL_EXPORT retType CDECL EXPORT
	#define CH_EXTERN_LIBRARY( retType )		CH_EXTERN DLL_IMPORT retType CDECL EXPORT
#endif

#define CH_TYPEDEF_FUNC( retType, typeName ) \
                typedef retType (*typeName)

#define CH_TYPEDEF_FUNC_C( retType, typeName ) \
                typedef retType (*typeName)

#define CH_TYPEDEF_LIBRARY( retType, typeName ) \
                typedef retType (API *typeName)

#define CH_TYPEDEF_LIBRARY_C( retType, typeName ) \
                typedef retType (API *typeName)

#define CH_TYPEDEF_CALLBACK( retType, typeName ) \
                typedef retType (CALLBACK EXPORT *typeName)

#define CH_TYPEDEF_CALLBACK_C( retType, typeName ) \
                typedef retType (FAR __cdecl EXPORT *typeName)

#define CH_TYPEDEF_INTERN_FUNC( retType, typeName ) \
                typedef retType (NEAR *typeName)

#define CH_TYPEDEF_INTERN_FUNC_C( retType, typeName ) \
                typedef retType (NEAR *typeName)

#if defined( CH_MSW )
											// MSW-specific definitions
	typedef CString			ChString;
	typedef CRect			ChRect;
	typedef CPoint			ChPoint;
	typedef CFont			ChFont;
	typedef CSize			ChSize;
	typedef CScrollView		ChScrollView;
	typedef CClientDC		ChClientDC;
 #if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x0500)) || (defined(_MSC_VER) && (_MSC_VER >= 1200))
	// bool is defined by the compiler
 #else
	typedef BOOL			bool;  

	#define true		1
	#define false		0
 #endif

	#if defined( CH_ARCH_16 )

		#define ChMemClear( pDest, lLen )			_fmemset( (void *)(pDest), 0, ((size_t)(lLen)) )
		#define ChMemClearStruct( pDest )			_fmemset( (void *)(pDest), 0, sizeof( *(pDest) ) )
		#define ChMemCopy( pDest, pSrc, lCount )	_fmemcpy( (void *)(pDest), (void *)(pSrc), (chuint16)(lCount) )
		#define ChMemMove( pDest, pSrc, lCount )	_fmemmove( (void *)(pDest), (void *)(pSrc), (chuint16)(lCount) )
		#define ChMemCmp(pCmp1,pCmp2,lLen) 			_fmemcmp( (void*)pCmp1, (void*)pCmp2, (chuint16)lLen)
		#define DeleteFile(foo) 					_unlink(foo)
		#define CreateDirectory(dirname,bar) 		_mkdir(dirname,0755)
		#if !defined( MAX_PATH )
		#define MAX_PATH		256
		#endif
	#else	// defined( CH_ARCH_16 )

		#define ChMemClear( pDest, lLen )			RtlZeroMemory( ((void *)(pDest)), (lLen) )
		#define ChMemClearStruct( pDest )			RtlZeroMemory( ((void *)(pDest)), sizeof( *(pDest) ) )
		#define ChMemCopy( pDest, pSrc, lCount )	RtlCopyMemory( ((void *)(pDest)), ((void *)(pSrc)), (lCount) )
		#define ChMemMove( pDest, pSrc, lCount )	memmove( (void *)(pDest), (void *)(pSrc), (int)(lCount) )
		#define ChMemCmp(pCmp1,pCmp2,lLen) 			memcmp( (void*)pCmp1, (void*)pCmp2, lLen)
	#endif	// defined( CH_ARCH_16 )

	#define CH_PLATFORM "MS Windows"
    
    #if defined( CH_ARCH_16 )
	#define CH_EXPORT_CLASS		
    #else
	#if !defined(CH_STATIC_LINK)
		#define CH_EXPORT_CLASS		AFX_EXT_CLASS
	#else
		#define CH_EXPORT_CLASS		
	#endif
	#endif

#elif defined( CH_UNIX )
											// Unix-specific definitions
	typedef bool		BOOL;

class ChString : public String
{
  private:
	char *buffer;
  public:
	ChString() : String(), buffer(0) { };
	ChString(const char* t) : String(t), buffer( 0 ) { };
	ChString(const char* t, int len) : String( t, len ), buffer( 0 ) { };
	ChString(const ChString& x) : String(x), buffer( 0 ) { };
	ChString(const SubString& x) : String(x), buffer( 0 ) { };
	ChString(char c) : String(c), buffer( 0 ) { };

	ChString& operator =  (const ChString& y) { String::operator=(y); return *this; };
	ChString& operator =  (const String& y) { String::operator=(y); return *this; };
	ChString& operator =  (const char* y) { String::operator=(y); return *this; };
	ChString& operator =  (char c) { String::operator=(c); return *this; };
	ChString& operator =  (const SubString& y) { String::operator=(y); return *this; };
	
	// New functions, defined for MSW CString compatibility:
	ChString Left(int len) { return at(0, len); };
	ChString Left(int len) const {
		ChString temp = *this;
		return temp.at(0, len); 
	};
	ChString Mid(int start, int end = -1) { 
		if (end == -1)
			return from( start );
		else
			return at( start, end );
	}
	int GetLength() const { return length(); };
	int Find(char c, int startpos = 0) const { return index( c, startpos ); };
	int Find(const String& y, int startpos = 0) const { return index( y, startpos ); };
	int Find(const SubString& y, int startpos = 0) const { return index( y, startpos ); };
	int Find(const char* t, int startpos = 0) const { return index( t, startpos ); };

	int ReverseFind(char c) const {
		char *cp = strrchr( chars(), c );
		if (cp)
			return (cp - chars());
		else
			return -1;
	};
//	int ReverseFind(const String& y) const { return rindex( y ); };
//	int ReverseFind(const SubString& y) const { return rindex( y ); };
//	int ReverseFind(const char* t) const { return rindex( chars(), t ); };
	ChString Right(int pos)  { return after(pos); };
	int CompareNoCase(const ChString& s) const { return fcompare( *this, s ); };
	void MakeLower() { downcase(); };
	int IsEmpty() const { return empty(); };
	void Empty() { *this = ""; };
	void Format( const char *strFormat, ... ) {
		char temp[1024];
		va_list ap;

		va_start(ap, strFormat);
		vsprintf( temp, strFormat, ap );
		va_end(ap);
		*this = (char *)temp;
	}
	void TrimLeft(void) { *this = Left( Find( ' ' ) ); };
	void TrimRight(void) { *this = Right( ReverseFind( ' ' ) ); };
	void LoadString(int resource_val) { *this = "resource string"; };
	void SetAt(int pos, char newchar) { (*this)[pos] = newchar; };
	char *GetBuffer(int newlen) { 
		buffer = new char[newlen];
		return buffer;
	}
	void ReleaseBuffer(void) { 
		if (buffer) {
			*this = buffer;
			buffer = 0;
		}
	};
};
#ifdef CH_CLIENT
	typedef chuint32 COLORREF; // XXX
	// BGR should set some sort of flag on the color to indicate that
	// it's a background color.  XXX
	#define BGR(foo) (foo)
	typedef XFontStruct *LOGFONT; // XXX
	typedef chuint32 CREATESTRUCT;
	typedef void **HCURSOR; // XXX
#endif // CH_CLIENT

	#if defined( CH_DEBUG )
		#define ASSERT( f )	assert( f )		// ASSERT is defined if debug mode
	#else	// defined( CH_DEBUG )
		#define ASSERT( f )
	#endif	// defined( CH_DEBUG )

	#define VERIFY( f )	assert( f )			// VERIFY is always defined

	#define TRACE( foo ) fprintf(stderr, foo)
	#define TRACE1( foo, bar ) fprintf(stderr, foo, bar)
	#define TRACE2( foo, bar, baz ) fprintf(stderr, foo, bar, baz)
	#define lstrlen(foo) strlen(foo)
	#define lstrcat(foo,bar) strcat(foo,bar)
	#define lstrcmpi(foo,bar) strcasecmp(foo,bar)
	#define lstrcmp(foo,bar) strcmp(foo,bar)
	#define lstrcpy(foo,bar) strcpy(foo,bar)
	#define wsprintf sprintf
	#define ltoa(num, str, len) sprintf( (str), "%d", (num) )

	#define DeleteFile(foo) unlink(foo)
	#define CreateDirectory(dirname,bar) (!mkdir((dirname),0755))
	#define _filelength( foo )	filelength( foo )

	#define ChMemCopy(pDest,pSrc,lLen) memcpy(pDest,pSrc,lLen)
	#define ChMemCmp(pDest,pSrc,lLen) memcmp(pCmp1,pCmp2,lLen)
	#define ChMemClear( pDest, lLen ) memset( (void *)(pDest), 0, ((size_t)(lLen)) )
	#define ChMemClearStruct( pDest ) ChMemClear( pDest, sizeof( *(pDest) ) )
	#define ChMemMove( pDest, pSrc, lCount )	memmove( (void *)(pDest), (void *)(pSrc), (int)(lCount) )

	#ifdef __linux__
	#define CH_PLATFORM "Linux"
	#else
	#define CH_PLATFORM "Solaris"
	#endif

	#define CH_EXPORT_CLASS					// Does nothing on UNIX

	#define MAX_PATH MAXPATHLEN

	typedef void **HANDLE;
	typedef int *LPCRITICAL_SECTION;
	typedef int CRITICAL_SECTION;

	typedef chuint32 UINT;
	typedef void *LPVOID;
	typedef void *LPCREATESTRUCT;
	typedef char *LPCTSTR;
	typedef char *LPSTR;

	typedef chuint8 BYTE;
	typedef chuint16 WORD;
	typedef chuint32 DWORD;

	extern chuint32 _filelength(int fd);

	#define DECLARE_SERIAL(foo)
	#define IMPLEMENT_SERIAL(class, baseclass, schema)

#endif	// Platform-specific definitions

#endif /* !defined( RC_INVOKED ) */
#endif /* !defined( _CHTYPES_H ) */

// Local Variables: ***
// tab-width:4 ***
// End: ***

// $Log$
// Revision 1.1.1.1  2003/02/03 18:56:01  uecasm
// Import of source tree as at version 2.53 release.
//
