/*
 * prof_file.c ---- routines that manipulate an individual profile file.
 */

#include "prof_int.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef NO_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <errno.h>


#ifdef _WINDOWS
#define stat _stat
#endif

errcode_t profile_open_file(filename, ret_prof)
	const char *filename;
	prf_file_t *ret_prof;
{
	prf_file_t	prf;
	errcode_t	retval;

	prf = malloc(sizeof(struct _prf_file_t));
	if (!prf)
		return ENOMEM;
	memset(prf, 0, sizeof(struct _prf_file_t));
	prf->filename = malloc(strlen(filename)+1);
	if (!prf->filename) {
		free(prf);
		return ENOMEM;
	}
	strcpy(prf->filename, filename);

	retval = profile_update_file(prf);
	if (retval) {
		profile_close_file(prf);
		return retval;
	}

	*ret_prof = prf;
	return 0;
}

errcode_t profile_update_file(prf)
	prf_file_t prf;
{
	errcode_t retval;
#ifdef HAS_STAT
	struct stat st;
#endif
	FILE *f;

#ifdef HAS_STAT
	if (stat(prf->filename, &st))
		return errno;
	if (st.st_mtime == prf->timestamp)
		return 0;
#endif
	if (prf->root)
		profile_free_node(prf->root);
	f = fopen(prf->filename, "r");
	if (f == NULL)
		return errno;
	retval = profile_parse_file(f, &prf->root);
	fclose(f);
	if (retval)
		return retval;
#ifdef HAS_STAT
	prf->timestamp = st.st_mtime;
#endif
	return 0;
}

errcode_t profile_close_file(prf)
	prf_file_t prf;
{
	if (prf->filename)
		free(prf->filename);
	if (prf->root)
		profile_free_node(prf->root);
	free(prf);

	return 0;
}

