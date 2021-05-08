#include "tools.h"
#include "stdio.h"
#include <alloc.h>
#include <string.h>
#include "trace.h"

extern char testbit();
extern TOKEN *makepat();
extern char *stoupper();
extern void unmakepat();
extern int omatch(char **, TOKEN *, char *);
#define max(a,b) (a>b?a:b)

char *amatch(lin, pat, boln)
char *lin, *boln;
TOKEN *pat;

{

    char *bocl, *rval, *strstart;
    //  TRACE("amatch");
    if (pat == 0)
	return (0);

    strstart = lin;

    while (pat) {
	if (pat->tok == CLOSURE && pat->next) {
	    pat = pat->next;
	    bocl = lin;

	    while (*lin && omatch(&lin, pat, boln));

	    if ((pat = pat->next)) {
		while (bocl <= lin) {
		    if ((rval = amatch(lin, pat, boln))) {
			return (rval);
		    } else
			--lin;
		}
		return (0);
	    }
	} else if (omatch(&lin, pat, boln)) {
	    pat = pat->next;
	} else {
	    return (0);
	}
    }
    --lin;
    return (max(strstart, lin));
}

#ifdef DEBUG1

void delete(ch, str)
int ch;
char *str;
{
    ch &= 0xff;

    while (*str && *str != ch)
	str++;

    while (*str) {
	*str = *(str + 1);
	str++;
    }
}

void insert(ch, str)
int ch;
char *str;
{
    char *bp;

    bp = str;

    while (*str)
	str++;
    do {
	*(str + 1) = *str;
	str--;
    } while (str >= bp);
    *bp = ch;
}

void pr_line(ln)
char *ln;
{
    for (; *ln; ln++) {
	if ((' ' <= *ln) && (*ln <= '"'))
	    putchar(*ln);
	else {
	    printf("\\0x%02x", *ln);
	    if (*ln == '\n')
		putchar('\n');
	}
    }
}

void pr_tok(head)
TOKEN *head;
{
    char *str;
    int i;

    for (; head; head = head->next) {
	switch (head->tok) {
	case BOL:
	    str = "BOL";
	    break;
	case EOL:
	    str = "EOL";
	    break;
	case ANY:
	    str = "ANY";
	    break;
	case LITCHAR:
	    str = "LIBCHAR";
	    break;
	case ESCAPE:
	    str = "ESCAPE";
	    break;
	case CCL:
	    str = "CCL";
	    break;
	case CCLEND:
	    str = "CCLEND";
	    break;
	case NCCL:
	    str = "NCCL";
	    break;
	case CLOSURE:
	    str = "CLOSURE";
	    break;
	default:
	    str = "*** unknown ***";
	}

	printf("%-8s at: 0x%x, ", str, head);

	if (head->tok == CCL || head->tok == NCCL) {
	    printf("string (at 0x%x) =<", head->bitmap);
	    for (i = 0; i < 0x7f; i++)
		if (testbit(i, head->bitmap))
		    putchar(i);
	    printf(">, ");
	} else if (head->tok == LITCHAR)
	    printf("lchar = %c, ", head->lchar);

	printf("next = 0x%x\n", head->next);

    }
    putchar('\n');
}

#endif


void setbit(c, field)
int c;
char field[];
{
    TRACE("setbit");
    field[(c & 0x7f) >> 3] |= 1 << (c & 0x07);
}

char testbit(c, field)
int c;
char *field;
{
    TRACE("testbit");
    return (field[(c & 0x7f) >> 3] & (1 << (c & 0x07)));
}

char *dodash(delim, src, map)
int delim;
char *src, *map;
{
    int first, last;
    char *start;
    TRACE("dodash");
    start = src;

    while (*src && *src != delim) {
	if (*src != '-')
	    setbit(esc(&src), map);
	else if (src == start || *(src + 1) == delim)
	    setbit('-', map);
	else {
	    src++;
	    if (*src < *(src - 2)) {
		first = *src;
		last = *(src - 2);
	    } else {
		first = *(src - 2);
		last = *src;
	    }
	    while (++first <= last)
		setbit(first, map);
	    src++;
	}
    }
    return (src);
}

int esc(s)
char **s;
{
    int rval;
    char c;
    TRACE("esc");
    if (**s != ESCAPE) {
	rval = *((*s)++);
    } else {
	(*s)++;
	c = toupper(**s);
	switch (c) {
	case 0:
	    {
		rval = ESCAPE;
		break;
	    }
	case 'B':
	    {
		rval = '\b';
		break;
	    }
	case 'F':
	    {
		rval = '\f';
		break;
	    }
	case 'N':
	    {
		rval = '\n';
		break;
	    }
	case 'R':
	    {
		rval = '\r';
		break;
	    }
	case 'S':
	    {
		rval = ' ';
		break;
	    }
	case 'T':
	    {
		rval = '\t';
		break;
	    }
	default:
	    {
		rval = **s;
		break;
	    }
	}
	(*s)++;
    }
    return (rval);
}

TOKEN *getpat(arg)
char *arg;
{
    TRACE("getpat");
    return (makepat(arg, '\000'));
}

TOKEN *makepat(arg, delim)
char *arg;
int delim;
{
    TOKEN *head, *tail;
    TOKEN *ntok;
    int error, i;
    TRACE("makepat");
    if (*arg == '\0' || *arg == delim || *arg == '\n' || *arg == CLOSURE)
	return (0);

    error = 0;
    head = 0;
    tail = 0;

    for (; *arg && *arg != delim && *arg != '\n' && !error; arg++) {
	ntok = calloc(TOKSIZE, 1);
	if (ntok == 0) {
	    fprintf(stderr, "Not enough memory for pattern template\n");
	    exit(1);
	    break;
	}
	switch (*arg) {
	case ANY:
	    ntok->tok = ANY;
	    break;
	case BOL:
	    if (head == 0)
		ntok->tok = BOL;
	    else
		error = 1;
	    break;
	case EOL:
	    if (*(arg + 1) == delim || *(arg + 1) == '\0'
		|| *(arg + 1) == '\n')
		ntok->tok = EOL;
	    else
		error = 1;
	    break;
	case CCL:
	    if (*(arg + 1) == NEGATE) {
		ntok->tok = NCCL;
		arg += 2;
	    } else {
		ntok->tok = CCL;
		arg++;
	    }

	    if ((ntok->bitmap = calloc(16, 1))) {
#ifdef SOLARIS
		Lmemset(ntok->bitmap, 0, 16);
#else
		memset(ntok->bitmap, 0, 16);
#endif
		arg = dodash(CCLEND, arg, ntok->bitmap);
	    } else {
		fprintf(stderr, "Not enough memory for pat\n");
		error = 1;
	    }
	    break;
	case CLOSURE:
	    switch (tail->tok) {
	    case BOL:
	    case EOL:
	    case CLOSURE:
		return (0);
	    default:
		ntok->tok = CLOSURE;
	    }
	    break;
	default:
	    ntok->tok = LITCHAR;
	    ntok->lchar = esc(&arg);
	    --arg;
	}

	if (error) {
	    unmakepat(head);
	    return (0);
	} else if (head == 0) {
	    ntok->next = 0;
	    head = tail = ntok;
	} else if (ntok->tok != CLOSURE) {
	    tail->next = ntok;
	    ntok->next = tail;
	    tail = ntok;
	} else if (head != tail) {
	    (tail->next)->next = ntok;
	    ntok->next = tail;
	} else {
	    ntok->next = head;
	    tail->next = ntok;
	    head = ntok;
	}
    }
    tail->next = 0;
    return (head);
}

char *matchs(line, pat, ret_endp)
char *line;
TOKEN *pat;
int ret_endp;
{
    char *rval, *bptr;
    TRACE("matchs");
    bptr = line;
    while (*line) {
	if ((rval = amatch(line, pat, bptr)) == 0) {
	    line++;
	} else {
	    rval = ret_endp ? rval : line;
	    break;
	}
    }
    return (rval);
}

char *stoupper(str)
char *str;
{
    char *rval;
    TRACE("stoupper");
    rval = str;

    while (*str) {
	if ('a' <= *str && *str <= 'z')
	    *str -= ('a' - 'A');

	str++;
    }
    return (rval);
}

int omatch(linp, pat, boln)
char **linp, *boln;
TOKEN *pat;
{

    int advance;
    // TRACE("omatch");
    advance = -1;

    if (**linp) {
	switch (pat->tok) {
	case LITCHAR:
	    if (**linp == pat->lchar)
		advance = 1;
	    break;
	case BOL:
	    if (*linp == boln)
		advance = 0;
	    break;
	case ANY:
	    if (**linp != '\n')
		advance = 1;
	    break;
	case EOL:
	    if (**linp == '\n')
		advance = 0;
	    break;
	case CCL:
	    if (testbit(**linp, pat->bitmap))
		advance = 1;
	    break;
	case NCCL:
	    if (!testbit(**linp, pat->bitmap))
		advance = 1;
	    break;
	default:
	    printf("omatch: can't happen\n");
	}
    }
    if (advance >= 0)
	*linp += advance;
    return (++advance);
}

void unmakepat(head)
TOKEN *head;
{
    TOKEN *old_head;
    TRACE("unmakepat");
    while (head) {
	switch (head->tok) {
	case CCL:
	case NCCL:
	    free(head->bitmap);
	default:
	    old_head = head;
	    head = head->next;
	    free(old_head);
	    break;
	}
    }
}

char *Lindex(str, c)
char *str;
char c;
{
    TRACE("Lindex");
    while (*str) {
	if (*str++ == c) {
	    return str;
	}
    }
    return 0;
}
