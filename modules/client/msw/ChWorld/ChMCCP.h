/*----------------------------------------------------------------------------
  _   _ _  _                 ____        _                     _
 | | | | || |               |  __|      | |                   (_)
 | | | | || |_ _ __  __ _   | |_  _ __ _| |_ ___ _ ___ __ _ __ _ ___  ___ ___
 | | | | |_  _| '__|/ _` |  |  _|| '_ \_   _| _ \ '__|'_ \ '__| | __|/ _ \ __|
 | '-' | || | | |  | (_| |  | |__| | | || ||  __/ | | |_) ||  | |__ |  __/__ |
  \___/|_||_| |_|   \__,_|  |____|_| |_||_| \___|_| | .__/_|  |_|___|\___|___|
                                                    | |     
                                                    |_|

    The contents of this file are subject to the Andromedia Public
	License Version 1.0 (the "License"); you may not use this file
	except in compliance with the License. You may obtain a copy of
	the License at http://pueblo.sf.net/APL/

    Software distributed under the License is distributed on an
	"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
	implied. See the License for the specific language governing
	rights and limitations under the License.

    The Original Code is Pueblo/UE client code, released April 1, 2002.

    The Initial Developer of the Original Code is Ultra Enterprises.
	
    Contributor(s):
	--------------------------------------------------------------------------
	   Ultra Enterprises:   Gavin Lambert

					Wrote and designed this class; it was not originally part of
					the Pueblo client.

------------------------------------------------------------------------------

	This file consists of the interface to the ChMCCP class, which handles
	the behind-the-scenes stuff for server-to-client compression using MCCP.

	This class is based on code written by Oliver Jowett <icecube@ihug.co.nz>

----------------------------------------------------------------------------*/

// $Header$

#if !defined( _CHMCCP_H )
#define _CHMCCP_H

typedef struct z_stream_s z_stream;

class ChMCCP
{
public:
	ChMCCP();
	~ChMCCP();

		enum CompressMode {
			modeUncompressed,
			modeCompressV1,
			modeCompressV2,
		};

    /*  Perform decompression and negotiation on some received data.
     *  'data' is a pointer to the received data, 'len' is its length.
     */
    void receive(const char *data, unsigned len);

    /*  Return the number of pending decompressed bytes that can currently
     *  be read by mudcompress_get
     */
    int pending();

    /*  Return true if this decompressor encountered a fatal error.
     */
		bool error() { return m_state.error; }

    /*  Read decompressed data from the decompressor into 'buf', up to a
     *  maximum of 'size' bytes. Returns the number of bytes actually copied.
     */
    int get(char *buf, unsigned int size);

    /*  Set *comp to the number of compressed bytes read, and *uncomp to the
     *  number of bytes they expanded to, for this decompressor.
     */
    void stats(unsigned long *comp, unsigned long *uncomp);

    /*  Return true if this decompressor has successfully negotiated
     *  compression and is currently performing decompression.
     */
		bool compressing() { return (m_state.stream != NULL); }

		/*  Change compression mode (because of prior telnet negotiation)
		 */
		void negotiated(CompressMode newMode);

protected:
		void grow_inbuf(int needed);
		void grow_outbuf(int needed);
		void decompress_inbuf();

private:
	struct mc_state {
    z_stream *stream;      /* stream we're using */

    unsigned char *inbuf;  /* input buffer (data from mud) */
    unsigned int insize;   /* .. and how much is used */
    unsigned int inalloc;  /* .. and how much is allocated */
    
    unsigned char *outbuf; /* output buffer (data to user) */
    unsigned int outsize;  /* .. and how much is used */
    unsigned int outalloc; /* .. and how much is allocated */

    bool error;
		CompressMode mode;     /* type of compression in effect */
    
    unsigned long comp;
    unsigned long uncomp;
	} m_state;
};

#endif	// !defined( _CHMCCP_H )

// $Log$
