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
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

void yyerror(const char *s, ...);
void yywarn(const char *s, ...);

// int pipecount = 1, curpipe = 1;
int linecount = 1, curpipe = 1, nexpipe = 1, ydebug;

// env var
char *home, *prompt, *path, *debugenv;

// variable stuff
struct var vars;

// void variableinit(struct command *pcmd);
// void quoteinit(struct command *pcmd);
// int varlookup(char *name);
// int exportvarlookup(char *name);
// void update_env_var(char *name, struct command *pcmd);
// void exportvar(struct command *pcmd);

int main(int argc, char *argv[]){
    // debug mode:
    if (argc == 2 && strcmp(argv[1],"-d") == 1)
        ydebug = 1;  /* turn on ugly YACC debugging */
    else
        ydebug = 0;  /* turn off ugly YACC debugging */

    
    // inits for env vars
    putenv("PROMPT=shell2>");
    putenv("DEBUG=OFF");

    // env vars
    home = getenv("HOME"), path = getenv("PATH"), prompt = getenv("PROMPT"), debugenv = getenv("DEBUG");
    vars.varname[0] = "DEBUG", vars.varvalue[0] = "OFF";

    // init prompt
    printf("%s", prompt);

    // init for variable stuff
    vars.placeholder = 1;

    // parse and do the entire prog
    yyparse();


    exit(0);
}

void doline(struct command *pcmd){
    // if we got debug
    if(strcmp(debugenv, "ON") == 0) 
        debug(pcmd);
//******************************************************************************************
//                              to do: 1, run commands with absolute path names (trivial)
    if (fork() == 0 && strcmp(pcmd->cmd, "cd") != 0 && strstr(pcmd->cmd, "/bin/") != NULL){ // isnt a cd and it has an absolute path, strstr is a string subsequence checker.
        execv(pcmd->cmd, pcmd->argv);
        perror(pcmd->cmd);
        _exit(0);
    }
    else
        wait(0);
//******************************************************************************************
//                              to do: 2 add stdin/stdout/stderr redirection (simple - use class examples)
    /** standard operations for doing this:
     * We check if we got a redirection
     * We open the file to be redirected
     * we dup the stdout/stdin/stderr to fd
     * close the stdout/stdin/stderr 
     * exec(....)
     * close the fd
     */
    if(pcmd -> infile != NULL){
        int fd; /* file descriptors */
        fd = open(pcmd->infile, O_RDONLY, 0); // open file
        if (fd == -1) { 
            perror(pcmd->infile); 
            exit(0); 
        }
        if (fork() == 0) {  /* Child process */
            if(dup2(fd, 0) == -1) /* bind stdin to fd */
                perror("dup2");
            if(close(fd)  == -1) // close the regular tdin
                perror("close");
            if(execv(pcmd->cmd, pcmd->argv)  == -1) // redirect output to fd
                perror("execv");
            perror(pcmd->cmd);
            _exit(0); // exit child
        }
        wait(NULL); 
        if(close(fd)  == -1)
            perror("close");

    } // end of infile
    if(pcmd -> outfile != NULL){ // working
        int fd; /* file descriptors */
        fd = open(pcmd->outfile, O_WRONLY | O_CREAT, 1);
        if (fd == -1) { 
            perror(pcmd->outfile); 
            exit(0); 
        }
        if (fork() == 0) {  /* Child process */
            if(dup2(fd, 1) == -1) /* bind stdout to fd */
                perror("dup2");
            if(close(fd)  == -1)
                perror("close");
            if(execv(pcmd->cmd, pcmd->argv)  == -1)
                perror("execv");
            perror(pcmd->cmd);
            _exit(0); // exit child
        }
        wait(NULL); 
        if(close(fd)  == -1)
            perror("close");

    }
    else if(pcmd -> errfile != NULL){
        int fd; /* file descriptors */
        fd = open(pcmd->errfile, O_WRONLY | O_CREAT, 2);
        if (fd == -1) { 
            perror(pcmd->errfile); 
            exit(0); 
        }
        if (fork() == 0) {  /* Child process */
            if(dup2(fd, 2) == -1) /* bind err to fd */
                perror("dup2");
            if(close(fd)  == -1)
                perror("close");
            if(execv(pcmd->cmd, pcmd->argv)  == -1)
                perror("execv");
            perror(pcmd->cmd);
            _exit(0); // exit child
        }
        wait(NULL); 
        if(close(fd)  == -1)
            perror("close");
    }
    else if(pcmd -> output_append != NULL){
        int fd; /* file descriptors */
        fd = open(pcmd->output_append, O_WRONLY | O_CREAT | O_APPEND, 1); // open in append
        if (fd == -1) { 
            perror(pcmd->output_append); 
            exit(0); 
        }
        if (fork() == 0) {  /* Child process */
            if(dup2(fd, 1) == -1) /* bind stdout append to fd */
                perror("dup2");
            if(close(fd)  == -1)
                perror("close");
            if(execv(pcmd->cmd, pcmd->argv)  == -1)
                perror("execv");
            perror(pcmd->cmd);
            _exit(-1); // exit child
        }
        wait(NULL); 
        if(close(fd)  == -1)
            perror("close");
    }
    else if(pcmd -> error_append != NULL){
        int fd; /* file descriptors */
        fd = open(pcmd->error_append, O_WRONLY | O_CREAT | O_APPEND, 2);
        if (fd == -1) { 
            perror(pcmd->error_append); 
            exit(0); 
        }
        if (fork() == 0) {  /* Child process */
            if(dup2(fd, 2) == -1) /* bind err append to fd */
                perror("dup2");
            if(close(fd)  == -1)
                perror("close");
            if(execv(pcmd->cmd, pcmd->argv)  == -1)
                perror("execv");
            perror(pcmd->cmd);
            _exit(0); // exit child
        }
        wait(NULL); 
        if(close(fd)  == -1)
            perror("close");
    }
//******************************************************************************************
//                            task 3, using path to find commands
    // regular cd
    if(strcmp(pcmd->cmd,"cd") == 0 && pcmd->argc == 1){
        if (chdir(home) == -1) // root, same thing as just cd
            perror(pcmd->cmd);
    } // cd with a parth or something
    if (strcmp(pcmd->cmd, "cd") == 0 && pcmd->argc == 2){
        if (chdir(pcmd->argv[1]) == -1) // we got a cd with a path
            perror(pcmd->cmd);
    }
    
    // for debug
    if(strcmp(debugenv, "ON") == 0) 
        printf("\nthis is path: %s\n", path);

    // position of where we were last at for later
    int pos2 = 0;
    for(int i = 0; i <= strlen(path);i++){
        if(path[i] == ':'){
            // we got to a : now we load up a new string with that path and search it for the cmd

            // make char the correct size so we can load
            char *tmppath = malloc(strlen(path) + 1);

            // load the parsed path
            strncpy(tmppath, &path[pos2], i - pos2); // copy from the start to the :

            // update the position for the next time
            pos2 = i + 1; // get to the next spot after a :

            if(strcmp(debugenv, "ON") == 0) 
                printf("\nThis is tmppath: %s\n", tmppath);

            // makes the char the size of what we need
            char *completecmd = malloc(strlen(tmppath) + strlen(pcmd->cmd) + 1);

            // builds the complete cmd
            sprintf(completecmd, "%s/%s", tmppath, pcmd->cmd);

            if(strcmp(debugenv, "ON") == 0) 
                printf("\nThis is the completecmd: %s \n", completecmd);

            // check access, if it does we execute it
            if(access(completecmd, F_OK | X_OK) != -1){
                // run the cmd
                if(fork() == 0){
                    execv(completecmd, pcmd-> argv);
                    perror(completecmd);
                    _exit(0);
                }
                wait(0);
                free(completecmd);
                free(tmppath);
                break;
            }
        }
        // if we are at the end, need this for an edge case
        if(i + 1 == strlen(path)){
            // we got to a : now we load up a new string with that path and search it for the cmd
            
            // make char the correct size so we can load
            char *tmppath = malloc(strlen(path) + 1);

            // load the parsed path
            strncpy(tmppath, &path[pos2], (i + 1) - pos2); // copy from the start to the :

            // update the position for the next time
            pos2 = i + 1; // get to the next spot after a :

            if(strcmp(debugenv, "ON") == 0) 
                printf("\nThis is tmppath: %s\n", tmppath);

            // makes the char the size of what we need
            char *completecmd = malloc(strlen(tmppath) + strlen(pcmd->cmd) + 1);

            // builds the complete cmd
            sprintf(completecmd, "%s/%s", tmppath, pcmd->cmd);

            if(strcmp(debugenv, "ON") == 0) 
                printf("\nThis is the completecmd: %s \n", completecmd);

            // check access, if it does we execute it
            if(access(completecmd, F_OK | X_OK) != -1){
                // run the cmd
                if(fork() == 0){
                    execv(completecmd, pcmd-> argv);
                    perror(completecmd);
                    _exit(0);
                }
                wait(0);
                free(completecmd);
                free(tmppath);
                break;
            }   
        }
    } // end of for
//******************************************************************************************
//                          4. implement variable assignment
    if (vars.init != NULL){
        // we got a new variable, add it to the list
        if(pcmd->argc > 1)
            printf("\nIncorrect syntax %d", pcmd->argc);
        else
            variableinit(pcmd);
    }
    if(vars.quoteinit != NULL){
        if(pcmd->argc > 1)
            printf("\nIncorrect syntax");
        else
            quoteinit(pcmd);
    }
    // check if we got DEBUG, PROMPT, HOME, export for env vars
    if(vars.placeholder >= 1){
        // loop through the var names
        for(int i = 0; i < vars.placeholder; i++){
            // look for anything for env vars
            if((strcmp(vars.varname[i], "DEBUG") == 0 && strcmp(vars.varvalue[i], "''") == 0) || strcmp(pcmd->cmd, "DEBUG=''") == 0 || strcmp(pcmd->cmd, "DEBUG=OFF") == 0){
                putenv("DEBUG=OFF");
                debugenv = getenv("DEBUG");
            }
            // else if(strcmp(vars.varname[i], "DEBUG") == 0 && strcmp(vars.varvalue[i], "''") != 0){
            //     putenv("DEBUG=ON");
            //     debugenv = getenv("DEBUG");
            // }// debug
            if(strcmp(vars.varname[i], "PROMPT") == 0){
                putenv(pcmd->cmd);
                prompt=getenv("PROMPT");
            } // prompt
            if(strcmp(vars.varname[i], "HOME") == 0){
                putenv(pcmd->cmd);
                home=getenv("HOME");
            } // exported vars
            if(strcmp(pcmd->cmd, "export") == 0 || strcmp(pcmd->cmd,"EXPORT") == 0){
                exportvar(pcmd);
            } // end of export
        }
    }

    // print out all variables
    // if(strcmp(getenv("DEBUG"), "ON") == 0){
    //     printf("\nthis is placeholder %d\n",vars.placeholder);
    //     if(vars.placeholder >= 1){
    //         for(int i = 0; i < vars.placeholder; i++){
    //             printf("\nvariable: %s value: %s\n", vars.varname[i], vars.varvalue[i]);
    //         }
    //     }
    // }
//******************************************************************************************
    // prompt
    printf("%s", prompt);
}

// checks if the var exists or not, if it does we update it
void exportvar(struct command *pcmd){
    // look up what the variable is and get its location
    int loc = exportvarlookup(pcmd->argv[1]);

    // its not currently an env var
    if(loc == -1){
        // check if its actually a var
        int varcheck = varlookup(pcmd->argv[1]);

        if(varcheck == -1){
            // its not a variable in general
            printf("\nThis is not a variable\n");
        }
        else{
            // it is a variable that currently exists. Add it as a env

            vars.exname[vars.exhold] = vars.varname[varcheck];
            vars.exval[vars.exhold] = vars.varvalue[varcheck];

            // combine the string for the orginal method of XYZ=ABC
            char *tmp = malloc(strlen(vars.exname[vars.exhold]) + strlen(vars.exval[vars.exhold]) + 1);

            // add togeher the pieces to get the original cmd
            sprintf(tmp, "%s=%s", vars.exname[vars.exhold], vars.exval[vars.exhold]);
            
            // if(strcmp(debugenv, "ON") == 0) 
            printf("\nthis is tmp %s", tmp);

            // make an env var
            putenv(tmp);

            // if(strcmp(debugenv, "ON") == 0) {
            printf("\nthis is the env var: %s", getenv(pcmd->argv[1]));
            
            // inc exported counter
            vars.exhold++;
        }
    }
    else{
        printf("\nThis variable has already been exported\n");
    }
    
    return;
}

// env var exists so we need to update it
void update_env_var(char *name, struct command *pcmd){
    // check if it is a env var
    for(int i = 0; i < vars.exhold; i++){
        if(strcmp(vars.exname[i], name) == 0){
            // it is a env var so we update it
            // need to find its location
            int loc = varlookup(name);

            // need to put together the full cmd
            char *fullcmd = malloc(strlen(vars.varname[loc]) + strlen(vars.varvalue[loc]) + 1);
            sprintf(fullcmd, "%s=%s", vars.varname[loc], vars.varvalue[loc]);

            // if(strcmp(debugenv, "ON") == 0) 
            printf("\nthis is tmp %s", fullcmd);

            // make an env var
            putenv(fullcmd);

            // if(strcmp(debugenv, "ON") == 0) {
            printf("\nthis is the env var: %s", getenv(vars.varname[loc]));
        }
    }
    return;
}

/**
 * Does the same as variableinit but removes the quotes
 */ 
void quoteinit(struct command *pcmd){
    for(int i = 0; i<= strlen(vars.quoteinit); i++){
        if(vars.quoteinit[i] == '='){
            char *name = malloc(i);
            strncpy(name, &vars.quoteinit[0], i);

            // check if it exists
            int check = varlookup(name);
            if(check == -1) // it doesnt exists, add it
                vars.varname[vars.placeholder] = name;

            // now we know everything after this is the arguemnt
            char *vararg = malloc(strlen(vars.quoteinit) - i + 1);
            strncpy(vararg, &vars.quoteinit[i + 2], strlen(vars.quoteinit) - i - 3);

            if(check == -1) // doesnt exist, add it
                vars.varvalue[vars.placeholder] = vararg;
            else{
                // it does exist, replace it
                vars.varvalue[check] = vararg;
                
                // if it does exist and its also a env var
                update_env_var(name, pcmd);
            }
            if(check == -1) // doesnt exist so we added it, inc counter
                vars.placeholder++;

            // clear vars.init
            vars.quoteinit = NULL;
            return;
        }
    }
}

// will see if the variable already exists if it doesnt itll return -1 otherwise itll return the location
int varlookup(char *name){
    if(vars.expansion == -1){
        // flag to tell us not to expand 
        vars.expansion = 0;
        return -1;
    }
    for(int i = 0; i < vars.placeholder; i++){
        if(strcmp(vars.varname[i], name) == 0){
            // we got a match return the location
            return i;
        }
    }
    // no match
    return -1;
}

// same thing as above but for exported vars
int exportvarlookup(char *name){
    for(int i = 0; i < vars.exhold; i++){
        if(strcmp(vars.exname[i], name) == 0){
            // we got a match return the location
            return i;
        }
    }
    // no match
    return -1;
}

/**
 * This is loading vars varname and varvalue with a char array that points to the char *name and char *varargs below
 */ 
void variableinit(struct command *pcmd){
    // parse the variable and split it into 2 groups

    // check if the variable already exists, if it does we just update it

    for(int i = 0; i <= strlen(vars.init); i++){
        if(vars.init[i] == '='){
            char *name = malloc(i);
            strncpy(name, &vars.init[0], i);

            // check if it exists
            int check = varlookup(name);
            if(check == -1){
                // doesnt exist add it
                vars.varname[vars.placeholder] = name;
            }

            // now we know everything after this is the arguemnt
            char *vararg = malloc(strlen(vars.init) - i + 1);
            strncpy(vararg, &vars.init[i + 1], strlen(vars.init) - i);

            if(check == -1){
                // doesnt exist add it
                vars.varvalue[vars.placeholder] = vararg;
            }
            else{
                // it does exist
                vars.varvalue[check] = vararg;

                // if it does exist and its also a env var
                update_env_var(name, pcmd);
            }
            
            if(check == -1){
                // update total 
                vars.placeholder++;
            }

            // clear vars.init
            vars.init = NULL;
            return;
        }
    } // end of for
}

// give a line we check if there are vars then replace
char *var_replace(char *line){
    // go through the line and see if there is a var
    char *retline = malloc(100);
    for(int i = 0; i < strlen(line); i++){
        if(line[i] == '$'){
            // found a var possibly
            // check if its a brace one or not
            if(line[i + 1] == '{'){
                // length of the variable
                int varlength = 0;
                for(int j = i + 2; j < strlen(line); j++){
                    if(line[j] == '}'){
                        varlength = j - i - 2;
                        j = strlen(line); // exit this loop
                    }
                } // end of j
                // get variable name
                char *varname = malloc(varlength);
                strncpy(varname, &line[i + 2], varlength);
                
                // check varname for ## or %%
                int startdel = 0, enddel = 0, pos = 0, firstinst = 0;
                for(int j = 0; j < strlen(varname); j++){
                    if(varname[j] == '%' && firstinst == 0)
                        pos = j, firstinst = -1, enddel++;
                    else if(varname[j] == '%')
                        enddel++;
                    if(varname[j] == '#' && firstinst == 0)
                        pos = j, startdel++, firstinst = -1;
                    else if(varname[j] == '#')
                        startdel++;
                }
                // see if it exist
                if(enddel == 0 && startdel == 0){
                    int check = varlookup(varname);
                    if(check == -1){
                        // doesnt exist
                        retline[strlen(retline)] = line[i]; // add the char
                        retline[strlen(retline)] = '\0'; // null terminate
                    }
                    else{
                        // does exist
                        char *replacement = vars.varvalue[check];
                        for(int j = 0; j < strlen(replacement); j++){

                            retline[strlen(retline)] = replacement[j];
                        }
                        i += varlength + 2;
                        // printf("\nWe are at line[i] :  %c\n", line[i]);
                    }
                } // start and end check
                else if(enddel != 0){
                    // remove chars from the end
                    // get how many letter after the key
                    int amount = 0;
                    for(int j = pos + enddel; j < strlen(varname); j++){
                        amount++;
                    }
                    // get the var name
                    char *newname = malloc(strlen(varname) - enddel - amount);
                    strncpy(newname, &varname[0], strlen(varname) - enddel - amount);
                    
                    // does it exist
                    int check = varlookup(newname);
                    if(check == -1){
                        // doesnt exist
                        retline[strlen(retline)] = line[i]; // add the char
                        retline[strlen(retline)] = '\0'; // null terminate
                    }
                    else{
                        // get the letters after the key
                        char *letters = malloc(amount);
                        strncpy(letters, &varname[strlen(varname) - amount], amount);
                        // echo "thi ${v1%ef}"
                        // check that the letters given do match
                        char *replace = vars.varvalue[check];
                        int flag = 0, replaceidx = strlen(replace) - 1, letteridx = strlen(letters) - 1;
                        for(int j = 0; j < amount; j++){
                            printf("\nwords %c %c \n", replace[replaceidx], letters[letteridx]);
                            if(replace[replaceidx] != letters[letteridx])
                                flag = -1;
                            
                            replaceidx--, letteridx--;
                        }
                        printf("\nthis is flag %d\n", flag);
                        if(flag == 0){
                            // remove the first letters of size amount
                            char *replace = vars.varvalue[check];
                            for(int j = 0; j < strlen(replace) - amount; j++){
                                retline[strlen(retline)] = replace[j];
                            }

                            i += varlength + 2;
                        }
                        else{
                            // dont match so we do nothin
                            retline[strlen(retline)] = line[i]; // add the char
                            retline[strlen(retline)] = '\0'; // null terminate
                        }
                    } // end of else
                }
                else if(startdel != 0){
                    // remove chars from the start
                    // get how many letters are after the key
                    int amount = 0;
                    for(int j = pos + startdel; j < strlen(varname); j++){
                        amount++;
                    }
                    // get the var name
                    char *newname = malloc(strlen(varname) - startdel - amount);
                    strncpy(newname, &varname[0], strlen(varname) - startdel - amount);

                    // does it exist
                    int check = varlookup(newname);
                    if(check == -1){
                        // doesnt exist
                        retline[strlen(retline)] = line[i]; // add the char
                        retline[strlen(retline)] = '\0'; // null terminate
                    }
                    else{
                        // get the letters after the key
                        char *letters = malloc(amount);
                        strncpy(letters, &varname[strlen(varname) - amount], amount);
                        
                        // check that the letters given do match
                        char *replace = vars.varvalue[check];
                        int flag = 0;
                        for(int j = 0; j < amount; j++){
                            if(replace[j] != letters[j])
                                flag = -1;
                        }
                        if(flag == 0){
                            // remove the first letters of size amount
                            char *replace = vars.varvalue[check];
                            for(int j = amount; j < strlen(replace); j++){
                                retline[strlen(retline)] = replace[j];
                            }

                            i += varlength + 2;
                        }
                        else{
                            // dont match so we do nothin
                            retline[strlen(retline)] = line[i]; // add the char
                            retline[strlen(retline)] = '\0'; // null terminate
                        }
                    } // end of else
                } // end of deleting from the start
            } // end of if there is a bracket
            else{
                // get the length of the var
                int varlength = 0;
                for(int j = i + 1; j < strlen(line); j++){
                    // printf("this is line[j] : %c\n", line[j]);
                    if(line[j] == ' '){
                        varlength = j - i - 1;
                        j = strlen(line); // stop the loop
                    }
                    else if(j + 1 == strlen(line)){ // edge case of end of the line
                        varlength = j - i;
                        j = strlen(line);
                    }
                } // end of j
                // get the variables name
                char *varname = malloc(varlength);
                printf("\nthis is varlen : %d\n", varlength);
                strncpy(varname, &line[i + 1], varlength);
                // printf("\nthis is varlen : %s\n", varname);

                // see if it exists as a variable
                int check = varlookup(varname);
                // printf("\nWE GOT HERE %s\n", varname);
                // if it doesnt exist just load the word like usual
                if(check == -1){
                    retline[strlen(retline)] = line[i]; // add the char
                    retline[strlen(retline)] = '\0'; // null terminate
                }
                else{
                    // it does exist, we need to replace it
                    char *replacement = vars.varvalue[check];
                    for(int j = 0; j < strlen(replacement); j++){
                        // printf("\nvars value : %c", replacement[j]);
                        retline[strlen(retline)] = replacement[j];
                    }

                    i += varlength; // brings us to the space
                    printf("\n\nthis is where we are at %c\n\n", line[i]);
                }
            }
        }
        else{
            // there is no var add it to the line
            retline[strlen(retline)] = line[i]; // add the char
            retline[strlen(retline)] = '\0'; // null terminate
        }
    }// end of for i

    // return our line
    return retline;
}

void dopipe(struct command *nextpipe){
    if(nextpipe != NULL){

        // outputs
        printf("\n\nCommand Name: '%s'", nextpipe -> argv[0]);
        if(ydebug == 1)
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

void debug(struct command *pcmd)
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
    if(ydebug == 1)
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

