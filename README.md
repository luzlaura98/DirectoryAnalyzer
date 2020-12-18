# DirectoryAnalyzer
A script for Linux. Analyze the quantity of characters of a directory using threads.

## About the project
This project was done for **Sistemas Operativos**, an Universidad Nacional de La Matanza's subject.

### Consigna
Se deberá generar un programa que reciba por parámetros 3 valores: un path a un directorio de “entrada”, un
path a un directorio de “salida” y el nivel de paralelismo.
Una vez iniciado el programa se deberá repartir todos los archivos del directorio de entrada en forma equitativa
entre los N threads que se han generado. Cada thread deberá contar la cantidad de letras totales en el
contenido de cada archivo, generando un nuevo archivo en el directorio de salida con el mismo nombre original
y cuyo contenido sea:
Hora de inicio
Número de Thread
Cantidad de vocales
Cantidad de consonantes
Cantidad de “otros caracteres”.
Hora de finalización
Adicionalmente se deberá mostrar por pantalla:
* Cada thread, su número y qué archivos le tocó analizar.
* El nombre del archivo con menor cantidad de caracteres totales.
* El nombre archivo con mayor cantidad de caracteres totales.
* El nombre del primer archivo finalizado.
* El nombre del último archivo finalizado.

**Criterios de corrección (Obligatorio)**
* Debe cumplir con el enunciado 
* Se entrega el makefile correspondiente 
* El ejecutable cuenta con una ayuda (-h, --help) 
* Validación correcta de los parámetros 
* Proveer archivos de ejemplo para realizar pruebas 
* Utiliza la biblioteca de C++ para crear threads 
* Acepta correctamente paths absolutos y relativos 
* No se permiten soluciones que utilicen los syscalls system() o popen() 

## Getting Started
1. Compile
```
make ejercicio2
```
2. Execute
```
./ejercicio2 -entrada [DIRECTORY] -salida [OUTPUT DIRECTORY] -nivel [PARALLELISM LEVEL]
```

## Help
For more information and examples execute:
```
./ejercicio2 --help
```

## Acknowledgements
Project is part of multiple exercises in group. My classmate, Muguruza, contributed there.
