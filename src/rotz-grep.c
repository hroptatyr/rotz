/*** rotz-grep.c -- rotz tag grepper
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
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "raux.h"
#include "nifty.h"


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-grep.h"
#include "rotz-grep.x"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

static struct rotz_args_info argi[1];

static void
handle_one(rotz_t ctx, const char *input)
{
	const char *tagsym;
	rtz_vtx_t tsid;

	if ((tagsym = rotz_tag(input),
	     tsid = rotz_get_vertex(ctx, tagsym))) {
		;
	} else if ((tagsym = rotz_sym(input),
		    tsid = rotz_get_vertex(ctx, tagsym))) {
		;
	} else if (argi->invert_match_given) {
		/* not found but we're in invert-match mode */
		goto disp;
	} else {
		return;
	}

	/* found and we're in match mode? */
	if (LIKELY(!argi->invert_match_given)) {
		if (UNLIKELY(argi->normalise_given)) {
			input = rotz_massage_name(rotz_get_name(ctx, tsid));
		}
	disp:
		puts(input);
	}
	return;
}

int
main(int argc, char *argv[])
{
	rotz_t ctx;
	const char *db = RTZ_DFLT_DB;
	int res = 0;

	if (rotz_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->database_given) {
		db = argi->database_arg;
	}
	if (UNLIKELY((ctx = make_rotz(db)) == NULL)) {
		error("Error opening rotz datastore");
		res = 1;
		goto out;
	}

	for (unsigned int i = 0; i < argi->inputs_num; i++) {
		const char *const input = argi->inputs[i];

		handle_one(ctx, input);
	}
	if (argi->inputs_num == 0 && !isatty(STDIN_FILENO)) {
		/* read the guys from STDIN */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			handle_one(ctx, line);
		}
		free(line);
	}

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-grep.c ends here */
