#include "systemcalls.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "stdlib.h"
#include "sys/wait.h"
#include "stdbool.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    int ret_val;

    ret_val = system(cmd);

    return (ret_val == 0);
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);

    if(command[0][0] != '/'){
        fprintf(stderr, "the argument provided is not a full path but relative. exiting...");
        return false;
    }

    pid_t child_pid = fork();

    if(child_pid == -1){
        fprintf(stderr, "failed to call fork()");
        return false;
    }
    else if(child_pid == 0){
        execv(command[0], command);
        fprintf(stderr, "returned from execv means error");
        return false;
    }
    else {
        int status;

        if(waitpid(child_pid, &status, 0)==-1){
            fprintf(stderr, "error calling waitpid");
            return false;
        }

        if(WIFEXITED(status)){
            int exit_status = WEXITSTATUS(status);
            return (exit_status == 0);
        }
        else{
            fprintf(stderr, "child exited unexpectedly");
            return false;
        }

    }


    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

    if(command[0][0] != '/'){
        fprintf(stderr, "the argument provided is not a full path but relative. exiting...");
        return false;
    }

    pid_t child_pid = fork();

    if(child_pid == -1){
        fprintf(stderr, "failed to call fork()");
        return false;
    }
    else if(child_pid == 0){

        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            fprintf(stderr, "Failed to open output file\n");
            return false;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            fprintf(stderr, "Failed to redirect stdout\n");
            return false;
        }

        close(fd);

        execv(command[0], command);

        fprintf(stderr, "returned from execv means error");
        return false;
    }
    else {
        int status;

        if(waitpid(child_pid, &status, 0)==-1){
            fprintf(stderr, "error calling waitpid");
            return false;
        }

        if(WIFEXITED(status)){
            int exit_status = WEXITSTATUS(status);
            return (exit_status == 0);
        }
        else{
            fprintf(stderr, "child exited unexpectedly");
            return false;
        }

    }

    return true;
}
