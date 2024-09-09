    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <errno.h>

    #define MAX_LONGITUD_ENTRADA 1024
    #define MAX_NUM_PIPES 20
    #define MAX_ARGUMENTOS 150
    #define MAX_FAVS 100

    // Estructura para guardar los comandos favoritos
    typedef struct {
        char *comandos[MAX_FAVS];
        int num_comandos;
        char archivo_favoritos[256];
    } Favoritos;

    Favoritos favoritos = {.num_comandos = 0};

    // Función que divide un comando en argumentos
    int parsear_comando(char *comando, char **argumentos)
    {
        int n = 0;
        while ((argumentos[n] = strsep(&comando, " ")) != NULL)
        {
            if (*argumentos[n] != '\0')
                n++;
            if (n >= MAX_ARGUMENTOS)
                break;
        }
        return n;
    }

    // Función para ejecutar el comando
    void ejecutar_comando(char *comando)
    {
        char *argumentos[MAX_ARGUMENTOS + 1]; // incrementa el tamaño del array para soportar 150 argumentos
        int num_parametros = parsear_comando(comando, argumentos);
        if (execvp(argumentos[0], argumentos) == -1)
        {
            perror("Error al ejecutar el comando.");
            exit(EXIT_FAILURE);
        }
    }

    // Función para ejecutar los comandos con pipes
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
                perror("Error al ejecutar el comando.");
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

    // Función para agregar un comando a favoritos
    void agregar_a_favoritos(char *comando)
    {
        if (favoritos.num_comandos >= MAX_FAVS)
        {
            printf("Lista de favoritos llena.\n");
            return;
        }
        // Verifica si el comando ya está en favoritos
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            if (strcmp(favoritos.comandos[i], comando) == 0)
            {
                return; // no lo agrega si ya está
            }
        }
        favoritos.comandos[favoritos.num_comandos] = strdup(comando);
        favoritos.num_comandos++;
    }

    // Función para verificar si un comando está en favoritos
    int esta_en_favoritos(char *comando)
    {
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            if (strcmp(favoritos.comandos[i], comando) == 0)
            {
                return 1;
            }
        }
        return 0;
    }

    // Función para mostrar los comandos en favoritos
    void mostrar_favoritos()
    {
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            printf("%d: %s\n", i + 1, favoritos.comandos[i]);
        }
    }

    // Función para eliminar comandos de favoritos por su número
    void eliminar_favoritos(int nums[], int num_nums)
    {
        for (int i = 0; i < num_nums; i++)
        {
            int index = nums[i] - 1;
            if (index >= 0 && index < favoritos.num_comandos)
            {
                free(favoritos.comandos[index]);
                for (int j = index; j < favoritos.num_comandos - 1; j++)
                {
                    favoritos.comandos[j] = favoritos.comandos[j + 1];
                }
                favoritos.num_comandos--;
            }
        }
    }

    // Función para buscar un comando en favoritos
    void buscar_favorito(char *cmd)
    {
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            if (strstr(favoritos.comandos[i], cmd) != NULL)
            {
                printf("%d: %s\n", i + 1, favoritos.comandos[i]);
            }
        }
    }

    // Función para guardar los favoritos en un archivo
    void guardar_favoritos()
    {
        FILE *archivo = fopen(favoritos.archivo_favoritos, "w");
        if (!archivo)
        {
            perror("Error al abrir el archivo de favoritos para guardar.");
            return;
        }
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            fprintf(archivo, "%s\n", favoritos.comandos[i]);
        }
        fclose(archivo);
    }

    // Función para cargar los favoritos desde un archivo dado
    void cargar_favoritos(char *ruta)
    {
        FILE *archivo = fopen(ruta, "r");
        if (!archivo)
        {
            perror("Error al abrir el archivo de favoritos para cargar.");
            return;
        }

        char linea[MAX_LONGITUD_ENTRADA];
        while (fgets(linea, sizeof(linea), archivo))
        {
            linea[strcspn(linea, "\n")] = '\0'; 
            agregar_a_favoritos(linea);
        }
        fclose(archivo);
        printf("Favoritos cargados desde %s.\n", ruta);
    }

    // Función para crear el archivo de favoritos
    void crear_archivo_favoritos(char *ruta)
    {
        strncpy(favoritos.archivo_favoritos, ruta, sizeof(favoritos.archivo_favoritos) - 1);
        favoritos.archivo_favoritos[sizeof(favoritos.archivo_favoritos) - 1] = '\0';
        FILE *archivo = fopen(ruta, "w");
        if (!archivo)
        {
            perror("Error al crear el archivo de favoritos.");
        }
        else
        {
            fclose(archivo);
            printf("Archivo de favoritos creado: %s\n", ruta);
        }
    }

    // Función para ejecutar un comando favorito
    void ejecutar_favorito(int num)
    {
        if (num > 0 && num <= favoritos.num_comandos)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                ejecutar_comando(favoritos.comandos[num - 1]);
            }
            else if (pid < 0)
            {
                perror("Error al crear el proceso hijo.");
            }
            else
            {
                wait(NULL);
            }
        }
        else
        {
            printf("Número de comando inválido.\n");
        }
    }

    // Función para borrar todos los favoritos
    void borrar_favoritos()
    {
        for (int i = 0; i < favoritos.num_comandos; i++)
        {
            free(favoritos.comandos[i]);
        }
        favoritos.num_comandos = 0;
    }

    // Funcion para el comando de recordatorio

    void setrecordatorio(int segundos, const char *mensaje) {
        sleep(segundos);
        printf("\nRecordatorio: %s\n", mensaje);
    }

    // Función principal (main)
    int main()
    {
        char entrada[MAX_LONGITUD_ENTRADA];
        char *comandos[MAX_NUM_PIPES + 1]; // aquí se almacenan los comandos separados

        while (1)
        {
            printf("nuestraShell:$ ");
            fflush(stdout);

            if (fgets(entrada, sizeof(entrada), stdin) == NULL)
            {
                perror("Error al leer la entrada.");
                exit(EXIT_FAILURE);
            }

            entrada[strcspn(entrada, "\n")] = '\0';

            if (strlen(entrada) == 0)
            {
                continue;
            }

            if (strcmp(entrada, "exit") == 0)
            {
                break;
            }

            // manejo del recordatorio (parte 2.2)
            if (strncmp(entrada, "set recordatorio", 16) == 0)
            {
                char *tiempo_str = strtok(entrada + 17, " ");
                char *mensaje = strtok(NULL, "\"");

                if (tiempo_str && mensaje)
                {
                    int tiempo = atoi(tiempo_str);
                    if (tiempo > 0)
                    {
                        printf("Recordatorio programado en %d segundos: %s\n", tiempo, mensaje);
                        pid_t pid = fork();

                        if (pid == 0) //proceso hijo
                        {
                            setrecordatorio(tiempo, mensaje);
                            exit(0);
                        }
                        else if (pid < 0)
                        {
                            perror("Error al crear el proceso hijo.");
                        }
                        // el proceso padre continúa la ejecución normal de la shell
                    }
                    else
                    {
                        printf("Tiempo inválido para el recordatorio.\n");
                    }
                }
                else
                {
                    printf("Sintaxis incorrecta. Use: set recordatorio <tiempo> \"<mensaje>\"\n");
                }
                continue;
            }

            // manejo del comando "favs" (parte 2.1)
            if (strncmp(entrada, "favs", 4) == 0)
            {
                char *subcomando = strtok(entrada + 5, " ");

                if (strcmp(subcomando, "crear") == 0)
                {
                    char *ruta = strtok(NULL, " ");
                    crear_archivo_favoritos(ruta);
                }
                else if (strcmp(subcomando, "mostrar") == 0)
                {
                    mostrar_favoritos();
                }
                else if (strcmp(subcomando, "borrar") == 0)
                {
                    borrar_favoritos();
                }
                else if (strcmp(subcomando, "eliminar") == 0)
                {
                    char *nums_str = strtok(NULL, ",");
                    int nums[MAX_FAVS];
                    int num_count = 0;

                    while (nums_str != NULL && num_count < MAX_FAVS)
                    {
                        nums[num_count++] = atoi(nums_str);
                        nums_str = strtok(NULL, ",");
                    }

                    eliminar_favoritos(nums, num_count);
                }
                else if (strcmp(subcomando, "buscar") == 0)
                {
                    char *cmd = strtok(NULL, " ");
                    buscar_favorito(cmd);
                }
                else if (strcmp(subcomando, "guardar") == 0)
                {
                    guardar_favoritos();
                }
                else if (strcmp(subcomando, "cargar") == 0)
                {
                    char *ruta = strtok(NULL, " ");
                    if (ruta)
                    {
                        cargar_favoritos(ruta);
                    }
                    else
                    {
                        printf("Por favor, especifique un archivo para cargar.\n");
                    }
                }

                else if (strcmp(subcomando, "ejecutar") == 0)
                {
                    int num = atoi(strtok(NULL, " "));
                    ejecutar_favorito(num);
                }
                continue;
            }

            // manejo de pipes y comandos normales (parte 1)
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
                ejecutar_comandos_pipe(comandos, num_comandos);
            }
            else
            {
                if (!esta_en_favoritos(comandos[0]))
                {
                    pid_t pid = fork();
                    if (pid == 0)
                    {
                        ejecutar_comando(comandos[0]);
                    }
                    else if (pid < 0)
                    {
                        perror("Error al crear el proceso hijo.");
                    }
                    else
                    {
                        int status;
                        wait(&status);

                        if (status == 0)
                        {
                            agregar_a_favoritos(comandos[0]);
                        }
                    }
                }
            }
        }

        return 0;
    }
