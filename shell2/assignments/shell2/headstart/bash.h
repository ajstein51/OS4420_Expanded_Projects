/*
 * Ostermann's shell header file
 */


#define MAX_ARGS 100


/* This is the definition of a command */
struct command {
    char *cmd;
    int argc;
    char *argv[MAX_ARGS];
    char *infile;
    char *outfile;
    char *errfile;

    char *output_append;		/* boolean: append stdout? */
    char *error_append;		/* boolean: append stderr? */

    // saw in class
    char *file[100];
    char append[100];

    struct command *nextpipe;
};

struct var{
    // key value
    char *varname[100], *varvalue[100];
    // keep track of args
    int placeholder;

    // init for setting up the key/val
    char *init;
    
    // track exported vars
    char *exname[100], *exval[100];
    int exhold;

    // init for quote
    char *quoteinit;
    
    // check if we need to expand
    int expansion; 


};


/* externals */
extern int yydebug;


/* global routine decls */
void doline(struct command *pass);
void debug(struct command *pass);
int yyparse(void);
void dopipe(struct command *nextpipe);

void variableinit(struct command *pcmd);
void quoteinit(struct command *pcmd);
int varlookup(char *name);
int exportvarlookup(char *name);
void update_env_var(char *name, struct command *pcmd);
void exportvar(struct command *pcmd);
char *var_replace(char *line);

