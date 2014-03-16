/*** rotz-del.c -- rotz tag del'er
 *
 * Copyright (C) 2013-2014 Sebastian Freundt
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
#include <fcntl.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "rotz-umb.h"
#include "nifty.h"


static int verbosep;
#define _		rotz_massage_name

static void
del_tag(rotz_t ctx, rtz_vtx_t tid, const char *sym)
{
	const char *symspc_sym;
	rtz_vtx_t sid;

	if (UNLIKELY((symspc_sym = rotz_sym(sym)) == NULL)) {
		return;
	} else if (LIKELY((sid = rotz_get_vertex(ctx, symspc_sym)) == 0U)) {
		if (UNLIKELY(verbosep)) {
			fprintf(stderr, "\
Error: cannot find sym `%s' in database file\n", sym);
		}
		return;
	}

	if (UNLIKELY(verbosep)) {
		fputc('-', stdout);
		fputs(_(rotz_get_name(ctx, tid)), stdout);
		fputc('\t', stdout);
		fputs(_(rotz_get_name(ctx, sid)), stdout);
		fputc('\n', stdout);
	}

	rotz_rem_edge(ctx, tid, sid);
	rotz_rem_edge(ctx, sid, tid);
	return;
}

static void
del_vtx(rotz_t ctx, const char *v)
{
	rtz_vtxlst_t el;
	rtz_vtx_t vid;

	if (UNLIKELY((vid = rotz_get_vertex(ctx, v)) == 0U)) {
		/* not sure what to delete */
		return;
	} else if (LIKELY((el = rotz_get_edges(ctx, vid)).d == NULL)) {
		/* nothing to delete */
		;
	} else {
		/* get rid of all the edges */
		rotz_rem_edges(ctx, vid);

		/* now go through the list EL and delete TID */
		for (size_t i = 0; i < el.z; i++) {
			if (UNLIKELY(verbosep)) {
				fputc('-', stdout);
				fputs(_(rotz_get_name(ctx, vid)), stdout);
				fputc('\t', stdout);
				fputs(_(rotz_get_name(ctx, el.d[i])), stdout);
				fputc('\n', stdout);
			}
			rotz_rem_edge(ctx, el.d[i], vid);
		}
		/* finalise the list */
		rotz_free_vtxlst(el);
	}
	/* finally delete the vertex */
	rotz_rem_vertex(ctx, v);
	return;
}

static void
del_syms(rotz_t ctx, const char *tag)
{
	/* massage tag */
	del_vtx(ctx, rotz_tag(tag));
	return;
}

static void
del_sym(rotz_t ctx, const char *sym)
{
	/* massage sym */
	del_vtx(ctx, rotz_sym(sym));
	return;
}


#if defined STANDALONE
int
rotz_cmd_del(const struct yuck_cmd_del_s argi[static 1U])
{
	rotz_t ctx;

	if (argi->verbose_flag) {
		verbosep = 1;
	}

	if (UNLIKELY((ctx = make_rotz(db, O_CREAT | O_RDWR)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		return 1;
	}
	if (argi->nargs > 1U) {
		const char *tag;
		rtz_vtx_t tid;

		tag = rotz_tag(argi->args[0U]);
		if (UNLIKELY((tid = rotz_get_vertex(ctx, tag)) == 0U)) {
			if (UNLIKELY(verbosep)) {
				fprintf(stderr, "\
Error: cannot find tag `%s' in database file, no deletions\n", argi->args[0U]);
			}
			goto fini;
		}
		for (size_t i = 1U; i < argi->nargs; i++) {
			del_tag(ctx, tid, argi->args[i]);
		}
	} else if (argi->nargs == 1U && isatty(STDIN_FILENO)) {
		/* del all syms assoc'd with TAG */
		del_syms(ctx, argi->args[0U]);
	} else if (argi->nargs == 1U) {
		/* del tag/sym pairs from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;
		const char *tag;
		rtz_vtx_t tid;

		tag = rotz_tag(argi->args[0U]);
		if (UNLIKELY((tid = rotz_get_vertex(ctx, tag)) == 0U)) {
			if (UNLIKELY(verbosep)) {
				fprintf(stderr, "\
Error: cannot find tag `%s' in database file, no deletions\n", argi->args[0U]);
			}
			goto fini;
		}

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			del_tag(ctx, tid, line);
		}
		free(line);
	} else if (!isatty(STDIN_FILENO)) {
		/* del tags from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		if (argi->syms_flag) {
			while ((nrd = getline(&line, &llen, stdin)) > 0) {
				line[nrd - 1] = '\0';
				del_sym(ctx, line);
			}
		} else {
			while ((nrd = getline(&line, &llen, stdin)) > 0) {
				line[nrd - 1] = '\0';
				del_syms(ctx, line);
			}
		}
		free(line);
	}

fini:
	/* big rcource freeing */
	free_rotz(ctx);
	return 0;
}
#endif	/* STANDALONE */

/* rotz-del.c ends here */
