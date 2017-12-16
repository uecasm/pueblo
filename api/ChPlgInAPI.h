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

	This file contains the interface of plugin entry points.

----------------------------------------------------------------------------*/

/*
 *  npapi.h
 *  Netscape client plug-in API spec
 */

#ifndef _NPAPI_H_
#define _NPAPI_H_

#include "jri/jri.h"		/* Java Runtime Interface */


/*
 *  Version constants
 */

#define NP_VERSION_MAJOR 0
#define NP_VERSION_MINOR 9



/*----------------------------------------------------------------------*/
/*                   Definition of Basic Types                          */
/*----------------------------------------------------------------------*/
 
#ifndef _UINT16
typedef unsigned short uint16;
#endif
#ifndef _UINT32
#if defined(__alpha)
typedef unsigned int uint32;
#else /* __alpha */
typedef unsigned long uint32;
#endif /* __alpha */
#endif
#ifndef _INT16
typedef short int16;
#endif
#ifndef _INT32
#if defined(__alpha)
typedef int int32;
#else /* __alpha */
typedef long int32;
#endif /* __alpha */
#endif

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef NULL
#define NULL (0L)
#endif

typedef unsigned char	NPBool;
typedef void*			NPEvent;
typedef int16			NPError;
typedef int16			NPReason;
typedef char*			NPMIMEType;

/*----------------------------------------------------------------------*/
/*                   Structures and definitions                         */
/*----------------------------------------------------------------------*/

/*
 *  NPP is a plug-in's opaque instance handle
 */
typedef struct _NPP
{
    void*	pdata;			/* plug-in private data */
    void*	ndata;			/* netscape private data */
} NPP_t;

typedef NPP_t*  NPP;


typedef struct _NPStream
{
    void*		pdata;		/* plug-in private data */
    void*		ndata;		/* netscape private data */
    const char*		url;
    uint32		end;
    uint32		lastmodified;
    void*		notifyData;
} NPStream;


typedef struct _NPByteRange
{
    int32	offset;			/* negative offset means from the end */
    uint32	length;
    struct _NPByteRange* next;
} NPByteRange;


typedef struct _NPSavedData
{
    int32	len;
    void*	buf;
} NPSavedData;


typedef struct _NPRect
{
    uint16	top;
    uint16	left;
    uint16	bottom;
    uint16	right;
} NPRect;


typedef struct _NPWindow 
{
    void*	window;		/* Platform specific window handle */
    uint32	x;			/* Position of top left corner relative */
    uint32	y; 			/*	to a netscape page.					*/
    uint32	width;		/* Maximum window size */
    uint32	height;
    NPRect	clipRect;	/* Clipping rectangle in port coordinates */
						/* Used by MAC only.                      */
} NPWindow;


typedef struct _NPFullPrint
{
    NPBool	pluginPrinted;	/* Set TRUE if plugin handled fullscreen */
							/*	printing							 */
    NPBool	printOne;		/* TRUE if plugin should print one copy  */
							/*	to default printer					 */
    void*	platformPrint;	/* Platform-specific printing info */
} NPFullPrint;

typedef struct _NPEmbedPrint
{
    NPWindow	window;
    void*	platformPrint;	/* Platform-specific printing info */
} NPEmbedPrint;

typedef struct _NPPrint
{
    uint16	mode;						/* NP_FULL or NP_EMBED */
    union
    {
		NPFullPrint		fullPrint;		/* if mode is NP_FULL */
		NPEmbedPrint	embedPrint;		/* if mode is NP_EMBED */
    } print;
} NPPrint;


/*
 * Values for mode passed to NPP_New:
 */
#define NP_EMBED		1
#define NP_FULL			2

/*
 * Values for stream type passed to NPP_NewStream:
 */
#define NP_NORMAL		1
#define NP_SEEK			2
#define NP_ASFILE		3
#define NP_ASFILEONLY		4

#define NP_MAXREADY	(((unsigned)(~0)<<1)>>1)



/*----------------------------------------------------------------------*/
/*                   Error and Reason Code definitions                  */
/*----------------------------------------------------------------------*/

/*
 *	Values of type NPError:
 */
#define NPERR_BASE							0
#define NPERR_NO_ERROR						(NPERR_BASE + 0)
#define NPERR_GENERIC_ERROR					(NPERR_BASE + 1)
#define NPERR_INVALID_INSTANCE_ERROR		(NPERR_BASE + 2)
#define NPERR_INVALID_FUNCTABLE_ERROR		(NPERR_BASE + 3)
#define NPERR_MODULE_LOAD_FAILED_ERROR		(NPERR_BASE + 4)
#define NPERR_OUT_OF_MEMORY_ERROR			(NPERR_BASE + 5)
#define NPERR_INVALID_PLUGIN_ERROR			(NPERR_BASE + 6)
#define NPERR_INVALID_PLUGIN_DIR_ERROR		(NPERR_BASE + 7)
#define NPERR_INCOMPATIBLE_VERSION_ERROR	(NPERR_BASE + 8)
#define NPERR_INVALID_PARAM 				(NPERR_BASE + 9)
#define NPERR_INVALID_URL 					(NPERR_BASE + 10)
#define NPERR_FILE_NOT_FOUND 				(NPERR_BASE + 11)
#define NPERR_NO_DATA		 				(NPERR_BASE + 12)
#define NPERR_STREAM_NOT_SEEKABLE			(NPERR_BASE + 13)

/*
 *	Values of type NPReason:
 */
#define NPRES_BASE                      	0
#define NPRES_DONE			               	(NPRES_BASE + 0)
#define NPRES_NETWORK_ERR               	(NPRES_BASE + 1)
#define NPRES_USER_BREAK                	(NPRES_BASE + 2)

/*
 *	Don't use these obsolete error codes any more.
 */
 #define NP_NOERR  NP_NOERR_is_obsolete_use_NPERR_NO_ERROR
 #define NP_EINVAL NP_EINVAL_is_obsolete_use_NPERR_GENERIC_ERROR
 #define NP_EABORT NP_EABORT_is_obsolete_use_NPRES_USER_BREAK

/*
 * Version feature information
 */
#define NPVERS_HAS_STREAMOUTPUT		8
#define NPVERS_HAS_NOTIFICATION		9
#define NPVERS_HAS_LIVECONNECT		9




/*----------------------------------------------------------------------*/
/*                   Function Prototypes                                */
/*----------------------------------------------------------------------*/

#if defined(_WINDOWS) && !defined(WIN32)
#define NP_LOADDS  _loadds
#else
#define NP_LOADDS
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NP_EXPORT _stdcall
/*
 * NPP_* functions are provided by the plugin and called by the navigator.
 */

NPError               	NPP_Initialize(void);
void                  	NPP_Shutdown(void);
NPError     NP_LOADDS	NPP_New(NPMIMEType pluginType, NPP instance,
								uint16 mode, int16 argc, char* argn[],
								char* argv[], NPSavedData* saved);
NPError     NP_LOADDS	NPP_Destroy(NPP instance, NPSavedData** save);
NPError     NP_LOADDS	NPP_SetWindow(NPP instance, NPWindow* window);
NPError     NP_LOADDS	NPP_NewStream(NPP instance, NPMIMEType type,
									  NPStream* stream, NPBool seekable,
									  uint16* stype);
NPError     NP_LOADDS	NPP_DestroyStream(NPP instance, NPStream* stream,
										  NPReason reason);
int32       NP_LOADDS	NPP_WriteReady(NPP instance, NPStream* stream);
int32       NP_LOADDS	NPP_Write(NPP instance, NPStream* stream, int32 offset,
								  int32 len, void* buffer);
void        NP_LOADDS	NPP_StreamAsFile(NPP instance, NPStream* stream,
										 const char* fname);
void        NP_LOADDS	NPP_Print(NPP instance, NPPrint* platformPrint);
int16                 	NPP_HandleEvent(NPP instance, void* event);
void                 	NPP_URLNotify(NPP instance, const char* url,
									  NPReason reason, void* notifyData);
jref					NPP_GetJavaClass(void);


/*
 * NPN_* functions are provided by the navigator and called by the plugin.
 */
 
void        	CHPN_Version(int* plugin_major, int* plugin_minor,
							int* netscape_major, int* netscape_minor);
NPError     	CHPN_GetURLNotify(NPP instance, const char* url,
								 const char* target, void* notifyData);
NPError     	CHPN_GetURL(NPP instance, const char* url,
						   const char* target);
NPError     	CHPN_PostURLNotify(NPP instance, const char* url,
								  const char* target, uint32 len,
								  const char* buf, NPBool file,
								  void* notifyData);
NPError     	CHPN_PostURL(NPP instance, const char* url,
							const char* target, uint32 len,
							const char* buf, NPBool file);
NPError     	CHPN_RequestRead(NPStream* stream, NPByteRange* rangeList);
NPError     	CHPN_NewStream(NPP instance, NPMIMEType type,
							  const char* target, NPStream** stream);
int32       	CHPN_Write(NPP instance, NPStream* stream, int32 len,
						  void* buffer);
NPError    		CHPN_DestroyStream(NPP instance, NPStream* stream,
								  NPReason reason);
void        	CHPN_Status(NPP instance, const char* message);
const char* 	CHPN_UserAgent(NPP instance);
void*       	CHPN_MemAlloc(uint32 size);
void        	CHPN_MemFree(void* ptr);
uint32      	CHPN_MemFlush(uint32 size);
void			CHPN_ReloadPlugins(NPBool reloadPages);
JRIEnv*			CHPN_getJavaEnv(void);
jref			CHPN_getJavaPeer(NPP instance);


#ifdef __cplusplus
}  /* end extern "C" */
#endif


/* NPP_Initialize */


typedef void (*NPP_InitializeUPP)(void);
#define NewNPP_InitializeProc(FUNC)		\
		((NPP_InitializeUPP) (FUNC))
#define CallNPP_InitializeProc(FUNC)		\
		(*(FUNC))()


/* NPP_Shutdown */


typedef void (*NPP_ShutdownUPP)(void);
#define NewNPP_ShutdownProc(FUNC)		\
		((NPP_ShutdownUPP) (FUNC))
#define CallNPP_ShutdownProc(FUNC)		\
		(*(FUNC))()


/* NPP_New */


typedef NPError	(*NPP_NewUPP)(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
#define NewNPP_NewProc(FUNC)		\
		((NPP_NewUPP) (FUNC))
#define CallNPP_NewProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)		\
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))


/* NPP_Destroy */


typedef NPError	(*NPP_DestroyUPP)(NPP instance, NPSavedData** save);
#define NewNPP_DestroyProc(FUNC)		\
		((NPP_DestroyUPP) (FUNC))
#define CallNPP_DestroyProc(FUNC, ARG1, ARG2)		\
		(*(FUNC))((ARG1), (ARG2))


/* NPP_SetWindow */


typedef NPError	(*NPP_SetWindowUPP)(NPP instance, NPWindow* window);
#define NewNPP_SetWindowProc(FUNC)		\
		((NPP_SetWindowUPP) (FUNC))
#define CallNPP_SetWindowProc(FUNC, ARG1, ARG2)		\
		(*(FUNC))((ARG1), (ARG2))


/* NPP_NewStream */


typedef NPError	(*NPP_NewStreamUPP)(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
#define NewNPP_NewStreamProc(FUNC)		\
		((NPP_NewStreamUPP) (FUNC))
#define CallNPP_NewStreamProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5) \
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5))


/* NPP_DestroyStream */

typedef NPError	(*NPP_DestroyStreamUPP)(NPP instance, NPStream* stream, NPError reason);
#define NewNPP_DestroyStreamProc(FUNC)		\
		((NPP_DestroyStreamUPP) (FUNC))
#define CallNPP_DestroyStreamProc(FUNC,  NPParg, NPStreamPtr, NPErrorArg)		\
		(*(FUNC))((NPParg), (NPStreamPtr), (NPErrorArg))



/* NPP_WriteReady */


typedef int32 (*NPP_WriteReadyUPP)(NPP instance, NPStream* stream);
#define NewNPP_WriteReadyProc(FUNC)		\
		((NPP_WriteReadyUPP) (FUNC))
#define CallNPP_WriteReadyProc(FUNC,  NPParg, NPStreamPtr)		\
		(*(FUNC))((NPParg), (NPStreamPtr))


/* NPP_Write */


typedef int32 (*NPP_WriteUPP)(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
#define NewNPP_WriteProc(FUNC)		\
		((NPP_WriteUPP) (FUNC))
#define CallNPP_WriteProc(FUNC,  NPParg, NPStreamPtr, offsetArg, lenArg, bufferPtr)		\
		(*(FUNC))((NPParg), (NPStreamPtr), (offsetArg), (lenArg), (bufferPtr))


/* NPP_StreamAsFile */


typedef void (*NPP_StreamAsFileUPP)(NPP instance, NPStream* stream, const char* fname);
#define NewNPP_StreamAsFileProc(FUNC)		\
		((NPP_StreamAsFileUPP) (FUNC))
#define CallNPP_StreamAsFileProc(FUNC,  ARG1, ARG2, ARG3)		\
		(*(FUNC))((ARG1), (ARG2), (ARG3))


/* NPP_Print */


typedef void (*NPP_PrintUPP)(NPP instance, NPPrint* platformPrint);
#define NewNPP_PrintProc(FUNC)		\
		((NPP_PrintUPP) (FUNC))
#define CallNPP_PrintProc(FUNC,  NPParg, NPPrintArg)		\
		(*(FUNC))((NPParg), (NPPrintArg))



/* NPP_HandleEvent */


typedef int16 (*NPP_HandleEventUPP)(NPP instance, void* event);
#define NewNPP_HandleEventProc(FUNC)		\
		((NPP_HandleEventUPP) (FUNC))
#define CallNPP_HandleEventProc(FUNC,  NPParg, voidPtr)		\
		(*(FUNC))((NPParg), (voidPtr))


/* NPP_URLNotify */


typedef void (*NPP_URLNotifyUPP)(NPP instance, const char* url, NPReason reason, void* notifyData);
#define NewNPP_URLNotifyProc(FUNC)		\
		((NPP_URLNotifyUPP) (FUNC))
#define CallNPP_URLNotifyProc(FUNC,  ARG1, ARG2, ARG3, ARG4)		\
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4))



/*
 *  Netscape entry points
 */

/* NPN_GetUrlNotify */

typedef NPError	(*NPN_GetURLNotifyUPP)(NPP instance, const char* url, const char* window, void* notifyData);
#define NewNPN_GetURLNotifyProc(FUNC)		\
		((NPN_GetURLNotifyUPP) (FUNC))
#define CallNPN_GetURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4)		\
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4))


/* NPN_PostUrlNotify */

typedef NPError (*NPN_PostURLNotifyUPP)(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData);
#define NewNPN_PostURLNotifyProc(FUNC)		\
		((NPN_PostURLNotifyUPP) (FUNC))
#define CallNPN_PostURLNotifyProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) \
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7))


/* CHPN_GetUrl */


typedef NPError	(*NPN_GetURLUPP)(NPP instance, const char* url, const char* window);
#define NewNPN_GetURLProc(FUNC)		\
		((NPN_GetURLUPP) (FUNC))
#define CallNPN_GetURLProc(FUNC, ARG1, ARG2, ARG3)		\
		(*(FUNC))((ARG1), (ARG2), (ARG3))


/* NPN_PostUrl */


typedef NPError (*NPN_PostURLUPP)(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file);
#define NewNPN_PostURLProc(FUNC)		\
		((NPN_PostURLUPP) (FUNC))
#define CallNPN_PostURLProc(FUNC, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
		(*(FUNC))((ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6))


/* NPN_RequestRead */


typedef NPError	(*NPN_RequestReadUPP)(NPStream* stream, NPByteRange* rangeList);
#define NewNPN_RequestReadProc(FUNC)		\
		((NPN_RequestReadUPP) (FUNC))
#define CallNPN_RequestReadProc(FUNC, stream, range)		\
		(*(FUNC))((stream), (range))



/* NPN_NewStream */



typedef NPError	(*NPN_NewStreamUPP)(NPP instance, NPMIMEType type, const char* window, NPStream** stream);
#define NewNPN_NewStreamProc(FUNC)		\
		((NPN_NewStreamUPP) (FUNC))
#define CallNPN_NewStreamProc(FUNC, npp, type, window, stream)		\
		(*(FUNC))((npp), (type), (window), (stream))


/* NPN_Write */


typedef int32 (*NPN_WriteUPP)(NPP instance, NPStream* stream, int32 len, void* buffer);
#define NewNPN_WriteProc(FUNC)		\
		((NPN_WriteUPP) (FUNC))
#define CallNPN_WriteProc(FUNC, npp, stream, len, buffer)		\
		(*(FUNC))((npp), (stream), (len), (buffer))


/* NPN_DestroyStream */


typedef NPError (*NPN_DestroyStreamUPP)(NPP instance, NPStream* stream, NPError reason);
#define NewNPN_DestroyStreamProc(FUNC)		\
		((NPN_DestroyStreamUPP) (FUNC))
#define CallNPN_DestroyStreamProc(FUNC, npp, stream, err)		\
		(*(FUNC))((npp), (stream), (err))


/* NPN_Status */

typedef void (*NPN_StatusUPP)(NPP instance, const char* message);
#define NewNPN_StatusProc(FUNC)		\
		((NPN_StatusUPP) (FUNC))
#define CallNPN_StatusProc(FUNC, npp, msg)		\
		(*(FUNC))((npp), (msg))	



/* NPN_UserAgent */

typedef const char*	(*NPN_UserAgentUPP)(NPP instance);
#define NewNPN_UserAgentProc(FUNC)              \
                ((NPN_UserAgentUPP) (FUNC))
#define CallNPN_UserAgentProc(FUNC, ARG1)               \
                (*(FUNC))((ARG1))



/* NPN_MemAlloc */

typedef void* (*NPN_MemAllocUPP)(uint32 size);
#define NewNPN_MemAllocProc(FUNC)		\
		((NPN_MemAllocUPP) (FUNC))
#define CallNPN_MemAllocProc(FUNC, ARG1)		\
		(*(FUNC))((ARG1))	



/* NPN__MemFree */


typedef void (*NPN_MemFreeUPP)(void* ptr);
#define NewNPN_MemFreeProc(FUNC)		\
		((NPN_MemFreeUPP) (FUNC))
#define CallNPN_MemFreeProc(FUNC, ARG1)		\
		(*(FUNC))((ARG1))	


/* NPN_MemFlush */


typedef uint32 (*NPN_MemFlushUPP)(uint32 size);
#define NewNPN_MemFlushProc(FUNC)		\
		((NPN_MemFlushUPP) (FUNC))
#define CallNPN_MemFlushProc(FUNC, ARG1)		\
		(*(FUNC))((ARG1))	

/* NPN_ReloadPlugins */

typedef void (*NPN_ReloadPluginsUPP)(NPBool reloadPages);
#define NewNPN_ReloadPluginsProc(FUNC)		\
		((NPN_ReloadPluginsUPP) (FUNC))
#define CallNPN_ReloadPluginsProc(FUNC, ARG1)		\
		(*(FUNC))((ARG1))	

/* NPN_GetJavaEnv */


typedef JRIEnv* (*NPN_GetJavaEnvUPP)(void);
#define NewNPN_GetJavaEnvProc(FUNC)		\
		((NPN_GetJavaEnvUPP) (FUNC))
#define CallNPN_GetJavaEnvProc(FUNC)		\
		(*(FUNC))()	



/* NPN_GetJavaPeer */


typedef jref (*NPN_GetJavaPeerUPP)(NPP instance);
#define NewNPN_GetJavaPeerProc(FUNC)		\
		((NPN_GetJavaPeerUPP) (FUNC))
#define CallNPN_GetJavaPeerProc(FUNC, ARG1)		\
		(*(FUNC))((ARG1))	





/******************************************************************************************
 * The actual plugin function table definitions
 *******************************************************************************************/

typedef struct _NPPluginFuncs {
    uint16 size;
    uint16 version;
    NPP_NewUPP newp;
    NPP_DestroyUPP destroy;
    NPP_SetWindowUPP setwindow;
    NPP_NewStreamUPP newstream;
    NPP_DestroyStreamUPP destroystream;
    NPP_StreamAsFileUPP asfile;
    NPP_WriteReadyUPP writeready;
    NPP_WriteUPP write;
    NPP_PrintUPP print;
    NPP_HandleEventUPP event;
    NPP_URLNotifyUPP urlnotify;
    JRIGlobalRef javaClass;
} NPPluginFuncs;

typedef struct _NPNetscapeFuncs {
    uint16 size;
    uint16 version;
    NPN_GetURLUPP geturl;
    NPN_PostURLUPP posturl;
    NPN_RequestReadUPP requestread;
    NPN_NewStreamUPP newstream;
    NPN_WriteUPP write;
    NPN_DestroyStreamUPP destroystream;
    NPN_StatusUPP status;
    NPN_UserAgentUPP uagent;
    NPN_MemAllocUPP memalloc;
    NPN_MemFreeUPP memfree;
    NPN_MemFlushUPP memflush;
    NPN_ReloadPluginsUPP reloadplugins;
    NPN_GetJavaEnvUPP getJavaEnv;
    NPN_GetJavaPeerUPP getJavaPeer;
    NPN_GetURLNotifyUPP geturlnotify;
    NPN_PostURLNotifyUPP posturlnotify;

} NPNetscapeFuncs;




/*
 * Main entry point of the plugin.
 * This routine will be called when the plugin is loaded. The function
 * tables are passed in and the plugin fills in the NPPluginFuncs table
 * and NPPShutdownUPP for Netscape's use.
 */


#ifdef __cplusplus
extern "C" {
#endif

/* plugin meta member functions */

NPError WINAPI NP_GetEntryPoints(NPPluginFuncs* pFuncs);

NPError WINAPI NP_Initialize(NPNetscapeFuncs* pFuncs);

NPError WINAPI NP_Shutdown();



#ifdef __cplusplus
}
#endif

typedef NPError  (WINAPI *NP_GetEntryPointsUPP)(NPPluginFuncs* pFuncs);
typedef NPError  (WINAPI *NP_InitializeUPP)(NPNetscapeFuncs* pFuncs );
typedef NPError  (WINAPI *NP_ShutdownUPP)();

#endif /* _NPAPI_H_ */

