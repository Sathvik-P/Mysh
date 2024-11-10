#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG 32
#define MAX_COMMANDS 3

void parse_input(char *input, char **args);
void execute_command(char **args);
void handle_pipes(char **commands);

int main() {
    char input[MAX_INPUT_SIZE];
    char *commands[MAX_COMMANDS];
    
    while (1) 
    {
        printf("$ ");
        
        // Read user input
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
            break;
        
        // Remove newline character from input
        input[strcspn(input, "\n")] = '\0';
        
        // Parse input by pipes
        char *token = strtok(input, "|");
        int cmd_count = 0;
        while (token != NULL) 
	{
		// check if input is empty
		while (isspace(*token))
			token++;
		// if input is empty, go back to prompt
		if (*token == 0)
			break;

            commands[cmd_count++] = token;
            token = strtok(NULL, "|");
        }
        commands[cmd_count] = NULL;  // Null-terminate the command array
        
        // If no input, continue
        if (cmd_count == 0) 
            continue;


        // If single command, execute normally
        if (cmd_count == 1) 
	{
            char *args[MAX_ARG];
            parse_input(commands[0], args);

	    // check if input is exit, if it is exit
	    if (strcmp(args[0], "exit") == 0)
		    exit(EXIT_SUCCESS);
            execute_command(args);
        } 
	else
            handle_pipes(commands);
    }
    return 0;
}

// Tokenize a command by spaces to get individual arguments
void parse_input(char *command, char **args) 
{
    char *token = strtok(command, " ");
    int index = 0;
    
    // fill the token with the next command
    while (token != NULL) 
    {
        args[index] = token;
        index++;
        token = strtok(NULL, " ");
    }
    args[index] = NULL;
}

// Execute a single command without pipes
void execute_command(char **args) {
    pid_t pid = fork();  // Create child process
    
    if (pid < 0) 
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0)
    {
        if (execvp(args[0], args) < 0) 
	{
            perror("Execution failed");
            exit(EXIT_FAILURE);
	}
    } 
    else   // Parent process
        wait(NULL);  // Wait for the child process to complete
}

// Handle multiple commands with pipes
void handle_pipes(char **commands) 
{
    int pipefd[2];
    int in_fd = STDIN_FILENO;  // File descriptor for input
    pid_t pid;

    for (int i = 0; commands[i] != NULL; i++) 
    {
        char *args[MAX_ARG];
        parse_input(commands[i], args);

        // If there's a next command, create a pipe so it does not go to stdout
        if (commands[i + 1] != NULL) 
	{
            if (pipe(pipefd) == -1) 
	    {
                perror("Pipe failed");
                exit(EXIT_FAILURE);
            }
        } 
	else // Last command goes to stdout
            pipefd[1] = STDOUT_FILENO;  

        pid = fork();
        if (pid < 0) 
	{
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) 
	{
	    // redirect input
            dup2(in_fd, STDIN_FILENO);

	   // if there are still more commands
            if (commands[i + 1] != NULL)
                dup2(pipefd[1], STDOUT_FILENO);  // Redirect output to pipe

            // Close unused pipe ends in the child process
            if (commands[i + 1] != NULL) 
	    {
                close(pipefd[0]);  
                close(pipefd[1]); 
            }

            // Execute the command
            if (execvp(args[0], args) < 0) 
	    {
                perror("Execution failed");
                exit(EXIT_FAILURE);
            }
        } 
	else
	{	// Parent process
            wait(NULL);  // Wait for the child process to finish
            
            // Close unused pipe ends in the parent process
            if (commands[i + 1] != NULL)
                close(pipefd[1]);  // Close the write end of the pipe

	    // reopen read end of pipe for next command
            in_fd = pipefd[0]; 
        }
    }
}

