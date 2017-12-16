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

	This file consists of the implementation for the ChConn class, which
	handles connections.

		sockbuf			(sockinetbuf on the client)
			ChConn

----------------------------------------------------------------------------*/

#if !defined( _CHCONN_H )
#define _CHCONN_H

#include <SocketXX.h>
//#include <protocol.h>


#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA    
#endif


/*----------------------------------------------------------------------------
	ChConn class
----------------------------------------------------------------------------*/

											/* Currently, the client and
												server versions of ChConn
												are different, as the client
												version is based upon
												sockinetbuf and the server
												version is based upon
												sockbuf.  This should be fixed
												eventually. */

class CH_EXPORT_CLASS ChConn : public sockinetbuf
{
	private:
		chint32		m_lConnId;

	public:
		char input[1024];

		ChConn( sockinetbuf s ) : sockinetbuf( s ),
				m_lConnId( 0 )
			{
				input[0] = '\0';
			}

		ChConn() :
				sockinetbuf( sockbuf::sock_stream ),
				m_lConnId( 0 )

			{
				input[0] = '\0';
			}

		ChConn( ChSocketHandler pHandler, chparam userData = 0 ) :
				sockinetbuf( sockbuf::sock_stream, pHandler, userData ),
				m_lConnId( 0 )
			{
				input[0] = '\0';
			}

		void SendBlock( const void *pBuf, chint32 lLen ) const;
		void SendLine( const char *pstrLine ) const;
};

#if defined( CH_MSW ) && defined( CH_ARCH_16 )
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR    
#endif

#endif	// _CHCONN_H

// Local Variables: ***
// tab-width:4 ***
// End: ***
