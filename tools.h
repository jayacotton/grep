/*#define CPM*/
#define NUL 0
#define CR 0x0d
#define SUB 0x1a 
#define CPMEOF SUB

#define islower(c)  ('a'<= (c) && (c) <=  'z')
#define toupper(c) (islower(c) ? (c) - ('a' - 'A') : (c))

#define BOL '^'
#define EOL '$'
#define ANY '.'
#define LITCHAR 'L'
#define ESCAPE 92 
#define CCL '['
#define CCLEND ']'
#define NEGATE '^'
#define NCCL '!'
#define CLOSURE '*'
#define OR_SYM '|'

struct token
{
char tok;
char lchar;
char *bitmap;
struct token *next;
};
#define TOKEN struct token 

#define TOKSIZE 8 
#define MAXSTR 132

/*
extern wildexp();
extern stoupper();
extern matchs();
extern stoupper();
extern makepat();
extern pr_tok();
*/

