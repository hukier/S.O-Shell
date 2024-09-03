#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_INPUT_LENGHT 1024
#define INITIAL_ARGS_SIZE 10
#define MAX_NUM_PIPES 20

// divide un comando en argumentos y los almacena en args
int analizar_comando(char *comando, char ***args)
{
    int size = INITIAL_ARGS_SIZE;
    int n = 0;

    *args = malloc(size * sizeof(char *));
    if (*args == NULL)
    {
        perror("Error de malloc.");
        exit(EXIT_FAILURE);
    }

    char *token = strtok(comando, " \n");
    while (token != NULL)
    {
        if (n >= size)
        {
            size *= 2;
            *args = realloc(*args, size * sizeof(char *));
            if (*args == NULL)
            {
                perror("Error de realloc.");
                exit(EXIT_FAILURE);
            }
        }
        (*args)[n++] = token;
        token = strtok(NULL, " \n");
    }

    (*args)[n] = NULL; // terminar el arreglo de argumentos con NULL
    return n;
}

// ejecuta un comando
void ejecutar_comando(char *comando)
{
    char **args;
    int num_parametros = analizar_comando(comando, &args);

    // ejecutar el comando
    if (execvp(args[0], args) == -1)
    {
        perror("Error al ejecutar el comando.");
        exit(EXIT_FAILURE);
    }

    // liberar memoria
    free(args);
}

// ejecuta comandos conectados por pipes (tuberías)
void ejecutar_comandos_con_pipes(char *comandos[], int num_comandos)
{
    int pipe_fd[MAX_NUM_PIPES][2];
    pid_t pid;

    // crear las tuberías necesarias
    for (int i = 0; i < num_comandos - 1; i++)
    {
        if (pipe(pipe_fd[i]) == -1)
        {
            perror("Error al crear la tubería.");
            exit(EXIT_FAILURE);
        }
    }

    // iterar sobre cada comando para crear un proceso hijo que lo ejecute
    for (int i = 0; i < num_comandos; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            perror("Error al crear el Proceso-Hijo.");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {

            // si no es el primer comando, redirige la entrada estándar
            if (i > 0)
            {
                dup2(pipe_fd[i - 1][0], STDIN_FILENO);
            }

            // y si no es el último, redirige la salida estándar
            if (i < num_comandos - 1)
            {
                dup2(pipe_fd[i][1], STDOUT_FILENO);
            }

            // cerrar todas las tuberías en el proceso hijo para que no haya fugas
            for (int j = 0; j < num_comandos - 1; j++)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }

            // ejecutar el comando actual
            ejecutar_comando(comandos[i]);
            perror("Error al ejecutar el comando.");
            exit(EXIT_FAILURE);
        }
    }

    // cerrar todas las tuberías en el proceso padre
    for (int i = 0; i < num_comandos - 1; i++)
    {
        close(pipe_fd[i][0]);
        close(pipe_fd[i][1]);
    }

    // espera a que todos los procesos hijos terminen
    for (int i = 0; i < num_comandos; i++)
    {
        wait(NULL);
    }
}

// la shell
int main()
{
    char entrada[MAX_INPUT_LENGHT];
    char *comandos[MAX_NUM_PIPES + 1];

    while (1)
    {
        printf("NuestraShell:$ ");
        fflush(stdout);

        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            if (feof(stdin))
            {
                break; //(Ctrl+D)
            }
            perror("Error al leer la entrada.");
            exit(EXIT_FAILURE);
        }

        entrada[strcspn(entrada, "\n")] = '\0'; // elimina el salto de línea al final

        if (strcmp(entrada, "exit") == 0)
        {
            break;
        }

        int num_comandos = 0;
        comandos[num_comandos] = strtok(entrada, "|");

        while (comandos[num_comandos] != NULL && num_comandos < MAX_NUM_PIPES)
        {
            comandos[++num_comandos] = strtok(NULL, "|");
        }

        for (int i = 0; i < num_comandos; i++)
        {
            comandos[i] = comandos[i] + strspn(comandos[i], " ");
        }

        if (num_comandos > 1)
        {
            ejecutar_comandos_con_pipes(comandos, num_comandos);
        }
        else
        {
            pid_t pid = fork();

            if (pid == 0)
            {
                ejecutar_comando(comandos[0]);
            }
            else if (pid < 0)
            {
                perror("Error al crear el Proceso-Hijo.");
            }
            else
            {
                wait(NULL);
            }
        }
    }

    return 0;
}