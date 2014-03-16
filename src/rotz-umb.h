/*** rotz-umb.h -- rotz umbrella tool
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
#if !defined INCLUDED_rotz_umb_h_
#define INCLUDED_rotz_umb_h_

#include "rotz-umb.yuch"

/* common arguments */
extern const char *db;


extern int rotz_cmd_add(const struct yuck_cmd_add_s*);
extern int rotz_cmd_alias(const struct yuck_cmd_alias_s*);
extern int rotz_cmd_cloud(const struct yuck_cmd_cloud_s*);
extern int rotz_cmd_combine(const struct yuck_cmd_combine_s*);
extern int rotz_cmd_del(const struct yuck_cmd_del_s*);
extern int rotz_cmd_export(const struct yuck_cmd_export_s*);
extern int rotz_cmd_fsck(const struct yuck_cmd_fsck_s*);
extern int rotz_cmd_grep(const struct yuck_cmd_grep_s*);
extern int rotz_cmd_rename(const struct yuck_cmd_rename_s*);
extern int rotz_cmd_search(const struct yuck_cmd_search_s*);
extern int rotz_cmd_show(const struct yuck_cmd_show_s*);

#endif	/* INCLUDED_rotz_umb_h_ */
