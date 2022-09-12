/*
 * Headstart for Ostermann's shell project
 *
 * Shawn Ostermann -- Sept 9, 2021
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "bash.h"
#include <string.h>


void yyerror(const char *s,...);
void yywarn(const char *s,...);

// int pipecount = 1, curpipe = 1;
int linecount = 1, curpipe = 1, nexpipe = 1;

int main(int argc, char *argv[]){

    // debug mode:
    if (argc == 2 && strcmp(argv[1],"-d") == 1)
        yydebug = 1;  /* turn on ugly YACC debugging */
    else
        yydebug = 0;  /* turn off ugly YACC debugging */

    /* parse the input file */
    yyparse();

    exit(0);
}

void dopipe(struct command *nextpipe){
    if(nextpipe != NULL){

        // outputs
        printf("\n\nCommand Name: '%s'", nextpipe -> argv[0]);
        if(yydebug == 1)
            printf("size of argc:\t %d \n", nextpipe -> argc);
        
        for(int x = 0; x < nextpipe -> argc; x++){
            printf("\n\targv[%d]:\t'%s'",x, nextpipe -> argv[x]);
        }

        if(nextpipe -> infile != NULL && strcmp(nextpipe -> infile, "PIPE") == 0){
            printf("\nstdin:\t\t'%s%d'", nextpipe -> infile, curpipe);
            curpipe = nexpipe;
        }
        // else if (nextpipe -> infile != NULL && strcmp(nextpipe -> infile, "PIPE") == 1) // not pipe
        //     yyerror("PIPE and Input file");
        else
            printf("\nstdin:\t\t'<undirected>'");
        if(nextpipe -> outfile != NULL && strcmp(nextpipe -> outfile, "PIPE") == 0){
            printf("\nstdout:\t\t'%s%d'", nextpipe -> outfile, nexpipe);
            curpipe = nexpipe, nexpipe++;
        }
        // else if(nextpipe -> outfile != NULL && strcmp(nextpipe -> outfile, "PIPE") == 1) // not a pipe
        //     yyerror("PIPE and Output file");
        if(nextpipe -> output_append != NULL)
            printf("\nstdout:\t\t'%s' (append)", nextpipe -> output_append);
        else if (nextpipe -> outfile == NULL && nextpipe -> output_append == NULL)
            printf("\nstdout:\t\t'<undirected>'");
        if(nextpipe -> errfile != NULL)
            printf("\nstderr:\t\t'%s'", nextpipe -> errfile);
        if(nextpipe -> error_append != NULL)
            printf("\nstderr:\t\t'%s' (append)", nextpipe -> error_append);
        else if (nextpipe -> errfile == NULL && nextpipe -> error_append == NULL)
            printf("\nstderr:\t\t'<undirected>'");

        // recurse to the nextpipe until its NULL
        dopipe(nextpipe -> nextpipe);
    }
    else
        return;
}

void doline(struct command *pcmd)
{ // cant have an input file if we are in pipe
    // error check
    if(pcmd -> outfile != NULL && pcmd -> output_append != NULL){ // there is both an output file and outputappent
        yyerror("Too many outfiles");
        return;
    }   
    else if(pcmd -> errfile != NULL && pcmd -> error_append != NULL){ // there is both an error file and error append
        yyerror("Too many error redirections");
        return;
    }
    // warning check
    if(pcmd -> outfile != NULL && pcmd -> errfile != NULL)
        printf("There is a warning");

    // put line count
    printf("\n\n============ line %d ============\n", linecount);
    
    // go through the list and start printing everything else out
    printf("Command Name: '%s'", pcmd->cmd);
    if(yydebug == 1)
        printf("size of argc:\t %d \n", pcmd -> argc);
    for(int i = 0; i < pcmd -> argc; i++){
        printf("\n\targv[%d]:\t'%s'",i, pcmd->argv[i]);
    }

    // if we got a pipe
    if(pcmd -> nextpipe != NULL){
        // first pass so do this
        printf("\nstdin:\t\t'<undirected>'");
        if(pcmd -> nextpipe -> outfile != NULL && strcmp(pcmd -> nextpipe -> outfile, "PIPE") == 0){
            printf("\nstdout:\t\t'%s%d'", pcmd -> nextpipe -> outfile, nexpipe);
            curpipe = nexpipe, nexpipe++;
        }
        if(pcmd -> nextpipe -> output_append != NULL)
            printf("\nstdout:\t\t'%s' (append)", pcmd -> nextpipe -> output_append);
        else if (pcmd -> nextpipe -> outfile == NULL && pcmd -> nextpipe -> output_append == NULL)
            printf("\nstdout:\t\t'<undirected>'");
        if(pcmd -> nextpipe -> errfile != NULL)
            printf("\nstderr:\t\t'%s'", pcmd -> nextpipe -> errfile);
        if(pcmd -> nextpipe -> error_append != NULL)
            printf("\nstderr:\t\t'%s' (append)", pcmd -> nextpipe -> error_append);
        else if (pcmd -> nextpipe -> errfile == NULL && pcmd -> nextpipe -> error_append == NULL)
            printf("\nstderr:\t\t'<undirected>'");
        dopipe(pcmd -> nextpipe);
    }
    else{ // redirection and there isnt a pipe
        // infile
        if(pcmd -> infile != NULL)
            printf("\nstdin:\t\t'%s'", pcmd -> infile);
        else
            printf("\nstdin:\t\t'<undirected>'");
        
        // outfile/append
        if(pcmd -> outfile != NULL)
            printf("\nstdout:\t\t'%s'", pcmd -> outfile);
        else if(pcmd -> output_append != NULL)
            printf("\nstdout:\t\t'%s' (append)", pcmd -> output_append);
        else if (pcmd -> outfile == NULL && pcmd -> output_append == NULL)
            printf("\nstdout:\t\t'<undirected>'");

        // error/append
        if(pcmd -> errfile != NULL)
            printf("\nstderr:\t\t'%s'", pcmd -> errfile);
        else if(pcmd -> error_append != NULL)
            printf("\nstderr:\t\t'%s' (append)", pcmd -> error_append);
        else if (pcmd -> errfile == NULL && pcmd -> error_append == NULL)
            printf("\nstderr:\t\t'<undirected>'");
    }
    // for always clean output
    printf("\n");
    linecount++, curpipe = 1, nexpipe = 1;
}