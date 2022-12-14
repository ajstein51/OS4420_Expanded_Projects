%{
#include <stdio.h>
#include <stdarg.h>
#include "bash.h"

/* some internal files generated by bison */
int yylex();
void yyerror(const char *s,...);
void yywarn(const char *s,...);

struct args {
    char *arg; 
    struct args *next;
};
struct redirection{
	char *infilename; // for in files
	char *outfilename; // for outfiles
	char *errfile;
	char *erroutappend;
	char *outappend;
	struct redirection *nextred;
};


// saw in class
// struct file_redir{ 
// 	char *file[NUM_FD_SUPPORTED];
// 	char append[NUM_FD_SUPPORTED];
// }


/* include debugging code, in case we want it */
#define YYDEBUG 1

int lines = 1;
static int synerrors = 0;

int numinput = 0, numoutput = 0;

%}


%union	{ /* the types that we use in the tokens */
    char *string;
    struct command *pcmd;
    struct args *pargs;
	struct redirection *rdr;
    int number;

	// saw in class

	// struct {short fd; char append;} otype;
}


%token <char> EOLN PIPE INPUT OTHERCHARS
%token <number> OUTPUT OUTPUT_APPEND ERROUTPUT_APPEND ERROUT
%token <string> WORD

/*


// %type <pcmd> line 

/* saw in class 
type <pcmd> line cmd pipechain
type <redir> optredir
tope <pargs> arg args
*/
// %type <pcmd> cmd 
// %type <pcmd> pipechain
// %type <redir> opdtreir
%type <pcmd> line cmd 
%type <pargs> arg args
%type <rdr> optdir

%% 	/* beginning of the parsing rules	*/
input	: lines
	|
  	;

lines	: oneline
	| oneline lines
	;

oneline : line
		{ 
			if (synerrors != 0)
				synerrors = 0; // reset the error counter for the next line
			else{
				doline($1);
			}
		}
	  eoln
 	| eoln	/* blank line, do nothing */
	| error eoln
	/* if we got an error on the line, don't call the C program */
	;

eoln	: EOLN
		{ ++lines; }
	;

/* you need to finish the rest of this... */
cmd : WORD args
	{
		struct command *pcmd = (struct command *) malloc(sizeof(struct command));
		struct args *pargs = (struct args *) malloc(sizeof(struct args));

		// get the first arg and the cmd then increment argc to 1
		pcmd -> cmd = $1;
		pcmd ->argv[0] = pcmd -> cmd;
		pcmd->argc = 1;

		// go through the pargs list and put into pcmd
		for(pargs = $2; pargs; pargs = pargs -> next){
			pcmd->argv[pcmd->argc]=pargs->arg;
			++pcmd->argc;
		}

		numinput = 0, numoutput = 0;
		$$ = pcmd;
	}
	| WORD args optdir
	{
		// load up pcmd with pargs and rdr
		
		struct command *pcmd = (struct command *) malloc(sizeof(struct command));
		struct args *pargs = (struct args *) malloc(sizeof(struct args));
		struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));

		// get the first arg and the cmd then increment argc to 1
		pargs -> arg = $1;
		pcmd -> cmd = pargs -> arg;
		pcmd ->argv[0] = pcmd -> cmd;
		pcmd->argc = 1;

		// make structures = the argument structures
		pargs = $2;
		rdr = $3;

		if(rdr -> nextred != NULL){
			if(rdr -> nextred -> infilename != NULL)
				pcmd -> infile = rdr -> nextred -> infilename;
			else if(rdr -> nextred -> outfilename != NULL)
				pcmd -> outfile = rdr -> nextred -> outfilename;
			else if(rdr -> nextred -> errfile != NULL)
				pcmd -> errfile = rdr -> nextred -> errfile;
			else if(rdr -> nextred -> erroutappend != NULL)
				pcmd -> error_append = rdr -> nextred -> erroutappend;
			else if(rdr -> nextred -> outappend != NULL)
				pcmd -> output_append = rdr -> nextred -> outappend;
		}
		if(rdr -> infilename != NULL)
			pcmd -> infile = rdr -> infilename;

		if(rdr -> outfilename != NULL)
			pcmd -> outfile = rdr -> outfilename;
		if (rdr -> outappend != NULL)
			pcmd -> output_append = rdr -> outappend;
		if(rdr -> errfile != NULL)
			pcmd -> errfile = rdr -> errfile;
		if (rdr -> erroutappend != NULL)
			pcmd -> error_append = rdr -> erroutappend;

		if(numinput >= 2 || numoutput >= 2){
			synerrors += 1;
			numinput = 0, numoutput = 0;
		}

		// load up pcmd with the pargs
		for(pargs = $2; pargs; pargs = pargs -> next){
			pcmd->argv[pcmd->argc]=pargs->arg;
			++pcmd->argc;
		}

		numinput = 0, numoutput = 0;
		$$ = pcmd;
	}
	;

optdir : INPUT WORD
		{
			numinput += 1;
			struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			rdr -> infilename = $2;
			$$ = rdr;
		}
		| INPUT WORD optdir
		{
			numinput += 1;
			// struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			// rdr -> infilename = $2;
			// rdr -> nextred = $3;
			// $$ = rdr;
			$3 -> infilename = $2;
			$$ = $3;
		}
		| OUTPUT WORD
		{
			numoutput += 1;
			struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			rdr -> outfilename = $2;
			$$ = rdr;

		}
		| OUTPUT WORD optdir
		{
			numoutput += 1;
			// struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			// rdr -> outfilename = $2;
			// rdr -> nextred = $3;
			// $$ = rdr;
			$3 -> outfilename = $2;
			$$ = $3;
		}
		| ERROUT WORD
		{
			struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			rdr -> errfile = $2;
			$$ = rdr;
		}
		| ERROUT WORD optdir
		{
			// struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			// rdr -> errfile = $2;
			// rdr -> nextred = $3;
			// $$ = rdr;
			$3 -> errfile = $2;
			$$ = $3;
		}
		| OUTPUT_APPEND WORD
		{
			struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			rdr -> outappend = $2;
			$$ = rdr;
		}
		| OUTPUT_APPEND WORD optdir
		{
			// struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			// rdr -> outappend = $2;
			// rdr -> nextred = $3;
			// $$ = rdr;
			$3 -> outappend = $2;
			$$ = $3;
		}
		| ERROUTPUT_APPEND WORD
		{
			struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			rdr -> erroutappend = $2;
			$$ = rdr;
		}
		| ERROUTPUT_APPEND WORD optdir
		{
			// struct redirection *rdr = (struct redirection *) (malloc(sizeof(struct redirection)));
			// rdr -> erroutappend = $2;
			// rdr -> nextred = $3;
			// $$ = rdr;
			$3 -> erroutappend = $2;
			$$ = $3;
		}
		;

line	: cmd 
		{$$ = $1;}
		// WORD PIPE line
		| cmd PIPE line
		{
			$1 -> nextpipe = $3;
			$1 -> outfile = "PIPE";
			$3 -> infile = "PIPE";

		}
	;

/* stuff word into command */
arg : WORD
	{
		struct args *pargs = (struct args *) malloc(sizeof(struct args));
		pargs -> arg = $1;
		pargs -> next = NULL;
		$$ = pargs;
	}
;
args : arg args
	{ 
		$$ = $1; 
		$1 -> next = $2;
	}
	| 
	{
		$$ = NULL;
	}
	;	

%%

void
yyerror(const char *error_string, ...)
{
    va_list ap;
    int line_nmb(void);

    FILE *f = stdout;

    va_start(ap,error_string);

    ++synerrors; 

    fprintf(f,"\nError on line %d: ", lines);
    vfprintf(f,error_string,ap);
    fprintf(f,"\n");
    va_end(ap);
}
/*
**** Monday Notes: *****
$1 = WORD

Redirection:
- line is a cmd or WORD args optionalredirection
		cmd : WORD args optiondir
		;
		optiondir : redir optdir
		{
			$$ = $1
			$$ -> next = $2
		}
			| 
		; in here we would built the list
		redir : input WORD
		{
			set something = $1
			set something = $2
			$$ = that thing
		}
			| OUTPUT WORD
			| 
			| ERRORAPP WORD
			| APPEND WORD
			... other type of redirection
			... something with $1
			...
		; we do the same thing in this case that we would do in the arguments
		make the structure and pass it up


WORD = command
args = link lists of arguments
optdir = link list of redirection

struct redir node{
	int append; // if we are appending or not (tell if its > or >>) essentially a bool
	^ maybe dont use that
	char *filename; //
	int token; // enum type (input, output, or error)
	struct redir node *next;
}

***** WED ******
Questions: When to use pargs vs pcmd

	pipechain : pip

***** FRI *****
arg : word
{
	parg = (struct args *) malloc(sizeof())
	parg -> arg = $1
	parg -> next = NULL
	$$ = parg
}
args : arg args
{ $$ = $1; $1 -> next = $2;}
| 
{$$ = NULL;}


arg is a linked list, has 1 node
args is a linked list, has many nodes
arg 1 node will link to the start of args list
	to do this we do  (in the arg args)
	$$ = $1;
	$$ -> next = $2;
	| 
	$$ = NULL;

*******
For redirection
walk through the linked list if we found an input/output file we make char outfile/infile
to a book then check in bash.c
*************
if we found an error:


******* MONDAY *********
Question: Where to look for pipes in the parser
line : cmd
{

}
| cmd PIPE token
{

}

// can put input and other stuff in the line : cmd rule
or put it in:
cmd : WORD optionalagrs(optargs) optionaldirection(optdir)
*/