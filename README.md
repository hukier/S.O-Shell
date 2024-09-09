# Shell - Sistemas Operativos

## Integrantes

- Javier Torres Ortiz
- Martin Gonzalez cifuentes
- Felipe Tilleria Morales

## Descripción

Este es un proyecto de Sistemas Operativos que consiste en una Shell. La Shell es una interfaz de línea de comandos que permite al usuario interactuar con el sistema operativo ejecutando comandos.

## Compilación

Para compilar el proyecto, sigue los siguientes pasos:

1. Abre una terminal en la ubicación del proyecto.
2. Ejecuta el siguiente comando:
   ```
   gcc -o Shell Shell.c
   ```

## Ejecución

Para ejecutar la Shell, sigue los siguientes pasos:

1. Abre una terminal en la ubicación del proyecto.
2. Ejecuta el siguiente comando:
   ```
   ./Shell
   ```

## Ejecución de comandos

### Favs

1. **Crear** : favs crear, creara una lista de los comandos favoritos agregados hasta el momento en un archivo txt.
   ```
   $ favs crear <nombre del archivo>.txt
   ```
2. **Mostrar** : favs mostrar, Mostrara todos los comandos guardados en la lista.
   ```
   $ favs mostrar
   ```
3. **Ejecutar** : favs ejecutar, Ejecutara el comando guardado seleccionado.
   ```
   $ favs ejecutar <numero del comando>
   ```
4. **Eliminar** : favs eliminar, Eliminara el comando seleecionado de la lista.
   ```
   $ favs eliminar <numero del comando>
   ```
5. **Guardar** : favs guardar, Guardara el archivo txt con los comandos favoritos.
   ```
   $ favs guardar
   ```
6. **Cargar** : favs cargar, Cargara el archivo txt con los comandos guardados.
   ```
   $favs cargar <nombre del archivo>.txt
   ```

### Recordatorios

1. El comando recortadorios permite realizar una pausa activa hacia la shell con un tiempo determinado por el usuario

```
$ set recordatorio <tiempo> "<mensaje>"
```
