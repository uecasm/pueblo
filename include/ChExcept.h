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

	This file consists of the interfaces for the ChExcept class and
	derived classes.

----------------------------------------------------------------------------*/

#if !defined( _CHEXCEPT_H )
#define _CHEXCEPT_H

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif

#if defined( CH_EXCEPTIONS )

#include <ChTypes.h>


/*----------------------------------------------------------------------------
	ChEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChEx
				#if defined( CH_MSW ) && defined( CH_ARCH_16 )
				: public CException
				#endif
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChEx)
	#endif
	public:
		enum {	undefined = 0,			// Undefined exception
				endOfData,				// End of ChData hit while reading
				badSeek,				// A bad Seek operation occurred
				outOfMemory,			// Out of memory error
				connectFailed,			// 'connect' operation failed
				socketTimedOut,			// 'connect' operation timed out
				hostNotFound,			// The host name could not be found
				inProgress,				// Blocking call in progress
				last = 10000			// Always last exception defined
			};

	public:
		virtual ~ChEx() {}
		virtual chint16 GetCause() const { return m_sCause; }


	protected:
		ChEx( chint16 sCause = undefined ) : m_sCause( sCause ) {}

	protected:
		chint16		m_sCause;
};


/*----------------------------------------------------------------------------
	ChArchiveEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChArchiveEx : public ChEx
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChArchiveEx)         
	#endif
	public:
		ChArchiveEx( chint16 sCause = undefined ) : ChEx( sCause ) {}
		virtual ~ChArchiveEx() {}
};


/*----------------------------------------------------------------------------
	ChDataEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChDataEx : public ChEx
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChDataEx)
	#endif
	public:
		ChDataEx( chint16 sCause = undefined ) :
				ChEx( sCause ) {}
		virtual ~ChDataEx() {}
};


/*----------------------------------------------------------------------------
	ChMemEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChMemEx : public ChEx
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChMemEx)
	#endif
	public:
		ChMemEx() : ChEx( outOfMemory ) {}
		virtual ~ChMemEx() {}
};


/*----------------------------------------------------------------------------
	ChSocketEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChSocketEx : public ChEx
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChSocketEx)      
	#endif
	public:
		ChSocketEx( chint16 sCause = undefined ) : ChEx( sCause ) {}
		virtual ~ChSocketEx() {}
};
/*----------------------------------------------------------------------------
	ChJPEGEx class
----------------------------------------------------------------------------*/

class CH_EXPORT_CLASS ChJPEGEx : public ChEx
{
	#if defined( CH_MSW ) && defined( CH_ARCH_16 )
	DECLARE_DYNAMIC(ChJPEGEx)      
	#endif
	public:
		ChJPEGEx( chint16 sCause = undefined, const char *reason = "" ) : ChEx( sCause ), Reason(reason) {}
		virtual ~ChJPEGEx() {}
	public:
		enum {	undefined = 0,			// Undefined exception
				badAlignType,			// ALIGN_TYPE is wrong, please fix
				badAllocChunk,			// MAX_ALLOC_CHUNK is wrong, please fix
				badBufferMode,			// Bogus buffer control mode
				badComponentID,			// Invalid component ID %d in SOS
				badDCTSize,				// IDCT output block size %d not supported
				badInColorspace,		// Bogus input colorspace
				badJColorSpace,			// Bogus JPEG colorspace
				badLength,				// Bogus marker length
				badMCUSize,				// Sampling factors too large for interleaved scan
				badPoolID,				// Invalid memory pool code
				badPrecision,			// Unsupported JPEG data precision
				badSampling,			// Improper call to JPEG library in state
				badVirualAccess,		// Bogus virtual array access
				bufferSize,				// Buffer passed to JPEG library is too small
				cannotSuspend,			// Suspension not allowed here
				CCIR601NotImpl,			// CCIR601 sampling not implemented yet
				componentCount,			// Too many color components
				conversionNotImpl,		// Unsupported color conversion request
				bogusDACIndex,			// Bogus DAC index
				bogusDACValue,			// Bogus DAC value
				bogusDHTCounts,			// Bogus DHT counts
				bogusDHTIndex,			// Bogus DHT index 
				bogusDQTIndex,			// Bogus DQT index
				emptyImage,				// Empty JPEG image (DNL not supported)
				eoiExpected,			// Didn't expect more than one scan
				fileRead,				// Input file read error
				fileWrite,				// Output file write error --- out of disk space?
				fractSamplMotImpl,		// Fractional sampling not implemented yet
				huffClenOverflow,		// Huffman code size table overflow
				huffMissingCode,		// Missing Huffman code table entry
				imageTooBig,			// Maximum supported image dimension is %u pixels
				inputEmpty,				// Empty input file
				inputEOF,				// Premature end of input file
				JFIFMajor,				// Unsupported JFIF revision number
				notImplemented,			// Not implemented yet
				notCompiled,			// Requested feature was omitted at compile time
				noBackingStore,			// Backing store not supported
				noHUFFTable,			// Huffman table  was not defined
				noImage,				// JPEG datastream contains no image
				noQuantTable,			// Quantization table 0x%02x was not defined
				noSOI,					// Not a JPEG file
				outOfMEmory,  			// Insufficient memory 
				quantComponents,		// Cannot quantize more than %d color components
				quantFewColors,			// Cannot quantize to fewer than %d colors
				quantMAnyColors,		// Cannot quantize to more than %d colors
				SOFDuplicate,			// Invalid JPEG file structure: two SOF markers
				SOFNoSOS,				// Invalid JPEG file structure: missing SOS marker
				SOFUnsupported,			// Unsupported JPEG process
				SOIDuplicate,			// Invalid JPEG file structure
				SOSNoSOF,				// Invalid JPEG file structure
				tfileCreate,			// Failed to create temporary file 
				tfileRead,				// Read failed on temporary file
				tfileSeek,				// Seek failed on temporary file
				tfileWrite,				// Write failed on temporary file --- out of disk space?
				tooLittleData,			// Application transferred too few scanlines
				unknownMarker,			// Unsupported marker
				virtualBug,				// Virtual array controller messed up
				widthOverflow,			// Image too wide for this implementation
				inputFileOpenError,		// unable to open input file
				last = 10000			// Always last exception defined
			};

		ChString Reason;
};

#else	// defined( CH_EXCEPTIONS )

#define try
#define catch( foo )

#endif	// defined( CH_EXCEPTIONS )

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// !defined( _CHEXCEPT_H )
