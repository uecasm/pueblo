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

	This file consists of the Internet implementation for the SOCKS connect

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"						// Precompiled header directive

#include <ChSock.h>
#include <ChExcept.h>

#include <MemDebug.h>

void sockinetbuf::SOCKSconnect( unsigned long addr, int port_no )
{
											/* Address and portno are in host
												byte order */
    sockinetaddr	sa( addr, port_no );

    SOCKSconnect( sa );
}


void sockinetbuf::SOCKSconnect( const char* pstrHost, int iPort )
{
	sockinetaddr	sa( pstrHost, iPort, m_pSocket->IsAsyncSocket() );

	SOCKSconnect( sa );
}


void sockinetbuf::SOCKSconnect( unsigned long addr,
								const char* service_name,
								const char* protocol_name )
{
    sockinetaddr sa( addr, service_name, protocol_name,
    					m_pSocket->IsAsyncSocket() );

    SOCKSconnect( sa );
}


void sockinetbuf::SOCKSconnect( const char* host_name,
								const char* service_name,
								const char* protocol_name )
{
    sockinetaddr	sa( host_name, service_name, protocol_name,
    						m_pSocket->IsAsyncSocket() );

    SOCKSconnect( sa );
}


void sockinetbuf::SOCKSconnect( sockAddr& sa )
{  
	if (!ChChacoSocket::IsSOCKSInited() && ChChacoSocket::IsUsingSOCKS())
	{
		m_pSocket->InitSOCKSAddress();
	}

	if (ChChacoSocket::IsUsingSOCKS())
	{										// Connect to socks server

	    connect( ChChacoSocket::m_saSOCKS );// Will throw exception if it fails

											/* If I get here the we have
												connected to the SOCKS server.
												Send the SOCKS packet.  */
		EstablishSOCKSconnect( sa );
	}
	else
	{										// Do normal connect without SOCKS

		//m_pSocket->Connect( sa, sizeof( sockAddr ) );
		connect( sa );
	}
}


void sockinetbuf::EstablishSOCKSconnect( sockAddr& sa )
{
	bool			boolSuccess = true;
	unsigned char	socksPacket[14];

	socksPacket[0] = 4;	 // Version of socks
	socksPacket[1] = 1;	 // Connect command
	// Port number
	ChMemCopy( &socksPacket[2], &sa.m_sockAddrIn.sin_port, 2 );
	// IP address for the connect
	ChMemCopy( &socksPacket[4], &sa.m_sockAddrIn.sin_addr.s_addr, 4 );
	// User ID : Is this right ?
	ChMemCopy( &socksPacket[8], "guest", 5 );
	// NULL terminator
	socksPacket[13] = 0;

	// Send the command
	int wcnt = write( socksPacket, 14 );
	// 
	if (wcnt == -1)
	{	
		boolSuccess  = false;
	}

 	if (boolSuccess)
	{

		if (m_pSocket->IsAsyncSocket())
		{
			ChAsyncChacoSocket* pAsync = m_pSocket->GetAsyncSocket();
			// Waiting for socks
			pAsync->m_iState = ChAsyncChacoSocket::stateWaitingForSOCKS;

			pAsync->m_boolAsyncCallBlocking = true;
	
			pAsync->PumpMessages( (HANDLE)m_pSocket->GetSocket() );
			// We are out of blocking, check the status
			if ( pAsync->m_iSOCKSStatus != 90 )
			{
				boolSuccess = false;
			}
			// Just in case
			pAsync->m_boolAsyncCallBlocking = false;
	
			pAsync->m_iState = ChAsyncChacoSocket::stateConnected;
		}
		else
		{
			int iRead = read( socksPacket, 8 );

			int iRet = socksPacket[1];
			if ( iRet != 90 )
			{
				boolSuccess = false;
			}
		}
	}

 	if (!boolSuccess)
	{										// Connect failed throw exception
		#if defined( CH_EXCEPTIONS )
		{
			throw ChSocketEx( ChEx::connectFailed );
		}
		#endif
	}
}

// $Log$
