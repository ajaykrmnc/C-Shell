#include <stdio.h>
// fprintf()
// printf()
// stderr()
// getchar()
// perror()

#include <stdlib.h>
// malloc()
// realloc()
// free()
// exit()
// execvp()
// EXIT_SUCCESS, EXIT_FAILURE
#include <sys/wait.h>
// waitpid() and associated macros

#include <unistd.h>
// chdir() 
// fork()
// exec()
// pid_t

#include <string.h>
// strcmp()
// strtok()


/*
Function declaration for builting shell commands
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
List of builtin commands, followed by their corresponding functions
*/
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};
/*
This line of code defines an array of function pointers in C,
specifically pointers to functions that take a char ** (pointer to a pointer
to a character, typically representing an array of strings)
as an argument and return an int.
*/


int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}
/*
  Builtin function implementations.
*/

/**
   @brief Builtin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to\"cd\"\n");
    }else {
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    int i;
    printf("Ajay Kumar's LSH\n");
    printf("Type program names and arguments, and hit enter\n");
    printf("The following are built in: \n");

    for(i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other program \n");
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

#define LSH_RL_BUFSIZE 1024

// Reading a line
char *lsh_read_line(void){
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * bufsize);
    int c;
    if(!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        // if we hit EOF, replace it with a null character and return
        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;
        if(position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = (char *)realloc(buffer, bufsize);
            if(!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Parsing the line
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line){
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = (char **)malloc(bufsize * sizeof(char *));
    char *token;
    if(!tokens){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;
        if(position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            token = (char *)realloc(token, bufsize * sizeof(char));
            if(!token) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}
int lsh_launch(char **args) {
    pid_t pid, wid;
    int status;
    pid = fork();
    if(pid == 0) {
        // child process
        if(execvp(args[0], args) == -1){
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0) {
        // Error Forking
        perror("lsh");
    }else {
        // Parent Process
        do {
            wid = waitpid(pid, &status, WUNTRACED);
        }while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}



int lsh_execute (char ** args) {
    int i;
    if(args[0] == NULL) {
        return 1;
    }
    for(i = 0; i < lsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}



/**
   @brief Loop getting input and executing it.
*/

void lsh_loop(void){
    char *line;
    char **argv;
    int status;

    do{
        printf("> ");
        line = lsh_read_line();
        argv = lsh_split_line(line);
        status = lsh_execute(argv);
        free(line);
        free(argv);
    }while(status);
}
/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */

int main(int argc, char **argv){
    // Load config files, if any
    // run command loop
    lsh_loop();

    // Perform any shutdown/clenup
    return EXIT_SUCCESS;
}