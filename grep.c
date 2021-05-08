#include "tools.h"
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "trace.h"

#define MAXLINE 128
#define MAX_EXPR 64

extern void pr_count(int, char *, int);
extern void pr_match(int, char *, char *, int, int);
extern void cntrl_c();
extern char *matchs(char *, TOKEN *, int);
extern void xabort();
extern char **wildexp(int *, char ***);
extern void pr_usage();
extern void expand_sw(char *);
extern int get_expr(TOKEN **, int, char **);
extern char stoupper(char *);
extern TOKEN *makepat(char *, int);

int vflag, yflag, cflag, lflag, nflag, hflag, fflag;

// buffers to keep copies of the argv strings since wildcard
// is destroying arg 1 and arg 2.

char b1[32];
char b2[32];
char b3[32];
char b4[32];
char b5[32];
int bcount;

void main(int argc, char *argv[])
{
    int i, j, linenum, count;
    char line[MAXLINE];
    int numfiles;
    FILE *stream;
    int exprc;
    TOKEN *exprv[MAX_EXPR];
char **largv;

// before we turn the argv list over to wildexp lets do 2
// things 1. check to see if we have a wildcard expression
// 2.  if we have wild card, then make copies of the parameters
// upto but not includeing the wildcard.
    for (i = 1; i < argc; i++) {
	switch (i) {
	case 1:
	    memset(b1, 0, 32);
	    strcpy(b1, argv[1]);
	    break;
	case 2:
	    memset(b2, 0, 32);
	    strcpy(b2, argv[2]);
	    break;
	case 3:
	    memset(b3, 0, 32);
	    strcpy(b3, argv[3]);
	    break;
	case 4:
	    memset(b4, 0, 32);
	    strcpy(b4, argv[4]);
	    break;
	case 5:
	    memset(b5, 0, 32);
	    strcpy(b5, argv[5]);
	    break;
	default:
	    break;
	}
	if (strchr(argv[i], '*') || strchr(argv[i], '!')
	    || strchr(argv[i], '?')) {
	    bcount = i;
	    break;
	}
    }
    largv = wildexp(&argc, &argv);
// now put back the mangled start parameters
//
    for (i = 1; i < bcount; i++) {
	switch (i) {
	case 1:
	    largv[1] = (char *)&b1;
	    break;
	case 2:
	    largv[2] = (char *)&b2;
	    break;
	case 3:
	    largv[3] = (char *)&b3;
	    break;
	case 4:
	    largv[4] = (char *)&b4;
	    break;
	case 5:
	    largv[5] = (char *)&b5;
	    break;
	}
    }
    i = 1;
    if (argc < 2) {
	pr_usage();
	xabort();
    }
    if (*largv[i] == '-') {
	expand_sw(largv[i++]);

	if (i == argc) {
	    pr_usage();
	    xabort();
	}
    }

    if ((exprc = get_expr(exprv, MAX_EXPR, &largv[i++])) == 0) {
	pr_usage();
	xabort();
    }
// when we get here, the input source can be
// a list of files, or console
//
    numfiles = argc - i;
// numfiles is a counter for how many files are in the 
// command input list.
//

    do {
	if (numfiles) {
	    stream = fopen(largv[i], "r");
	    if (stream == NULL) {
		fprintf(stderr, "Can't open %s\n", largv[i]);
		continue;
	    }
	} else {
	    stream = stdin;
	}
	count = 0;
	linenum = 1;
	while (fgets(line, MAXLINE, stream)) {
	    if (yflag) // in cp/m land, always go to upper case
		stoupper(line);
	    for (j = exprc; --j >= 0;) {
		if (matchs(line, exprv[j], 0)) {
		    count++;
		    pr_match(linenum, line, largv[i], 1, numfiles);
		} else {
		    pr_match(linenum, line, largv[i], 0, numfiles);
		}
		linenum++;
		cntrl_c();
	    }
	    if (lflag && count)
		break;
	}
	pr_count(numfiles, largv[i], count);
	fclose(stream);
    } while (++i < argc);
    xabort();
}

void pr_count(fcount, fname, count)
int fcount, count;
char *fname;
{
    TRACE("pr_count");
    if (!cflag)
	return;

    if (fcount > 1)
	printf("%-12s: ", fname);
}

void pr_match(int linenum, char *line, char *fname, int match,
	      int numfiles)
{
    char buf[80];
    TRACE("pr_match");
//if(match)
//printf("%d %s %s %d %d\n",
//linenum,line,fname,match,numfiles);
    if (cflag)
	return;

    if ((vflag && !match) || (!vflag && match)) {
	if (!hflag && ((numfiles > 1) || lflag))
	    printf("%s%s", fname, lflag ? "\n" : ":");
	if (nflag)
	    printf("%03d:", linenum);
	if (!lflag)
	    printf("%s", line);
    }
}

void pr_usage()
{
    fprintf(stderr, "Dr Dobbs GREP ported by Jay Cotton %s,%s\n", __DATE__,
	    __TIME__);
    fprintf(stderr, "usage: grep [-cefhlnvy] [expression] <files ...>\n");
}

void xabort()
{
    exit(1);
}

void expand_sw(str)
char *str;
{
    TRACE("expand_sw");
    vflag = 0;
    cflag = 0;
    lflag = 0;
    nflag = 0;
    hflag = 0;
    fflag = 0;
    yflag = 0;

    while (*str) {
	switch (toupper(*str)) {
	case '-':
	case 'E':
	    break;
	case 'C':
	    cflag = 1;
	    break;
	case 'F':
	    fflag = 1;
	    break;
	case 'H':
	    hflag = 1;
	    break;
	case 'L':
	    lflag = 1;
	    break;
	case 'N':
	    nflag = 1;
	    break;
	case 'V':
	    vflag = 1;
	    break;
	case 'Y':
	    yflag = 1;
	    break;
	default:
	    pr_usage(3);
	    xabort();
	    break;
	}
	str++;
    }
}

int do_or(lp, expr, maxexpr)
char *lp;
TOKEN **expr;
int maxexpr;
{
    int found;
    TOKEN *pat;
    char *op;

    TRACE("do_or");
    found = 0;
    if (yflag)
	stoupper(lp);
    while (op = index(lp, OR_SYM)) {
	if (found <= maxexpr && (pat = makepat(lp, OR_SYM))) {
	    *expr++ = pat;
	    found++;
	}
	lp = ++op;

	if (pat == 0)
	    goto fatal_err;
    }
    if (found <= maxexpr && (pat = makepat(lp, OR_SYM))) {
	found++;
	*expr = pat;
    }

    if (pat == 0) {
      fatal_err:
	printf("Illegal expression: %s\n", lp);
	exit(1);
    }
    return (found);
}

int get_expr(TOKEN * expr[], int maxexpr, char **defexpr)
{
    FILE *stream;
    int count;
    char line[MAXLINE];

#ifdef DEBUG1
    int i;
#endif
    TRACE("get_expr");
    count = 0;
    if (fflag) {
	if ((stream = fopen(*defexpr, "r")) == NULL) {
	    fprintf(stderr, "Can't open %s\n", *defexpr);
	    xabort();
	}
	while ((maxexpr - count) && (fgets(line, MAXLINE, stream))) {
	    count += do_or(line, &expr[count], maxexpr - count);
	}

	fclose(stream);
    } else {
	if (count += do_or(*defexpr, &expr[count], maxexpr - count))
	    *defexpr = " ";
    }
#ifdef DEBUG1

    printf("count = %d\n", count);

    for (i = count; --i >= 0;) {
	pr_tok(expr[i]);
	printf
	    ("-------------------------------------------------------------\n");
    }
#endif
    return (count);
}

void cntrl_c()
{
    if (bdos(11, 0) && ((bdos(1, 0) & 0x7f) == 0x03))
	xabort();
}
