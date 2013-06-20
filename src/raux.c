/*** raux.c -- rotz aux stuff
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of rotz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */

#include "raux.h"
#include "nifty.h"


static inline void
shsort_gap(rtz_wtxlst_t wl, unsigned int gap)
{
	for (size_t i = gap; i < wl.z; i++) {
		unsigned int wi = wl.w[i];
		unsigned int di = wl.d[i];
		unsigned int j;

		for (j = i; j >= gap && wi > wl.w[j - gap]; j -= gap) {
			wl.w[j] = wl.w[j - gap];
			wl.d[j] = wl.d[j - gap];
		}
		wl.w[j] = wi;
		wl.d[j] = di;
	}
	return;
}

void
shsort(rtz_wtxlst_t wl)
{
	/* Ciura's gap sequence */
	static const unsigned int gaps[] = {
		701U, 301U, 132U, 57U, 23U, 10U, 4U, 1U
	};
	unsigned int h = gaps[0];

	/* extending the gap sequence by h_k <- 2.25h_{k-1} */
	for (unsigned int nx;
	     (nx = (unsigned int)(2.25 * (float)h)) < wl.z; h = nx);
	/* first go through the extended list */
	for (; h > gaps[0]; h = (unsigned int)((float)h / 2.25)) {
		shsort_gap(wl, h);
	}
	/* then resort to Ciura's list */
	for (size_t i = 0; i < countof(gaps); i++) {
		shsort_gap(wl, gaps[i]);
	}
	return;
}

/* raux.c ends here */
