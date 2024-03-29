/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief System-specific implementation of the \ref _read function used by
 *         the standard library.
 *
 * - Compiler:           IAR EWAVR and GNU GCC for AVR
 * - Supported devices:  All AVR devices with a USART module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "compiler.h"

// IAR common implmentation
#if ( defined(__ICCAVR32__) || defined(__ICCAVR__))

#include <yfuns.h>

_STD_BEGIN

#pragma module_name = "?__read"
extern volatile void *volatile stdio_base;
void (*ptr_get)(void volatile*,int*);

/*! \brief Reads a number of bytes, at most \a size, into the memory area
 *         pointed to by \a buffer.
 *
 * \param handle File handle to read from.
 * \param buffer Pointer to buffer to write read bytes to.
 * \param size Number of bytes to read.
 *
 * \return The number of bytes read, \c 0 at the end of the file, or
 *         \c _LLIO_ERROR on failure.
 */
size_t __read(int handle, unsigned char *buffer, size_t size)
{
	int nChars = 0;
	// This implementation only reads from stdin.
	// For all other file handles, it returns failure.
	if (handle != _LLIO_STDIN) {
		return _LLIO_ERROR;
	}
	for (; size > 0; --size) {
		int c;
		ptr_get(stdio_base,&c);
		if (c < 0)
			break;
		*buffer++ = c;
		++nChars;
	}
	return nChars;
}
_STD_END

// GCC AVR32 implementation
#elif (defined(__GNUC__) && !defined(XMEGA))

extern volatile void *volatile stdio_base;
extern void (*ptr_get)(void volatile*,int*);


int __attribute__((weak))
_read (int file, char * ptr, int len)
{
	int nChars = 0;

	if (file != 0)
		return -1;

	for (; len > 0; --len) {
		int c;
		ptr_get(stdio_base,&c);
		if (c < 0)
		break;
		*ptr++ = c;
		++nChars;
	}
	return nChars;
}

// GCC AVR implementation
#elif (defined(__GNUC__) && defined(XMEGA))

extern volatile void *volatile stdio_base;
extern void (*ptr_get)(void volatile*,int*);
int _read (int *f)
{
	int c;
	ptr_get(stdio_base,&c);
	return c;
}

#endif
