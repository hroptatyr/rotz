/*** rotz-fsck.c -- rotz tag fsck'er
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#if defined USE_TCBDB
# include <tcbdb.h>
#endif	/* USE_TCBDB */

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "nifty.h"


#if defined USE_LMDB
static void
dberror(rotz_t UNUSED(ctx), const char *premsg)
{
	fputs(premsg, stderr);
	fputc('\n', stderr);
	return;
}

static int
dfrg(rotz_t UNUSED(ctx))
{
	return 0;
}

static int
opti(rotz_t UNUSED(ctx))
{
	return 0;
}
#elif defined USE_TCBDB
struct rotz_s {
	TCBDB *db;
};

static void
dberror(rotz_t ctx, const char *premsg)
{
	int ecode = tcbdbecode(ctx->db);

	fputs(premsg, stderr);
	fputs(tcbdberrmsg(ecode), stderr);
	fputc('\n', stderr);
	return;
}

static int
dfrg(rotz_t ctx)
{
	if (!tcbdbdefrag(ctx->db, INT64_MAX)) {
		return -1;
	}
	return 0;
}

static int
opti(rotz_t ctx)
{
	/* values are taken from tcbmgr.c */
	static struct {
		int lmemb;
		int nmemb;
		int bnum;
		int8_t apow;
		int8_t fpow;
		uint8_t opts;
	} opt = {
		.lmemb = -1,
		.nmemb = -1,
		.bnum = -1,
		.apow = -1,
		.fpow = -1,
		.opts = UINT8_MAX,
	};

	if (!tcbdboptimize(
		ctx->db,
		opt.lmemb, opt.nmemb, opt.bnum, opt.apow, opt.fpow, opt.opts)) {
		return -1;
	}
	return 0;
}
#endif	/* USE_TCBDB */


#if defined STANDALONE
#include "rotz-fsck.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	const char *db = RTZ_DFLT_DB;
	rotz_t ctx;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
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

	/* first step is to defrag, ... */
	if (UNLIKELY(dfrg(ctx) < 0)) {
		dberror(ctx, "Error during defrag: ");
	} else if (UNLIKELY(opti(ctx) < 0)) {
		dberror(ctx, "Error during optimisation: ");
	}

	/* big rcource freeing */
	free_rotz(ctx);
out:
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* rotz-fsck.c ends here */
