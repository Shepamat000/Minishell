//P2-SSOO-23/24

//  MSH created by Matthew Shepard, 2024

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

/* myhistory */

/* myhistory */

struct command
{
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;

};

int history_size = 20;
struct command * history;
int head = 0;
int tail = 0;
int n_elem = 0;
// Acc variable (stored in environmental variable)
int acc = 0;

void free_command(struct command *cmd)
{
    if((*cmd).argvv != NULL)
    {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++)
        {
            for (argv = *(*cmd).argvv; argv && *argv; argv++)
            {
                if(*argv){
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd) {
    
    int num_commands = 0;
    while(argvv[num_commands] != NULL){
        num_commands++;
    }

    for(int f=0;f < 3; f++)
    {
        if(strcmp(filev[f], "0") != 0)
        {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else{
            strcpy((*cmd).filev[f], "0");
        }
    }

    

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands - 1; i++)
    {
        int args= 0;
        while( argvv[i][args] != NULL ){
            args++;
        }

        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j < args; j++) {
            if (argvv[i][j] != NULL) {
                (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]), sizeof(char));
                strcpy((*cmd).argvv[i][j], argvv[i][j] );
            }
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{


	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}


	/*********************************/

	char ***argvv = NULL;
	int num_commands;
	history = (struct command*) malloc(history_size * sizeof(struct command));
	int run_history = 0;

	while (1) 
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history) {
            run_history=0;
        }
        else {
            // Prompt 
            write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

            // Get command
            //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
            executed_cmd_lines++;
            if( end != 0 && executed_cmd_lines < end) {
                command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
            }
            else if( end != 0 && executed_cmd_lines == end)
                return 0;
            else
                command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/

	   if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
			else {
				/* Print command (debug)  */

				 //print_command(argvv, filev, in_background); 
                getCompleteCommand(argvv, 0);
                int hasInternalCommand = 0;

                // Internal commands

                // Myhistory Internal Command
                if (strcmp(argv_execvp[0], "myhistory") == 0) {
                    // Child
                    if (fork() == 0) {
                        // My history with index argument
                        if (argv_execvp[1] != NULL) {

                            int index;

                            // Error message if index is out bounds
                            if (index > tail) {
                                printf("ERROR: Command not found\n");
                                return -1;
                            }

                            for(int i=0; i < strlen(argv_execvp[1]); i++){
                               if (argv_execvp[1][i] - '0' < 10) index = index * 10 + (argv_execvp[1][i] - '0' );
                               else {
                                    printf("Please enter a valid number");
                                    return -1;
                               }
                            }

                            n_elem = index;
                            run_history = 1;
                        } 
                        else {
                            // No argument myhistory command
                             for (int i = 0; i < tail; i++) {
                                fprintf(stderr, "%d ", i);
                                int num_commands = 0;
                                while(history[i].argvv[num_commands] != NULL){
                                    int num_args = 0;
                                    if (num_commands != 0) fprintf(stderr, "| ");
                                    while(history[i].argvv[num_commands][num_args] != NULL){
                                        fprintf(stderr, "%s ", history[i].argvv[num_commands][num_args]);
                                        num_args++;
                                    }
                                    num_commands++;
                                }
                                // Print input redirection
                                if (strcmp(history[i].filev[0], "0") != 0) {
                                    fprintf(stderr, "< %s ", history[i].filev[0]);
                                }
                                // Print output redirection
                                if (strcmp(history[i].filev[1], "0") != 0) {
                                    fprintf(stderr, "> %s ", history[i].filev[1]);
                                }
                                // Print error output redirection
                                if (strcmp(history[i].filev[2], "0") != 0) {
                                    fprintf(stderr, "!> %s ", history[i].filev[2]);
                                }

                                if (history[i].in_background) {
                                    fprintf(stderr, "& ");
                                }
                                fprintf(stderr, "\n");
                            }
                            return 0;
                        }
                    // Parent
                    } else {
                        if (wait(&status) == -1) {
                            perror("Error during wait!");
                        }
                        hasInternalCommand = 1;
                    }
                }

                // My calc command.  Must come after my history so that running a mycalc command from history works
                if (strcmp(argv_execvp[0], "mycalc") == 0) {

                    // For piping acc value
                    int fdacc[2];
                    pipe(fdacc);

                    // Child
                    if (fork() == 0) {
                        // Mycalc internal function logic
                        if (argv_execvp[1] != NULL && argv_execvp[2] != NULL && argv_execvp[3] != NULL) {
                            int num1 = 0;
                            int num2 = 0; 
                            int result = 0;
                            int negative = 0;

                            // Convert argone to a number
                            int length = 0;
                            length = strlen(argv_execvp[1]);
                            for(int i=0; i < length; i++){
                                if (argv_execvp[1][i]=='-') negative = 1;
                                else num1 = num1 * 10 + (argv_execvp[1][i] - '0' );
                            }
                            if (negative) num1 = num1 * -1;

                            negative = 0;
                            // Convert argone to a number
                            length = strlen(argv_execvp[3]);
                            for(int i=0; i < length; i++){
                                if (argv_execvp[3][i]=='-') negative = 1;
                                else num2 = num2 * 10 + (argv_execvp[3][i] - '0' );
                            }   
                            if (negative) num2 = num2 * -1;

                            int result2;

                            // Add and acc logic
                            if (strcmp(argv_execvp[2], "add") == 0) {
                                result = num1 + num2;
                                acc = acc + result;
                                fprintf(stderr, "[OK] %d + %d = %d; Acc %d\n", num1, num2, result, acc);
                            }

                            // Multiplication Logic
                            else if (strcmp(argv_execvp[2], "mul") == 0) {
                                result = num1 * num2;
                                fprintf(stderr, "[OK] %d * %d = %d\n", num1, num2, result);
                            }

                            // Division logic
                            else if (strcmp(argv_execvp[2], "div") == 0) {
                                result = num1/num2;
                                fprintf(stderr, "[OK] %d / %d = %d", num1, num2, result);
                                result = num1 % num2;
                                fprintf(stderr, "; Remainder %d\n", result);
                            } else {
                                // Error if unacceptable operation argument 
                                printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                                return -1;
                            }

                            // Save new acc to pipe (Will only matter if add)
                            if (write(fdacc[1], &acc, sizeof(acc)) == -1) {
                                perror("Error during write");
                                return -1;
                            };
                            close(fdacc[1]);
                            return result;
                        } else {
                            printf("The structure of the command is mycalc < operand_1 > <add / mul / div > < operand_2 >");
                            return -1;
                        }
                        return 0;
                    // Parent
                    } else {
                        if (wait(&status) == -1) {
                            printf("Error during wait!");
                            return -1;
                        }
                        // Read from pipe
                        close(fdacc[1]);
                        if (read(fdacc[0], &acc, sizeof(acc))==-1) {
                            perror("Error during read");
                            return -1;
                        }
                        close(fdacc[0]);
                        hasInternalCommand = 1;
                    }
                }
                // If run history is true and provided command exists, replace current command with the one in history;
                if (run_history > 0) {
                    if (history[n_elem].argvv != NULL) {
                        argvv = history[n_elem].argvv;
                        hasInternalCommand = 0;
                        getCompleteCommand(argvv, 0);
                        fprintf(stderr, "Running command %d\n", n_elem);
                    } else {
                        printf("ERROR: Command not found\n");
                        return -1;
                    }
                }

                // History logic only if command is not empty, otherwise some very, very odd errors start occuring.
                if (strlen(argvv[0][0]) > 0) {
                    // If history is full, remove oldest and slide other elements back one
                    if (tail == history_size - 1) {
                        for (int i = 1; i < history_size; i++) {
                            history[i - 1] = history[i];
                        }
                    }
                    // Store in history.
                    if (tail < history_size) store_command(argvv, filev, in_background, &history[tail]);
                    // Increment history index
                    if (tail < history_size - 1) {
                        tail++;
                }
                    
                // Do not try to run commands through system if internal command is found
                if (hasInternalCommand == 0) {
                    /**/
                    int pid = fork();

                    switch (pid) {
                        case -1:
                            perror("Error in fork!");
                            return -1;
                        case 0:
                            // Execute commands

                            // Input redirection.  0660 to allow owner and user r/w access
                            if(strcmp(filev[0], "0") != 0) {
                                close (STDIN_FILENO);
                                if (open(filev[0], O_CREAT) == -1) {
                                    if (chmod(filev[1], 0660) == -1) {
                                        perror("Error setting file permissions!");
                                        return -1;
                                    }
                                    printf ("\n Error opening input file.  Returning to shell. \n");
                                    return -1;
                                }
                            }

                            // Output redirection
                            if(strcmp(filev[1], "0") != 0) {
                                close (STDOUT_FILENO);
                                if (creat(filev[1], 0660 | O_CREAT) == -1) {
                                    printf ("\n Error opening/creating output file.  Returning to shell. \n");
                                    return -1;
                                }
                            }

                            // Standard Error redirection
                            if(strcmp(filev[2], "0") != 0) {
                                close (STDERR_FILENO);
                                if (creat(filev[2], 0660 | O_CREAT) == -1) {
                                    printf ("\n Error opening/creating error output file.  Returning to shell. \n");
                                    return -1;
                                }
                            }
                        
                    

                            // Only one command, no need for piping logic
                            if (command_counter == 1) {
                                getCompleteCommand(argvv, 0);
                                execvp(argv_execvp[0], argv_execvp);
                                perror ("Error in command execution!");
                                return 0;
                            } 
                        
                            int fd[2];
                            pipe(fd);
                            int fdSecond[2];
                            pipe(fdSecond);
                            int counter = 1;
                            
                            // Pipe commands
                            if (fork() == 0) {
                                // Child
                                close (STDOUT_FILENO);
                                dup(fd[1]);
                                close(fd[0]);
                                getCompleteCommand(argvv, 0);
                                execvp(argv_execvp[0], argv_execvp);
                                perror ("Error in command execution!");
                            } else {
                                // Parent
                                if (wait(&status) == -1) {
                                        perror("Error during wait!");
                                }
                                
                                // Only two commands
                                if (command_counter == 2) {
                                    close (STDIN_FILENO);
                                    dup(fd[0]);
                                    close(fd[1]);
                                    getCompleteCommand(argvv, command_counter - 1);
                                    execvp(argv_execvp[0], argv_execvp);
                                    perror ("Error in command execution!");
                                // More than two commands
                                } else {
                                    // Child
                                    if (fork() == 0) {
                                        close (STDOUT_FILENO);
                                        dup(fdSecond[1]);
                                        close (STDIN_FILENO);
                                        dup(fd[0]);
                                        close(fdSecond[0]);
                                        close(fd[1]);
                                        getCompleteCommand(argvv, 1);
                                        execvp(argv_execvp[0], argv_execvp);
                                        perror ("Error in command execution!");

                                    // Parent
                                    } else {
                                        close (STDIN_FILENO);
                                        dup(fdSecond[0]);
                                        close(fdSecond[1]);
                                        close(fd[0]);
                                        close(fd[1]);
                                        getCompleteCommand(argvv, command_counter - 1);
                                        execvp(argv_execvp[0], argv_execvp);
                                        perror ("Error in command execution!");
                                        // Make sure to close file if we are done using it
                                        close (STDOUT_FILENO);
                                    }

                                }
                            }
                            return 0;

                        default:
                            // Wait unless running in background
                            if (!in_background) {
                                if (wait(&status) == -1) {
                                    perror("Error during wait!");
                                }
                            } else {
                                printf("[%d]\n", pid);
                            }

                    }
                }
                
                            
            }
       }
        }

        if (run_history) return 0;
        

        // Set account environment variable
        char envtemp[20];
        sprintf(envtemp, "%d", acc);
        setenv("Acc", envtemp, 1);
	}
	
	return 0;

}
