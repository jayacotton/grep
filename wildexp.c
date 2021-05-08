#if 0
/*
	WILDEXP.C 	v1.1	3/21/82
	BDS C Command-line Wild-card expansion utility
	Written by Leor Zolman

	Lets ambiguous file names appear on the command line to C programs,
	automatically expanding the parameter list to contain all files that
	fit the afn's.

	An afn preceded by a "!" causes all names matching the given afn to
	be EXCLUDED from the resulting expansion list. Thus, to yield a
	command line containing all files except "COM" files, you'd say:

		A>progname !*.com <cr>

	Another example: to get all files on B: except .C files, say:

		A>prognam b:*.* !b:*.c <cr>

	When giving a "!" afn, "*" chars in the string matches to the end of
	either the filename or extension, just like CP/M, but "?" chars match
	ONE and ONLY ONE character in either the filename or extension.


	To use WILDEXP, begin your "main" function as follows:

	---------------------------------------------
	main(argc,argv)
	char **argv;
	{
		...			/* local declarations  */
wildexp(&argc, &argv);		/* first statement in program  */
dioinit(&argc, argv);		/* if using DIO, put this here */
... -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -and link WILDEXP.CRL in with your program.That 's all there is to
	it; note that "wildexp" uses the "sbrk" function to obtain storage,
	so don' t go playing around with memory that is outside of the
    external or stack areas unless you obtain the memory through "sbrk" or "alloc" calls. * /
#endif
#define BASE 0
#define ERROR -1
#define TRUE 1
#define FALSE 0
#include	<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trace.h"
#define		MAXITEMS	200	/* max no. of items after expansion */
#define		SEARCH_FIRST	17	/* BDOS calls */
#define		SEARCH_NEXT	18
extern int haswild(char *);
extern char *hackname(char *, char *);
extern int match(char *, char *, char);

char ** wildexp(int *oargcp, void *oargvp)
{
    int nargc;			/* new argc */
    char **nargv = 0;		/* new argv */
    char **oargv;		/* old argv */
    int oargc;			/* old argc */
    char fcb[36];		/* fcb used for search for first/next calls */
    char dmapos;		/* value returned by search calls */
    char first_time;		/* used in search routine */
    char tmpfn[20],		/* temp filename buffer */
    *tmpfnp;
    char *notfns[20];		/* list of !<afn> entries */
    int notcount;		/* count of entries in notfns */
    char cur_drive;		/* currently logged drive */
    int i, j, k;

    cur_drive = bdos(25, 0);

    oargvp += 2;
    oargv = (char **) oargvp;
    oargc = *oargcp;
    nargc = 1;
    notcount = 0;

    TRACE("wildexp");
    nargv = calloc(MAXITEMS * 2 + 2, 1);
    if (nargv == 0)
	return ERROR;

    for (i = 1; i < oargc; i++) {
	TRACE("");
	if (oargv[i][0] == '!') {
	    if (i == 1) {
		oargv[oargc] = "*.*";
		oargc++;
	    }
	    notfns[notcount++] = &oargv[i][1];
	} else if (!haswild(oargv[i]))
	    nargv[nargc++] = oargv[i];
	else {
	    TRACE("");
	    setfcb(fcb, oargv[i]);

	    tmpfnp = tmpfn;
	    if ((tmpfn[1] = oargv[i][1]) == ':') {
		tmpfn[0] = oargv[i][0];
		tmpfnp = tmpfn + 2;
		bdos(14, tmpfn[0] - 'A');
	    }

	    first_time = TRUE;
	    while (1) {		/* find all matching files */
		TRACE("");
		dmapos = bdos(first_time ? SEARCH_FIRST : SEARCH_NEXT,
			      fcb);
		if (dmapos == 255)
		    break;
		TRACE("");
		first_time = FALSE;
		hackname(tmpfnp, (BASE + 0x80 + dmapos * 32));
		nargv[nargc] = calloc(strlen(tmpfn) + 1, 1);
		if (nargv[nargc] == ERROR)
		    return ERROR;
		strcpy(nargv[nargc++], tmpfn);
		TRACE("");
	    }
	    bdos(14, cur_drive);	/* restore to current drive */
	}
    }
    for (i = 0; i < notcount; i++) {
	for (j = 1; j < nargc; j++) {
	    while (match(notfns[i], nargv[j], cur_drive)) {
		if (j == --nargc)
		    break;
		for (k = j; k < nargc; k++)
		    nargv[k] = nargv[k + 1];
	    }
	}
    }
    *oargcp = nargc;
    *oargvp = nargv;
    return nargv;
}

char *hackname(dest, source)
char *dest, *source;
{
    int i, j;
    TRACE("hackname");
    j = 0;

    for (i = 1; i < 9; i++) {
	if (source[i] == ' ')
	    break;
	dest[j++] = source[i];
    }
    if (source[9] != ' ')
	dest[j++] = '.';

    for (i = 9; i < 12; i++) {
	if (source[i] == ' ')
	    break;
	dest[j++] = source[i];
    }
    dest[j] = '\0';
    return dest;
}

int haswild(fname)
char *fname;
{
    char c;
    TRACE("haswild");
    while (c = *fname++)
	if (c == '*' || c == '?')
	    return TRUE;
    return FALSE;
}

int match(wildnam, filnam, cur_drive)
char *wildnam, *filnam, cur_drive;
{
    char c;
    TRACE("match");
    if (wildnam[1] != ':') {
	if (filnam[1] == ':')
	    if (filnam[0] - 'A' == cur_drive)
		filnam += 2;
	    else
		return FALSE;
    } else {
	if (filnam[1] != ':')
	    if (wildnam[0] - 'A' == cur_drive)
		wildnam += 2;
	    else
		return FALSE;
    }

    while (c = *wildnam++)
	if (c == '?')
	    if ((c = *filnam++) && c != '.')
		continue;
	    else
		return FALSE;
	else if (c == '*') {
	    while (c = *wildnam) {
		wildnam++;
		if (c == '.')
		    break;
	    }
	    while (c = *filnam) {
		filnam++;
		if (c == '.')
		    break;
	    }
	} else if (c == *filnam++)
	    continue;
	else
	    return FALSE;

    if (!*filnam)
	return TRUE;
    else
	return FALSE;
    return FALSE;
}
