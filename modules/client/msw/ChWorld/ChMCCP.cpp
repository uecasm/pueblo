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

					Framed this class wrapper; it was not originally part of
					the Pueblo client.

------------------------------------------------------------------------------

	This file consists of the implementation of the ChMCCP class, which handles
	the behind-the-scenes stuff for server-to-client compression using MCCP.

	This class is based on code written by Oliver Jowett <icecube@ihug.co.nz>

----------------------------------------------------------------------------*/

// $Header$

#include "headers.h"
#include "ChMCCP.h"
#include <zlib.h>
#include "MemDebug.h"

/* Telnet values we're interested in */

#define IAC             255
#define DONT            254
#define DO              253
#define WONT            252
#define WILL            251
#define SB              250
#define SE              240

#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86

/* We say DO COMPRESS2 to WILL COMPRESS2, then DONT COMPRESS to WILL COMPRESS
 * -or-
 * We say DO COMPRESS to WILL COMPRESS if it arrives before any COMPRESS2.
 *
 * Later the server sends IAC SB COMPRESS IAC SE (v2) or IAC SB COMPRESS WILL
 * SE (v1), and immediately following
 * that, begins compressing
 *
 * Compression ends on a Z_STREAM_END, no other marker is used
 */

static unsigned char on_v1[] =   { IAC, SB, TELOPT_COMPRESS, WILL, SE, 0 };
static unsigned char on_v2[] =   { IAC, SB, TELOPT_COMPRESS2, IAC, SE, 0 };

ChMCCP::ChMCCP()
{
  m_state.stream   = NULL;   /* Not decompressing */
  m_state.inalloc  = 2048;
  m_state.outalloc = 2048;
  m_state.inbuf    = (unsigned char *)malloc(m_state.inalloc);
  m_state.outbuf   = (unsigned char *)malloc(m_state.outalloc);
  m_state.insize   = 0;
  m_state.outsize  = 0;
  m_state.comp     = 0;
  m_state.uncomp   = 0;
  m_state.error    = false;
	m_state.mode     = modeUncompressed;
}

ChMCCP::~ChMCCP()
{
  if (m_state.stream) {
    inflateEnd(m_state.stream);
    delete m_state.stream;
  }
	free(m_state.inbuf);
	free(m_state.outbuf);
}

void ChMCCP::grow_inbuf(int needed)
{
  unsigned int old = m_state.inalloc;
  
  while (m_state.inalloc < m_state.insize + needed)
    m_state.inalloc *= 2;

  if (old != m_state.inalloc)
    m_state.inbuf = (unsigned char *)realloc(m_state.inbuf, m_state.inalloc);
}
        
void ChMCCP::grow_outbuf(int needed)
{
  unsigned int old = m_state.outalloc;
  
  while (m_state.outalloc < m_state.outsize + needed)
    m_state.outalloc *= 2;

  if (old != m_state.outalloc)
    m_state.outbuf = (unsigned char *)realloc(m_state.outbuf, m_state.outalloc);
}

void ChMCCP::decompress_inbuf()
{
	int status;

	/* We are now decompressing from inbuf to outbuf */

	if (!m_state.insize)
		return; /* nothing to decompress? */

	m_state.stream->next_in = m_state.inbuf;
	m_state.stream->next_out = m_state.outbuf + m_state.outsize;
	m_state.stream->avail_in = m_state.insize;
	m_state.stream->avail_out = m_state.outalloc - m_state.outsize;

	status = inflate(m_state.stream, Z_SYNC_FLUSH);

	if ((status == Z_OK) || (status == Z_STREAM_END)) {
		/* Successful decompression */

		/* Remove used data from inbuf */
		m_state.comp += m_state.insize - m_state.stream->avail_in;
		m_state.uncomp += m_state.stream->next_out - m_state.outbuf;

		memmove(m_state.inbuf, m_state.stream->next_in, m_state.stream->avail_in);
		m_state.insize = m_state.stream->avail_in;

		/* Update outbuf pointers */
		m_state.outsize = m_state.stream->next_out - m_state.outbuf;

		/* Done */

		if (status == Z_STREAM_END) {
			/* Turn off compression too */
			grow_outbuf(m_state.insize);

			memcpy(m_state.outbuf + m_state.outsize, m_state.inbuf, m_state.insize);
			m_state.outsize += m_state.insize;
			m_state.insize = 0;

			inflateEnd(m_state.stream);
			delete m_state.stream;
			m_state.stream = NULL;
		}

		return;
	}

	if (status == Z_BUF_ERROR) {
		/* Full buffers? Maybe we need more output space.. */

		if (m_state.outsize * 2 > m_state.outalloc) {
			grow_outbuf(m_state.outalloc);
			decompress_inbuf();
		}
		return;
	}

	/* Error */
	m_state.error = true;
}

/* We received some data */
void ChMCCP::receive(const char *data, unsigned len)
{
	if (m_state.error)
		return;

	if (!m_state.stream) {
		int residual = -1;
		int clen;

		/* Just copy to outbuf. Also copy any residual inbuf */

		grow_outbuf(len + m_state.insize);
		memcpy(m_state.outbuf + m_state.outsize, data, len);
		m_state.outsize += len;
		memcpy(m_state.outbuf + m_state.outsize, m_state.inbuf, m_state.insize);
		m_state.outsize += m_state.insize;
		m_state.insize = 0;

		/* Check for Magic Marker. ugh this is messy */
		for (unsigned int i = 0; i < m_state.outsize; i++) {
			if (m_state.outbuf[i] == IAC) {
				if (i + 1 < m_state.outsize && m_state.outbuf[i+1] == IAC) {
					/* IAC IAC - ignore */
					i++;
					continue;
				}

				int slen = strlen((char *)on_v1);
				clen = (i + slen < m_state.outsize) ? slen : m_state.outsize - i;

				if ((!memcmp(&m_state.outbuf[i], on_v1, clen) && (m_state.mode == modeCompressV1)) ||
					(!memcmp(&m_state.outbuf[i], on_v2, clen) && (m_state.mode == modeCompressV2))) {
						if (clen != slen) {
							/* Partial match. Save it. */
							residual = i;
							break;
						}

						/* Switch to compression */
						/* copy any compressible bits to our inbuf */

						grow_inbuf(m_state.outsize - i - slen);

						memcpy(m_state.inbuf, m_state.outbuf + i + slen, m_state.outsize - i - slen);

						m_state.insize = m_state.outsize - i - slen;

						/* clean up our output buffer */
						m_state.outsize = i;

						/* init stream */
						m_state.stream = new z_stream;
						m_state.stream->zalloc = Z_NULL;
						m_state.stream->zfree = Z_NULL;
						m_state.stream->opaque = NULL;

						if (inflateInit(m_state.stream) != Z_OK) {
							m_state.error = true;
							delete m_state.stream;
							m_state.stream = NULL;
							return;
						}

						/* Continue with decompression */
						break;
					}
			}
		}

		if (!m_state.stream) { /* didn't start decompressing? */
			/* We might have some residual, copy to inbuf for later checking */

			if (residual != -1) {
				grow_inbuf(m_state.outsize - residual);
				memcpy(m_state.inbuf + m_state.insize, m_state.outbuf + residual, m_state.outsize - residual);
				m_state.outsize = residual;
			}

			return;
		}
	} else {
		/* New data to decompress. Copy to inbuf */
		grow_inbuf(len);
		memcpy(m_state.inbuf + m_state.insize, data, len);
		m_state.insize += len;
	}

	decompress_inbuf();
}

/* How much data is available? */
int ChMCCP::pending()
{
  return m_state.error ? 0 : m_state.outsize;
}

/* Get some data */
int ChMCCP::get(char *buf, unsigned int size)
{
	int copied;

	if (m_state.error || !m_state.outsize)
		return 0;

	if (size > m_state.outsize)
		copied = m_state.outsize;
	else
		copied = size;

	memcpy(buf, m_state.outbuf, copied);
	m_state.outsize -= copied;
	if (m_state.outsize)
		memmove(m_state.outbuf, m_state.outbuf + copied, m_state.outsize);

	/* Do some more decompression */
	decompress_inbuf();

	return copied;
}

void ChMCCP::stats(unsigned long *comp, unsigned long *uncomp)
{
	*comp = m_state.comp;
	*uncomp = m_state.uncomp;
}

void ChMCCP::negotiated(CompressMode newMode)
{
	if(m_state.mode == newMode) return;

	if((m_state.mode != modeUncompressed) && compressing())
	{
		// mode change from a compressed mode, so release buffers
		m_state.insize = 0;

		inflateEnd(m_state.stream);
		delete m_state.stream;
		m_state.stream = NULL;
	}

	m_state.mode = newMode;
	m_state.error = false;
}

// $Log$
