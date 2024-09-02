#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_INPUT_LENGTH 1024
#define INITIAL_ARGS_SIZE 10
#define MAX_NUM_PIPES 20

int parse_command(char *command, char ***args)
{
    int size = INITIAL_ARGS_SIZE;
    int n = 0;
    *args = malloc(size * sizeof(char *));
    if (*args == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char *token = strtok(command, " \n");
    while (token != NULL)
    {
        if (n >= size)
        {
            size *= 2;
            *args = realloc(*args, size * sizeof(char *));
            if (*args == NULL)
            {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        (*args)[n++] = token;
        token = strtok(NULL, " \n");
    }
    (*args)[n] = NULL; // Terminar el arreglo de argumentos con NULL

    return n;
}

void execute_command(char *command)
{
    char **args;
    int num_params = parse_command(command, &args);

    // Ejecutar el comando
    if (execvp(args[0], args) == -1)
    {
        perror("Error al ejecutar el comando");
        exit(EXIT_FAILURE);
    }

    // Liberar la memoria
    free(args);
}

void execute_pipe_commands(char *commands[], int num_commands)
{
    int pipe_fd[MAX_NUM_PIPES][2];
    pid_t pid;

    // Crear los pipes necesarios
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipe_fd[i]) == -1)
        {
            perror("Error al crear la tubería");
            exit(EXIT_FAILURE);
        }
    }

    // Iterar sobre cada comando para crear un proceso hijo que lo ejecute
    for (int i = 0; i < num_commands; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Error al crear el proceso hijo");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Si no es el primer comando, redirige la entrada estándar
            if (i > 0)
            {
                dup2(pipe_fd[i - 1][0], STDIN_FILENO);
            }
            // Si no es el último comando, redirige la salida estándar
            if (i < num_commands - 1)
            {
                dup2(pipe_fd[i][1], STDOUT_FILENO);
            }
            // Cerrar todos los pipes en el proceso hijo para que no haya fugas
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }

            // Ejecutar el comando actual
            execute_command(commands[i]);
            perror("Error al ejecutar el comando");
            exit(EXIT_FAILURE);
        }
    }

    // Cerrar todos los pipes en el proceso padre
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipe_fd[i][0]);
        close(pipe_fd[i][1]);
    }

    // Espera a que todos los procesos hijos terminen
    for (int i = 0; i < num_commands; i++)
    {
        wait(NULL);
    }
}