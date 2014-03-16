/*** rotz-umb.c -- rotz umbrella tool
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

const char *db = RTZ_DFLT_DB;


# include "rotz-umb.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	if (argi->database_arg) {
		db = argi->database_arg;
	}

	switch (argi->cmd) {
	default:
	case ROTZ_CMD_NONE:
		fputs("\
rotz: Error: invalid or no command given.\n\
See rotz --help for a list of commands.\n", stderr);
		rc = 1;
		break;

	case ROTZ_CMD_ADD:
		rc = rotz_cmd_add((const void*)argi);
		break;
	case ROTZ_CMD_ALIAS:
		rc = rotz_cmd_alias((const void*)argi);
		break;
	case ROTZ_CMD_CLOUD:
		rc = rotz_cmd_cloud((const void*)argi);
		break;
	case ROTZ_CMD_COMBINE:
		rc = rotz_cmd_combine((const void*)argi);
		break;
	case ROTZ_CMD_DEL:
		rc = rotz_cmd_del((const void*)argi);
		break;
	case ROTZ_CMD_EXPORT:
		rc = rotz_cmd_export((const void*)argi);
		break;
	case ROTZ_CMD_FSCK:
		rc = rotz_cmd_fsck((const void*)argi);
		break;
	case ROTZ_CMD_GREP:
		rc = rotz_cmd_grep((const void*)argi);
		break;
	case ROTZ_CMD_RENAME:
		rc = rotz_cmd_rename((const void*)argi);
		break;
	case ROTZ_CMD_SEARCH:
		rc = rotz_cmd_search((const void*)argi);
		break;
	case ROTZ_CMD_SHOW:
		rc = rotz_cmd_show((const void*)argi);
		break;
	}

out:
	yuck_free(argi);
	return rc;
}

/* rotz-umb.c ends here */
