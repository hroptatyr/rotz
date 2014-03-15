/*** rotz-rename.c -- rotz tag renameer
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
#include <fcntl.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "nifty.h"


static void
rename_tag(rotz_t ctx, const char *old, const char *new)
{
/* we rename by adding an alias NEW and then removing the alias OLD */
	const char *tag;
	rtz_vtx_t tid;

	if (UNLIKELY((tag = rotz_tag(old)) == NULL)) {
		return;
	} else if (UNLIKELY(!(tid = rotz_get_vertex(ctx, tag)))) {
		return;
	} else if (UNLIKELY((tag = rotz_tag(new)) == NULL)) {
		return;
	} else if (UNLIKELY(rotz_add_alias(ctx, tid, tag) < 0)) {
		fputs("\
couldn't rename tags, target tag exists\n", stderr);
		return;
	}
	/* delete the old `alias' */
	rotz_rem_alias(ctx, rotz_tag(old));
	return;
}


#if defined STANDALONE
#include "rotz-rename.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	rotz_t ctx;
	const char *db = RTZ_DFLT_DB;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	} else if (argi->nargs != 2) {
		fputs("Error: need OLDNAME and NEWNAME\n\n", stderr);
		yuck_auto_help(argi);
		rc = 1;
		goto out;
	}

	if (argi->database_arg) {
		db = argi->database_arg;
	}
	if (UNLIKELY((ctx = make_rotz(db, O_CREAT | O_RDWR)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		rc = 1;
		goto out;
	}

	rename_tag(ctx, argi->args[0U], argi->args[1U]);

	/* big rcource freeing */
	free_rotz(ctx);
out:
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* rotz-rename.c ends here */
