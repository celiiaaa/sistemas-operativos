# Sistemas Operativos

*Autora: Celia Patricio Ferrer*

*Curso: 2023-2024*

Este documento recoge una descripción de todo lo desarrollado en Sistemas Operativos.

# Indice

- [Práctica 1. Llamadas al Sistema Operativo](#práctica-1-llamadas-al-sistema-operativo)
    - [Mywc](#mywc)
    - [Myls](#myls)
    - [Myishere](#myishere)
- [Práctica 2. Minishell](#práctica-2-programación-de-un-intérprete-de-mandatos-minishell)
    - [Mandatos simples](#mandatos-simples)
    - [Secuencia de mandatos](#secuencia-de-mandatos)
    - [Mycalc](#mycalc)
    - [Myhistory](#myhistory)

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

---
---

## Práctica 2. Programación de un intérprete de mandatos (Minishell)

### Introducción

Este documento recoge una descripción detallada de cada implementación llevada a cabo en la *minishell* desarrollada. Se divide en cuatro principales bloques para ayudar a la interpretación y a su posible desarrollo en un futuro. Estas partes son: los mandatos internos `mycalc` y `myhistory`, los comandos simples y las secuencias de mandatos. Además, cada sección incluye un conjunto de pruebas realizadas para verificar el correcto funcionamiento de la *minishell*.

El programa principal implementa una *minishell*, una interfaz de línea de comandos simplificada para el sistema Unix. Se encarga de manejar la entrada de comandos del usuario, ejecutarlos y mostrar los resultados correspondientes. Además, proporciona funcionalidades básicas como la gestión de errores, la ejecución en segundo plano y el mantenimiento de un historial de comandos. También incluye un mecanismo para cerrar el programa y liberar el historial, mediante la señal de *Ctrl+c*.

Cabe mencionar que se han implementado los procedimientos necesarios para el manejo de errores, lo que garantiza la robustez y fiabilidad de este servicio. En caso de ocurrir algún problema o error, se proporciona la retroalimentación necesaria al usuario para informar sobre la situación. Esto se ha realizado desde la lectura del comando hasta en partes más detalladas, como en el formato incorrecto de los mandatos internos o en los cierres y aperturas de descriptores de ficheros.

Adicionalmente, se ha incluido un pequeño código para la liberación del historial. Para ello, se ha recorrido cada comando almacenado en el historial y se ha liberado, para liberar recursos. Y luego, se ha liberado el historial en sí. Esto se ha realizado bajo la señal de *Ctrl+c* junto a la finalización de la *minishell*.

### Mandatos simples

**Descripción del código**

Esta parte del programa corresponde con la ejecución llevada a cabo para los mandatos externos simples de la minishell desarrollada. Comienza tras comprobar que el contador de comandos es igual a exactamente la unidad, es decir, lo que se ha introducido en la *minishell* es un único comando. En primer lugar, se guarda en el historial dicho comando, de forma que, si excede el tamaño máximo del historial, libera el primer comando que fue almacenado.

A continuación, se ejecuta el *fork*. Se utiliza un *switch* para manejar posibles errores, que en cuyo caso se lanza el error correspondiente, y asignar tareas al hijo y al proceso padre. Si se trata del proceso hijo (el *pid* devuelto es 0), si hay redirecciones de cualquier tipo de los tres que hay, se realizan de la siguiente forma. Se cierra la entrada estándar, la salida estándar o la salida estándar de error, según corresponde. Y se abre con el modo y los permisos necesarios dicho fichero introducido. Finalmente, se lleva a cabo el execvp y se ejecuta el comando introducido.

En caso de que sea el proceso padre, se comprueba si se ha solicitado la ejecución en *background*. En caso negativo, se imprime el *pid* del proceso. Y en caso afirmativo, se espera con el bucle mientras que haya un hijo que no haya terminado con la función *wait*. En caso de que se produzca un error en dicha espera, se imprime el error correspondiente y se termina la ejecución del programa.

**Batería de pruebas**

En esta sección se recogen las pruebas llevadas a cabo para realizar la comprobación de dicho programa. Para obtener resultados y conclusiones, estas pruebas se han contrastado con las salidas obtenidas de ejecutar los mismos comandos ejecutados en la terminal de Linux Ubuntu.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `ls` | autores.txt libparser.so Makefile msh msh.c msh.o probador_ssoo_p2.sh | autores.txt libparser.so Makefile msh msh.c msh.o probador_ssoo_p2.sh | Ejecución de un comando simple sin argumentos. |
| `mkdir directorio` | Se crea el directorio con dicho nombre. | Se crea el directorio con dicho nombre. | Ejecución de un comando simple con un argumento. |
| `wc autores.txt &` | [92742] <br> 0 4 33 autores.txt | [92742] <br> 0 4 33 autores.txt | Ejecución de un comando simple en *background*. |
| `wc autores.txt > salida.txt &` | [927904] <br> Se crea el fichero salida.txt con: <br> 0 4 33 autores.txt | [927904] <br> Se crea el fichero salida.txt con: <br> 0 4 33 autores.txt | Ejecución de un comando simple en *background* y con redirección de la salida estándar. |
| `grep a < autores.txt` | 100471948, Patricio Ferrer, Celia | 100471948, Patricio Ferrer, Celia | Ejecución de un comando simple con redirección de la entrada estándar. |

### Secuencia de mandatos

**Descripción del código**

Esta segunda parte del programa corresponde con la ejecución llevada a cabo para la secuencia de mandatos externos compuestos de la *minishell* desarrollada. La secuencia de mandatos se conecta mediante pipes. Al igual que el anterior, lo primero que se comprueba es que sea una secuencia de mandatos. Para ello, no debe de tratarse ni del mandato interno *mycalc* ni del *myhist*, y el contador de comandos, debe ser mayor a uno.

Al igual que si se trata de un mandato simple, se almacena dicha secuencia de mandatos en el historial y se libera el mandato correspondiente en caso de que se haya llegado al máximo de espacio. Para ello, se hacen uso de las funciones *store_command* y *free_command*, además de actualizar los índices de la cabeza y la cola de la lista del historial.

Una vez guardada, se realiza la duplicación del fichero de entrada con la detección de posibles errores correspondiente, se guarda en una variable bajo el nombre *fdin*. Si no hubo ningún error, se realiza el bucle en el que se itera cada comando apara recorrerse todos ellos. En primer lugar, se crea el *pipe* si no se trata del último comando. Y, en segundo lugar, se lleva a cabo el *fork* con el *switch* para tratar el *pid* y ver si se trata del proceso padre o del hijo. En caso de error, se imprime el mensaje de error correspondiente y se pone fin al programa.

En caso de tratarse del proceso hijo, en primer lugar, si es especificado, se redirecciona la salida de error. Para ello, se cierra la salida estándar de error, se abre el fichero especificado en modo solo escritura, o se crea, o se trunca y se duplica el fichero para redirigir la salida estándar de error hacia él.

Lo siguiente que se lleva a cabo si se trata del primer comando y se especifica la redirección de entrada estándar, se cierra la entrada estándar, se abre el fichero en modo solo lectura y duplica este fichero para redirigir la entrada estándar hacia él. En caso contrario, si no se trata del primer comando, su entrada será la salida del comando anterior. Para ello, se cierra la entrada estándar, se duplica el descriptor asociado a la entrada y guardado en la variable *fdin* y se cierra.

Como continuación, se comprueba si se trata del último comando y se comprueba si se solicita redirección de la salida estándar. Si es así, se cierra la salida estándar, se abre el fichero y se duplica el descriptor para redirigir la salida estándar hacia él. Si no se trata del último comando, cierra la salida estándar, duplica el descriptor de fichero asociado a la salida de la tubería, para redireccionar la salida estándar hacia él, y cierra ambos ficheros, la salida de la tubería y la entrada.

Finalmente, el proceso hijo, obtiene el comando completo actual y lo ejecuta con un *execvp*. Por otro lado, si se trata del proceso padre, cierra el descriptor de fichero almacenado en la variable *fdin*. Además, si no es el último comando, almacena en la variable *fdin* la entrada de la tubería para la siguiente iteración del bucle *for*. Y, por último, el último proceso padre, si la ejecución es en *backgroud*, imprime el *pid* del proceso, y si no, hace un *wait* recogido en un bucle *while* para esperar a los hijos.

**Batería de pruebas**

En esta sección se recogen las pruebas llevadas a cabo para realizar la comprobación de dicho programa. Para obtener resultados y conclusiones, estas pruebas se han contrastado con las salidas obtenidas de ejecutar los mismos comandos ejecutados en la terminal de Linux Ubuntu.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `ls \| grep txt` | autores.txt | autores.txt | Se introduce una secuencia de mandatos sin redirecciones. |
| `ls \| grep txt > salida` | Se crea un archivo llamado salida con: autores.txt | Se crea un archivo llamado salida con: autores.txt | Se introduce una secuencia de mandatos con redirecciones. |
| `ls \| grep txt > salida &` | [11746] <br> Se crea un archivo llamado salida con: autores.txt | [11746] <br> Se crea un archivo llamado salida con: autores.txt | Se introduce una secuencia de dos mandatos con redirección y en *background*. |
| `ls \| grep txt \| wc -l \| sort > conteo` | [12482] <br> Se crea un archivo llamado conteo con: 1 | [12482] <br> Se crea un archivo llamado conteo con: 1 | Se introduce una secuencia de cuatro mandatos con redirección y en *background*. |

### Mycalc

**Descripción del código**

Esta otra parte del programa corresponde con la ejecución llevada a cabo para el mandato interno `mycalc` de la *minishell* desarrollada. Inicia verificando la sintaxis y comprueba que cumple con el formato: 
```sh
mycalc <operando1> <add/mul/div> <operando2>
```
Se requiere que los operandos sean números enteros y se prohíben redirecciones, secuencias de comandos y ejecución en segundo plano. Si hay errores de sintaxis, se mostrará un mensaje correspondiente en la salida estándar.

Una vez que la sintaxis se comprueba, se comprueba qué operación es la introducida. En caso de ser suma (*add*), se obtiene el valor de la variable de entorno y se incrementa con la suma de los operandos introducidos. La salida de esta operación se escribe en la salida estándar de error, mostrando la operación, el resultado y el valor de la variable que acumula las sumas.

En caso de ser multiplicación (*mul*), se realiza la operación y la salida se escribe en la salida estándar de error, mostrando la operación y el resultado. Y en caso de ser división (*div*), se realiza una comprobación para asegurar que se puede hacer dicha operación. Si es así, se muestran los errores correspondientes. Si no, se realiza la división y el módulo, el resto, y se escribe en la salida estándar de error.

**Batería de pruebas**

En esta sección se recogen las pruebas llevadas a cabo para realizar la comprobación de dicho programa. Para obtener resultados y conclusiones, estas pruebas se han contrastado con los resultados obtenidos de una calculadora.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `mycalc` | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | No se introduce ningún argumento. |
| `mycalc 1 add 1 > salida.txt` | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | Se introduce redirección de la salida estándar. |
| `mycalc 4 div uno` | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | Se introduce un operando que no es un número. |
| `mycalc 2 3 mul` | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | Se introduce la operación con un formato incorrecto. |
| `mycalc 3 suma 2` | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | [ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2> | Se introduce una operación que no existe. |
| `mycalc 3 add 4` <br> `mycalc -4 add 2` <br> `mycalc -10 add 2` <br> `mycalc 5 add -2` | [OK] 3 + 4 = 7; Acc 7 <br> [OK] -4 + 2 = -2; Acc 5 <br> [OK] -10 + 2 = -8; Acc - 3 <br>[OK] 5 + -2 = 3; Acc 0 | [OK] 3 + 4 = 7; Acc 7 <br> [OK] -4 + 2 = -2; Acc 5 <br> [OK] -10 + 2 = -8; Acc - 3 <br>[OK] 5 + -2 = 3; Acc 0 | Verificación del correcto funcionamiento del acumulador. |
| `env` | Acc = 0 | Acc = 0 | Imprimir la variable de entorno y su valor. |
| `mycalc 2 mul 4` | [OK] 2 * 4 = 8 | [OK] 2 * 4 = 8 | Se introduce la operación de multiplicación. |
| `mycalc 4 div 2` | [OK] 4 / 2 = 8; Resto 0 | [OK] 4 / 2 = 8; Resto 0 | Se introduce la operación de división |

### Myhistory

**Descripción del código**

Esta otra parte del programa corresponde con la ejecución llevada a cabo para el mandato interno llamado *myhistory* de la *minishell* desarrollada. Al igual que el anterior mandato interno, lo primero es verificar el formato del comando: *“myhistory <N>”*, siendo N un número que puede o no estar. Se prohíben redirecciones, secuencias de comandos y ejecución en segundo plano. Si hay errores de sintaxis, se mostrará un mensaje correspondiente en la salida estándar.

Lo siguiente que se comprueba es si N es dada o no. Por un lado, si no es dado, se imprime el historial al completo, un mandato en cada línea indexado del 0 al 19. Y, si es dado, se verifica que exista ese comando en el historial y que no esté fuera de los límites. Si se encuentra, se indica que el siguiente comando a ejecutar proviene del mandato interno *myhistory*. En caso de que no se encuentre, se escribe en la salida estándar que ese comando no ha sido encontrado.

En la siguiente iteración del bucle *while(1)*, comienza comprobando si la siguiente instrucción viene de este mandato interno. En este caso, se vuelve a indicar que el siguiente mandato no proviene de este mandato interno, se escribe en la salida estándar de error que se está ejecutando el mandato N del historial y se asigna el comando y los argumentos necesarios para ejecutar dicho mandato. La ejecución de a continuación se lleva a cabo como si fuera un comando ingresado de manera usual.

**Batería de pruebas**

En esta sección se recogen las pruebas llevadas a cabo para realizar la comprobación de dicho programa.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `myhistory` | 0 ls <br> 1 mycalc 1 add 2 <br> 2 wc < autores.txt <br> 3 ls \| grep c > out.txt & | 0 ls <br> 1 mycalc 1 add 2 <br> 2 wc < autores.txt <br> 3 ls \| grep c > out.txt & | Sin argumentos para que imprima el historial. |
| `myhistory 0` | Ejecutando el commando 1 autores.txt libparser.so Makefile msh msh.c msh.o probador_ssoo_p2.sh | Ejecutando el commando 1 autores.txt libparser.so Makefile msh msh.c msh.o probador_ssoo_p2.sh | Se ejecuta un comando almacenado en el historial. |
| `myhistory 20` | ERROR: Comando no encontrado | ERROR: Comando no encontrado | Se pasa un N que no existe |
| `myhistory 3` | Ejecutando el comando 3 <br> [24767] <br> Se crea un fichero llamado out.txt con: msh.c | Ejecutando el comando 3 <br> [24767] <br> Se crea un fichero llamado out.txt con: msh.c | Se pasa un N que tiene una secuencia de comandos y redirección de salida. |

---
---

## Práctica 3. Programación Multi–hilo

En esta última práctica se trata de un sistema multi-hilo implementa una cola circular para gestionar operaciones de una tienda mediante la comunicación entre productores y consumidores, logrando concurrencia y sincronización eficientes.

### Cola circular

Se trata del mecanismo de comunicación entre productores y consumidores de este sistema. Ha sido diseñada para garantizar la concurrencia y la sincronización. El fichero *queue.c* proporciona todas las funcionalidades necesarias para facilitar esta comunicación entre los dos roles de este sistema.

#### Element

La cola circular está compuesta por estas estructuras, las cuales se extraen o se insertan en ella. Cada una de estas estructuras contiene información relevante, la operación que se solicita llevar a cabo. Incluyen el identificador del producto (*producto_id*), el tipo de operación (*op*) y el número de unidades (*units*). Además, cada una cuenta con un puntero next que señala al siguiente elemento, y otro llamado *prev* que apunta al elemento anterior. Mientras los productores la agregan a la cola, los consumidores la sacarán para realizar lo que indique dicha operación.

#### Queue

Por otro lado, la cola circular es una estructura que está compuesta por las estructuras mencionadas anteriormente. Esta contiene varios campos importantes para el funcionamiento de la cola circular. Un puntero al primer elemento de la cola (*head*), otro al último elemento de la cola (*tail*), el tamaño máximo de la cola (*size*) y el número actual de elementos que están almacenados en la cola (*count*). Los punteros son útiles para la inserción y la extracción de los elementos. Mientras que el número de elementos y el tamaño de la cola controlan la capacidad de la cola y garantizan la concurrencia y sincronización adecuadas.

#### Concurrencia y sincronización

Los *mutex* aseguran la exclusión mutua, permitiendo que tanto los productores como los consumidores accedan a la cola circular sin enfrentar condiciones de carrera. Mientras que las dos variables condicionales permiten que los hilos esperen a que se cumplan ciertas condiciones para continuar la ejecución. En el caso de *cond_empty* se utiliza para que los hilos consumidores esperen si la cola se encuentra vacío. Y en el caso de *cond_full*, es utilizado, al contrario, para que los hilos productores esperen si la cola se encuentra llena.

#### Funciones

En primer lugar, se encuentra la definición de la función *queue_init*. Recibe un entero con el tamaño máximo de la cola que se desea inicializar. Esta función se encarga de crear una nueva cola circular y reservar la memoria necesaria para almacenar los elementos. Inicializa los punteros a *NULL*, establece el tamaño máximo de la cola y pone el contador de elementos a 0. Si al reservar memoria se produce un error, se muestra por pantalla y se retorna un puntero nulo. En cambio, si no se produce ningún error, entonces se retorna el puntero a la cola circular creado.

Para insertar elementos al final de la cola circular se ha creado la función *queue_put*. Esta función recibe un puntero a una cola circular y otro puntero a un elemento.

Comprueba si la cola está llena y si está vacía en ese instante. Si está vacía, inserta el elemento como único elemento, siendo la cabeza y la cola e incrementa el contador de elementos. Si no, se hacen uso de dos punteros auxiliares para guardar la cabeza y la cola actuales. Posteriormente, se inserta el nuevo elemento como siguiente de la cola y anterior a la cabeza, actualizando el puntero a la cola y el contador de elementos. Y retorna un 0 para indicar que finalizó con éxito.

La función `queue_get` se encarga de extraer elementos de la cola circular y tienen un funcionamiento parecido a la anterior. Verifica si la cola está vacía. Cuando hay elementos en la cola, se procede a extraer el elemento. Si la cola tiene un solo elemento, se actualizan los punteros *head* y *tail* a *NULL* y el contador de elementos. En caso contrario, se actualizan los punteros para eliminar el elemento extraído de la cola, se ajusta el puntero de la cabeza de la cola con el siguiente al elemento que se extrae y se configuran el *prev* y el *next* de la nueva cabeza. Posteriormente, se reduce el contador de elementos. Finalmente, se devuelve el elemento extraído.

Las siguiente dos funciones contienen una lógica muy parecida pero inversa. Por un lado, `queue_empty` devuelve si la cola dada está vacía (1) o no (0), comprobando que el contador de elementos sea o no igual a 0. Y, por otro lado, la función `queue_full` devuelve si la cola está llena (1) o no (0), comprobando si el contador de elementos es igual o no al tamaño máximo de la cola circular.

En caso de querer liberar los recursos utilizados por la cola circular y eliminarla, se ha implementado la función `queue_destroy`. Que se recorre todos los elementos que la cola circular dada almacena y los libera. Además de liberar el mutex y las variables condicionales utilizadas para la concurrencia y sincronización de este mecanismo. Finalmente, retorna el 0 como éxito y fin en la ejecución de esta función.

Esta última función, `print_queue`, se ha implementado de forma complementaria para visualizar la cola y ser consciente del funcionamiento de la implementación de este programa. Esta función muestra por pantalla todos los elementos almacenados en la cola circular pasada como parámetro, encapsulada en unos pares de corchetes. Se imprime el orden y muestra la operación almacenada de esta forma:
```sh
Elemento <id> : <producto_id> <op> <units>
```

#### Función principal

Esta parte corresponde a la implementación del hilo principal pedido. Este programa consiste en un sistema multi–hilo capaz de gestionar las operaciones de una tienda y calcular los beneficios y el *stock* en un entorno de concurrencia. En primer lugar, inicializa el beneficio total obtenido, el array que almacena el *stock* de cada producto, los *mutex* y las variables condicionales, que son utilizados para garantizar la concurrencia y la sincronización de este sistema.

Posteriormente, se realiza una comprobación inicial del número de argumentos. En caso de que sea incorrecto, se muestra por pantalla el mensaje correspondiente, que incluye el formato del comando a introducir: 
```sh
./store_manager <file> <n_prods> <n_cons> <b_size>
```
y se retorna el valor -1 para indicar que se produjo un error. Además, se comprueba que los valores introducidos sean válidos, que el fichero exista y que los números, tras la conversión a números, sean mayores a cero.

Lo siguiente que se lleva a cabo es la lectura del archivo pasado como parámetro. Se lee el número máximo de operaciones (siendo la primera línea) y se leen el resto de las líneas que contiene este archivo (las operaciones a realizar), almacenándolas en un array llamado *lista_operaciones* de forma ordenada.

Una vez que se ha terminado de leer hasta la última operación mandada dada por la primera línea, se cierra el fichero, realizando la comprobación correspondiente para detectar cualquier error. Posteriormente, se comprueba que el número de operaciones leídas no sea menor que el de las indicadas como máximo por el fichero. En caso de que sea así, se imprime el mensaje de error correspondiente y se finaliza la ejecución del programa con el valor de retorno -1.

Una vez que las operaciones del archivo han sido almacenadas en memoria, se procede a realizar la creación y el lanzamiento de los hilos productores y consumidores. Para ello, se realiza una declaración de un array de hilos de cada tipo de rol, con el tamaño especificado por comando. Cabe mencionar que la función para la creación de ambos hilos se ha utilizado *pthread_create*, y para los argumentos de cada uno, las estructuras que posteriormente se explicarán con mayor detalle.

Posteriormente, el programa procede distribuir equitativativamente las operaciones entre el número de hilos productores introducido por comando. Mientras se realiza esta distribución, se prepara los argumentos necesarios para crear los hilos productores. A continuación, se realiza el mismo procedimiento para la creación de los hilos consumidores. Y, como útlimo procedimiento con los hilos productor y consumidor, se ha relizado *pthread_join* para monitorear y esperar la finalización de todas las operaciones.

Una vez que todos los hilos han concluido su tarea, se procede a calcular el beneficio total obtenido y se muestra el stock actualizado de cada producto en la tienda. Finalmente, se liberan todos los recursos utilizados y se retorna el valor 0 para indicar la finalización del programa llevada con éxito.

#### Productor

El hilo productor se encarga de generar las operaciones, agregar las operaciones de forma ordenada para que posteriormente se lleven a cabo por un hilo consumidor. Los argumentos que este requiere son los siguientes: el buffer circular, el número máximo de operaciones que tiene que agregar en la cola, un array con los ids de las operaciones (las posiciones de todas las operaciones que ocupan en la lista de operaciones) y la lista de operaciones completa.

La función comienza con la obtención en variables locales de los argumentos pasados. Luego, mediante un bucle *for*, se lleva a cabo la inserción de las operaciones que tiene encargado el hilo. Se guarda la operación que se lleva a cabo en memoria de forma local y se inserta en la cola circular, con la función *queue_put*. Este proceso se realiza bajo el orden de lectura de las operaciones, para ello se sincroniza mediante un *mutex* y una variable condición que pone en espera a los hilos cuyas operaciones tengan un índice distinto al actual.

Además, se utiliza el *mutex* para que solo se realice esta función por un único hilo para evitar problemas de condiciones de carrera. Finalmente, esta función libera los recursos utilizados y finaliza la ejecución del hilo con *pthread_exit*.

#### Consumidor

Por otra parte, el hilo consumidor se encarga de extraer las operaciones de la cola, para llevar a cabo las operaciones que previamente han sido agregadas. Los argumentos que este requiere son los siguientes: el buffer circular, el número máximo de operaciones que tiene que agregar en la cola, un *array* con los *ids* de las operaciones (las posiciones de todas las operaciones que ocupan en la lista de operaciones), un puntero al beneficio total y otro al *stock* de los productos.

Esta función es muy parecida a la del productor. Comienza obteniendo los argumentos y los guarda en variables locales. Luego, realiza el bucle *for* para obtener las operaciones y realizar lo que sea necesario. Para obtener cada operación se utiliza la función *queue_get* pasando la cola circular como parámetro. Posteriormente, se realiza la operación en función del tipo de operación que sea, actualizando el beneficio (*profit*) y el *stock*.

Al igual que el anterior, el proceso se realiza bajo el orden de extracción de las operaciones, para ello se sincroniza mediante un *mutex* y una variable condición que pone en espera a los hilos cuyas operaciones tengan un índice distinto al actual. Además, se utiliza el *mutex* para que solo se realice esta función por un único hilo para evitar problemas de condiciones de carrera. Finalmente, esta función libera los recursos utilizados y finaliza la ejecución del hilo con *pthread_exit*.

#### Batería de pruebas

En esta sección se recogen todas las pruebas llevadas a cabo para realizar la comprobación de este programa.

En primer lugar, se han realizado este conjunto de pruebas básicas para comprobar la validación de los argumentos de entrada pasados por comando. Se ha querido llevar a cabo este conjunto de pruebas para ver la robustez de la detección de estos errores básicos.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `./store_manager` | Error. La operación es ./store_manager <file> <n_prods> <n_cons> <b_size> | Error. La operación es ./store_manager <file> <n_prods> <n_cons> <b_size> | Comando sin argumentos. Número de argumentos incorrecto. |
| `./store_manager no_existe.txt 1 1 1` | Error. No se pudo abrir el fichero. | Error. No se pudo abrir el fichero. | Fichero de entrada que no exista. |
| `./store_manager file.txt uno 1 1` | Error. El número de productores no es válido. | Error. El número de productores no es válido. | Introducir un número de productores que no sea válido. Es una cadena de caracteres. |
| `./store_manager file.txt 1 0 1` | Error. El número de consumidores no es válido. | Error. El número de consumidores no es válido. | Introducir un número de consumidores que no sea válido. Es menor/igual que cero. |
| `./store_manager file.txt 1 0 size` | Error. El tamaño del buffer no es válido. | Error. El tamaño del buffer no es válido. | Introducir un tamaño del
buffer que no sea válido. |

A continuación, se han realizado este otro conjunto de pruebas con diferentes ficheros. Este otro conjunto de pruebas se ha llevado a cabo para concluir y comprobar el correcto funcionamiento del código desarrollado.

| Input | Salida obtenida | Salida esperada | Descripción |
| ----------- | ----------- | ----------- | ----------- |
| `./store_manager vacio.txt 1 1 1` | Error. No se pudo leer el número de operaciones. | Error. No se pudo leer el número de operaciones. | Fichero vacío. |
| `./store_manager file1.txt 1 1 1` | Error. Número de operaciones leidas < máximo de operaciones indicado. | Error. Número de operaciones leidas < máximo de operaciones indicado. | Fichero con un número de operaciones máximo mayor que el número de operaciones escritas |
| `./store_manager file2.txt 1 1 1` | Total: 210 euros <br> Stock: <br> Product 1: 0 <br> Product 2: 10 <br> Product 3: 10 <br> Product 4: 0 <br> Product 5: 0 | Total: 210 euros <br> Stock: <br> Product 1: 0 <br> Product 2: 10 <br> Product 3: 10 <br> Product 4: 0 <br> Product 5: 0 | Fichero con un número de operación máximo menor que el número de operaciones escritas. Comprobar que se realizan solo las operaciones dadas por el número máximo indicado. |
| `./store_manager file3.txt 1 1 10` | Error. Operación no válida. | Error. Operación no válida. | Fichero con una operación que sea distinta a PURCHASE o a SALE. |
| `./store_manager file.txt 1 1 1` | Total: 120 euros <br> Stock: <br> Product 1: 20 <br> Product 2: 60 <br> Product 3: 20 <br> Product 4: 18 <br> Product 5: 2 | Total: 120 euros <br> Stock: <br> Product 1: 20 <br> Product 2: 60 <br> Product 3: 20 <br> Product 4: 18 <br> Product 5: 2 | Fichero proporcionado por el profesorado con 50 operaciones |
| `./store_manager file.txt 7 4 10` | Total: 120 euros <br> Stock: <br> Product 1: 20 <br> Product 2: 60 <br> Product 3: 20 <br> Product 4: 18 <br> Product 5: 2 | Total: 120 euros <br> Stock: <br> Product 1: 20 <br> Product 2: 60 <br> Product 3: 20 <br> Product 4: 18 <br> Product 5: 2 | Números de productores y consumidores mayor a uno con el fichero proporcionado por el profesorado de 50 operaciones. |

Los ficheros utilizados se exponen a continuación.

El fichero file1.txt es: 2<br>
2 PURCHASE 2


El fichero file2.txt es: 8<br>
1 PURCHASE 10<br>
2 PURCHASE 10<br>
3 PURCHASE 10<br>
4 PURCHASE 10<br>
5 PURCHASE 10<br>
4 SALE 10<br>
5 SALE 10<br>
1 SALE 10<br>
2 SALE 10<br>
3 SALE 10

El fichero file3.txt es: 2<br>
1 PURCHASE 10<br>
2 NO 10
