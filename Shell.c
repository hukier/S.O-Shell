#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_LENGHT 1024
#define MAX_NUM_PIPES 20      // número máximo de pipes por comando
#define INITIAL_ARGS_SIZE 150 // número máximo de argumentos que definimos para un comando

// Función que divide un comando en argumentos
int parsear_comando(char *comando, char **argumentos)
{
    int n = 0;
    while ((argumentos[n] = strsep(&comando, " ")) != NULL)
    {
        if (*argumentos[n] != '\0')
            n++;
        if (n >= INITIAL_ARGS_SIZE)
            break; // para evitar desbordamientos
    }
    return n;
}

// Función para ejecutar el comando
void ejecutar_comando(char *comando)
{
    char *argumentos[INITIAL_ARGS_SIZE + 1]; // Incrementa el tamaño del array para soportar 150 argumentos
    int num_parametros = parsear_comando(comando, argumentos);

    // ejecuta el comando
    if (execvp(argumentos[0], argumentos) == -1)
    {
        perror("Error al ejecutar el comando");
        exit(EXIT_FAILURE);
    }
}

// Función (con concurrencia) para ejecutar los comandos con pipes
void ejecutar_comandos_pipe(char *comandos[], int num_comandos)
{
    int pipe_fd[MAX_NUM_PIPES][2];
    pid_t pid;

    // crea los pipes necesarios
    for (int i = 0; i < num_comandos - 1; i++)
    {
        if (pipe(pipe_fd[i]) == -1)
        {
            perror("Error al crear la tubería.");
            exit(EXIT_FAILURE);
        }
    }

    // se itera sobre cada comando para crear un proceso hijo que lo ejecute
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
            // y si no es el último comando, redirige la salida estándar
            if (i < num_comandos - 1)
            {
                dup2(pipe_fd[i][1], STDOUT_FILENO);
            }
            // cerrar todos los pipes en el proceso hijo para que no haya fugas
            for (int j = 0; j < num_comandos - 1; j++)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }

            // se ejecuta el comando actual
            ejecutar_comando(comandos[i]);
            perror("Error al ejecutar el comando."); // 7. el comando ingresado no existe asi que da error
            exit(EXIT_FAILURE);
        }
    }

    // cerrar todos los pipes en el proceso padre
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

int main()
{
    char entrada[MAX_INPUT_LENGHT];
    char *comandos[MAX_NUM_PIPES + 1]; // aquí se almacenan los comandos separados

    while (1)
    {                              // bucle infinito para mantener la shell en ejecución
        printf("nuestraShell:$ "); // prompt
        fflush(stdout);

        // lee la entrada por teclado
        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            perror("Error al leer la entrada.");
            exit(EXIT_FAILURE);
        }

        entrada[strcspn(entrada, "\n")] = '\0'; // elimina el salto de línea al final

        // verifica si la entrada fue vacia para reimprimir el prompt
        if (strlen(entrada) == 0)
        {
            continue;
        }

        // si se ingresó "exit", se termina el programa
        if (strcmp(entrada, "exit") == 0)
        {
            break;
        }

        // divide la entrada en comandos con el "|" como separador
        int num_comandos = 0;
        comandos[num_comandos] = strtok(entrada, "|");
        while (comandos[num_comandos] != NULL && num_comandos < MAX_NUM_PIPES)
        {
            comandos[++num_comandos] = strtok(NULL, "|");
        }

        // elimina espacios extra en los comandos
        for (int i = 0; i < num_comandos; i++)
        {
            comandos[i] = comandos[i] + strspn(comandos[i], " ");
        }

        // ejecución de comandos, si hay más de uno se ejecuta con pipes
        if (num_comandos > 1)
        {
            ejecutar_comandos_pipe(comandos, num_comandos);
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