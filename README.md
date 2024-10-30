# Sistemas Operativos

*Autora: Celia Patricio Ferrer*

*Curso: 2023-2024*

Este documento recoge una descripción de todo lo desarrollado en Sistemas Operativos.

# Indice

- [Práctica 1. Llamadas al Sistema Operativo](#práctica-1-llamadas-al-sistema-operativo)
    - [Mywc](#mywc)
    - [Myls](#myls)
    - [Myishere](#myishere)

---
---

## Práctica 1. Llamadas al Sistema Operativo

La práctica desarrollada consistió en implementar tres programas en C que replican funcionalidades de comandos básicos de Linux. Cada programa fue probado y validado comparando sus resultados con los comandos de Linux correspondientes, asegurando su correcto funcionamiento. A lo largo de la práctica, se enfrentaron retos con la configuración del entorno Linux y el uso de llamadas al sistema, lo cual permitió mejorar los conocimientos en sistemas operativos y programación en C.

### Mywc

Este programa abre un fichero pasado como parámetro y muestra por salida estándar el número de líneas, de palabras y de bytes que tiene el fichero. Su funcionamiento se asemeja al mandato `wc` sobre un fichero, mostrando las mismas salidas.

**Descripción del código**

En primer lugar, se importan las librerías necesarias que utiliza este programa. Entre ellas encontramos *stdio.h*, *fcntl.h* y *unistd.h*. Siendo la primera para mostrar por consola un mensaje con la función printf y para utilizar llamadas al sistema para abrir (*open*) y para cerrar (*close*) archivos; la segunda para activar la flag de solo lectura llamada *O_RDONLY*; y la tercera para leer de byte en byte utilizando la función *read*.

En segundo lugar, se comprueba que el número de valores pasados como parámetros sea el correcto. En caso de no ser así, el programa muestra por consola el mensaje de error correspondiente y retorna un -1, indicando que se ha producido un error.

En tercer lugar, se abre el fichero pasado como parámetro, que ocupa la segunda posición en la línea de comandos. Para abrir el fichero, se ha utilizado la llamada al sistema *open*. El fichero especidicado en el primer argumento, *argv[1]*, y especificando el permiso de solo escritura, *O_READONLY*. En caso de producirse un error en la apertura del fichero, se muestra por consola y se devuelve el valor correspondiente (-1).

A continuación, se define e inicializan todos los contadores necesarios para poder tener un conteo de los valores, que finalmente, se mostrarán por pantalla. Además, se declara una variable binaria para marcar si el carácter actual pertenece a una palabra o no. Junto a todas estas variables, también se declara otra que va a almacenar el carácter actual que el programa está leyendo.

En cuarto lugar, se lee el contenido del fichero de byte en byte. Este proceso se ha conseguido implementar utilizando otra llamada al sistema, *read*. Esta función requiere: un descriptor de fichero, una dirección de memoria donde almacenar el carácter que acaba de leer y un tamaño en bytes (concretamente, la unidad, ya que se pide que se lea el contenido de byte en byte).
Antes de la lectura, se realiza una comprobación para verificar si se produce algún error de lectura. Y seguido a esto, se realiza un bucle *while* que recoge a dicha función hasta que devuelva un 0 o un número negativo. Para que el bucle finalice cuando se termine la lectura del contenido del fichero o se produzca un error.

La ejecución completa del bucle *while*, comienza incrementando el número de bytes. Continúa haciendo una serie de comprobaciones para ver si el carácter actual que acaba de ser leído se trata de un salto de línea, una tabulación, un espacio o una letra que pertenece a una palabra. Dependiendo de este resultado, se ejecuta un código u otro.

Si se trata de un salto de línea (“\n”), se incrementa el contador de líneas y se verifica si lo anterior era una palabra, en cuyo caso, se incrementa el contador de palabras y se reinicia la variable binaria a 0, para indicar que la palabra ha terminado. Si se trata de un espacio (“ “) o una tabulación (“\t”) y además lo anterior era una palabra, se incrementa el contador de palabras y se reinicia nuevamente la variable binaria a 0. Y si se trata de cualquier otra cosa, es decir, una letra, digito o carácter especial, se pone la variable binaria a 1, para indicar que comienza una nueva palabra.

Una vez que se finaliza de ejecutar el bucle *while*, se comprueba si lo último leído era una palabra. En caso afirmativo, se incrementa el contador de palabras.

Finalmente, se muestra por consola el resultado con el formato siguiente: 
```sh
<líneas> <palabras> <bytes> <fichero>
```
Además, se cierra el fichero, comprobando que se cierra correctamente e indicando si se produce un error. En caso de que no se haya producido ningún error, para indicar que se ha finalizado la ejecución de dicho programa correctamente, el programa retorna el valor 0.

**Batería de pruebas**

En esta sección se recogen las pruebas llevadas a cabo para realizar la comprobación de dicho programa. Para obtener resultados y conclusiones, estas pruebas se han contrastado con las salidas obtenidas de ejecutar el mismo fichero con el comando `wc` en la terminal de Linux Ubuntu.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `./mywc` | Error y retorna -1 | Error y retorna -1 | No se pasa ningún fichero. |
| `./mywc f1.txt f2.txt` | Error y retorna -1 | Error y retorna -1 | Se pasan dos ficheros. |
| `./mywc f1.txt` | 2 15 79 f1.txt | 2 15 79 f1.txt | Se pasa un fichero básico. |
| `./mywc no- existe.txt` | Error y retorna -1 | Error y retorna -1 | Se pasa un fichero que no existe. |
| `./mywc fichero- vacio.txt` | 0 0 0 f1.txt | 0 0 0 f1.txt | Se pasa un fichero que está vacío. |
| `./mywc f2.txt` | 1055 5267 28448 f2.txt | 1055 5267 28448 f2.txt | Se pasa un fichero largo. |

### Myls

Este segundo programa consiste en abrir un directorio pasado como argumento (o en caso contrario, el directorio actual que se encuentra abierto) y mostrar por consola cada entrada de dicho directorio, cada una en una línea. Esta implementación coincide con el comando `ls -1`.

**Descripción del código**

En primer lugar, se importan las bibliotecas necesarias para la correcta ejecución de este programa. Entre ellas, se encuentran *stdio.h* y *unistd.h* (contiene la función *getcwd*), utilizadas y explicadas en el anterior programa. Y otras nuevas como *sys/types.h*, *dirent.h* y *string.h*. La primera para utilizar las llamadas al sistema *opendir*, *readdir* y *closedir*, es decir, para el manejo de directorios. La segunda para utilizar la estructura *dirent*, una estructura que contiene sobre un archivo en un directorio concreto. Y la tercera para utilizar la función *strcpy*, para copiar cadenas de caracteres.

A continuación, se declaran las variables principales que este programa va a utilizar. La ruta en la que se encuentra el directorio, que es un *array* de longitud igual a la máxima permitida para un *path*. El puntero que apunta a la estructura *dirent* que va a almacenar el valor de la entrada leída. Y el puntero a un objeto de tipo dir que es el identificador del directorio que se pasa como argumento o en el que se está.

En segundo lugar, se realiza la comprobación de si se pasa como argumento o no dicho directorio. Es decir, si se especifica, se comprueba que se pase un solo argumento y se copia la ruta del fichero en la variable correspondiente utilizando la función *strcpy*. Mientras que, si el contador de argumentos es igual a la unidad, significa que no se especifica ningún directorio y debe abrir el actual. Para ello, se obtiene la ruta del directorio, con *getcwd*, y se verifica que no se produzcan errores.

Una vez obtenida la ruta del directorio, en tercer lugar, se abre dicho directorio. En caso de error, se muestra el mensaje correspondiente y se devuelve el valor de -1, para indicar que se ha producido un error. A continuación, se lee el contenido del directorio con la función *readdir*, que se ejecuta en un bucle *while* hasta que deje de leer entradas, y las devuelve por salida estándar, las muestra por consola. Para ello, se accede al atributo *d_name* de la estructura *dirent*.

Finalmente, se cierra el directorio, comprobando posibles errores. En caso de encontrar alguno, se retorna el valor -1 y se muestra el mensaje correspondiente. Mientras que, si toda la ejecución del programa se completa correctamente, se retorna el valor 0 para indicar la correcta y completa ejecución de dicho programa.

**Batería de pruebas**

En esta sección se exponen las pruebas que se han realizar para hacer la comprobación de dicho programa. Para obtener resultados y conclusiones, estas pruebas se han contrastado con las salidas obtenidas de ejecutar el mismo fichero con el comando `ls -1` en la terminal de Linux Ubuntu.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `./myls dirA dirB` | Error y retorna -1 | Error y retorna -1 | Se pasan dos directorios. |
| `./myls` | probador_ssoo_p1.py <br> myls.c <br> myishere <br> myls <br> autores.txt <br> p1_tests <br> myishere.c <br> Makefile <br> mywc.c | probador_ssoo_p1.py <br> myls.c <br> myishere <br> myls <br> autores.txt <br> p1_tests <br> myishere.c <br> Makefile <br> mywc.c | No se pasa ningún argumento. |
| `./myls p1_tests` | dirA <br> . <br> f1.txt <br> dirB <br> .. | dirA <br> . <br> f1.txt <br> dirB <br> .. | Se pasa un directorio. |
| `./myls no-existe` | Error y retorna -1 | Error y retorna -1 | Se pasa un directorio que no existe. |

### Myishere

Se trata de un programa que recibe dos argumentos, el primero un nombre de un directorio y el segundo un nombre de un fichero. La función de este programa es identificar si ese directorio contiene ese fichero. En cualquier caso, se muestra el mensaje correspondiente por salida estándar, por la consola.

**Descripción del código**

En primer lugar, se incluyen las librerías necesarias que este programa necesita. Entre las que se encuentran *stdio.h*, *sys/types.h*, *dirent.h* y *string.h*, mencionadas en la descripción del programa anterior. Esta última ha sido importada para utilizar la función *strcmp*.

En segundo lugar, como los otras dos anteriores, se realiza la comprobación correspondiente para ver que se pasan el número correcto de parámetros; es decir, el nombre del programa, la ruta del directorio y la ruta del archivo. En caso de que no sea válido, se muestra por consola el error y se devuelve el número -1.

En tercer lugar, se abre el directorio pasado como segundo argumento y se guarda en un puntero a *dir*. En caso de no poder abrir el directorio, se muestra por consola el error correspondiente y el programa retorna el valor -1, indicando que se produjo un error a lo largo de la ejecución.

En cuarto lugar, se leen todas las entradas que contiene ese directorio. Para ello, se realiza de la misma manera que el programa anterior. Mediante la función *readdir*, que se guarda en el puntero a la estructura *dirent*, recogida en un bucle *while*, que se ejecuta hasta que se terminen de leer todas entradas del directorio o se haya encontrado el fichero en ese directorio. Adicionalmente, en este bucle *while*, se busca que el nombre del fichero a buscar coincida con la entrada leida, en cuyo caso se muestra por consola el mensaje correspondiente y se cambia el valor a la variable binaria, indicando que el fichero ha sido encontrado.

Finalmente, tras la ejecución de este bucle, se comprueba si el fichero ha sido encontrado o no. En caso negativo, se muestra el mensaje correspondiente por consola. A continuación, se cierra el directorio, comprobando que sea posible, en caso de que no lo sea, se muestra el error por pantalla y se retorna -1. Y si toda la ejecución del programa se ha realizado al completo obteniendo el éxito, se retorna el valor 0.

**Batería de pruebas**

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `./myishere f1.txt` | Error y retorna -1 | Error y retorna -1 | Se pasa un argumento. |
| `./myishere dirA dirB f1.txt` | Error y retorna -1 | Error y retorna -1 | Se pasan tres argumentos. |
| `./myishere no-existe f1.txt` | Error y retorna -1 | Error y retorna -1 | Se pasa un directorio que no existe |
| `./myls p1_tests f1.txt` | File f1.txt is in directory p1_tests | File f1.txt is in directory p1_tests | Se pasa los argumentos correctos. |
| `./myls p1_tests no-existe.txt` | File no-existe.txt is not in directory p1_tests | File no-existe.txt is not in directory p1_tests | Se pasa un fichero que no existe. |


