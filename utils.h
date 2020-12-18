#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <ctime>
#include <thread>
#include <dirent.h>
#include <errno.h>

#define AYUDA "------------------Ayuda------------------\n\
#.SYNOPSIS\n\
#   Se deberá generar un programa que reciba por parámetros \n\
#   3 valores: un path a un directorio de “entrada”, un path a \n\
#   un directorio de “salida” y el nivel de paralelismo.\n\
#   Una vez iniciado el programa se deberá repartir todos \n\
#   los archivos del directorio de entrada en forma equitativa \n\
#   entre los N threads que se han generado. Cada thread deberá \n\
#   contar la cantidad de letras totales en el contenido de cada \n\
#   archivo\n\
#\n\
#.DESCRIPTION\n\
#   Cada proceso debe imprimir su nivel de descendencia, \n\
#   su Pid y el Pid de todos los procesos que lo precedieron.\n\
#    Adicionalmente, se puede recibir un parámetro N que \n\
#    indique cual es el último nivel de descendencia a mostrar.\n\
#    Por ej: Si N=2 crearía únicamente los procesos 1,2,3 y 4. \n\
#    Si el parámetro N no es enviado, genera el árbol completo.\n\
#    Poner una espera, por tiempo o hasta apretar una tecla, \n\
#    antes de finalizar para permitir validar la correcta ejecución.\n\
#\n\
#.EXAMPLE\n\
#   ./ejercicio2.exe -entrada \"../Mis Pruebas\" -salida \"/home/me/Documentos/Res\" -nivel 4\n\
#\n\
#.INPUT\n\
# Parámetros (el orden de los parámetros no debe ser fijo): \n\
# • un path a un directorio de “entrada”\n\
# • un path a un directorio de “salida” \n\
# • el nivel de paralelismo.\n\
#.Output\n\
#   Nuevo archivo en el directorio de salida \n\
#   con el mismo nombre original y cuyo contenido sea:\n\
#   Hora de inicio\n\
#   Número de Thread\n\
#   Cantidad de vocales\n\
#   Cantidad de consonantes\n\
#   Cantidad de “otros caracteres”.\n\
#   Hora de finalización\n\
#   \n\
#   Adicionalmente se deberá mostrar por pantalla:\n\
#   • Cada thread, su número y qué archivos le tocó analizar\n\
#   • El nombre del archivo con menor cantidad de caracteres totales.\n\
#   • El nombre archivo con mayor cantidad de caracteres totales.\n\
#   • El nombre del primer archivo finalizado.\n\
#   • El nombre del último archivo finalizado"

//Constantes
#define CANT_PARAMS 7 // ini + cant opciones * 2
#define REASONABLE_THREAD_MIN 1
#define REASONABLE_THREAD_MAX 10
#define MAX_LENGTH_PATH_FILE FILENAME_MAX//100
#define MAX_TOTAL_ARCHIVOS 100
#define MAX_ARCHIVOS_POR_THREAD 60
#define SIZE_HORA 9
#define FORMAT_HORA "%H:%M:%S"
#define FORMAT_FECHA_HORA "%d-%m-%Y %H:%M:%S"
#define DIVIDER "-------------------------------------------------------"

using namespace std;

//Estructuras
struct ReporteArchivo
{
	char nombre[MAX_LENGTH_PATH_FILE];
	int cantVocales;
	int cantConsonantes;
	int cantOtros;
};

struct HiloData
{
	struct ReporteArchivo reportes[MAX_TOTAL_ARCHIVOS];
	thread::id threadId;
	char tiempoInicio[SIZE_HORA];
	char tiempoFinal[SIZE_HORA];
	int from_row;
	int to_row;
	int cant_archivos;
};

struct ResumenReporteArchivo
{
	char nombre[MAX_LENGTH_PATH_FILE];
	int totalCaracteres;
};

struct EntradaMain
{
	char dirEntrada[MAX_LENGTH_PATH_FILE];
	char dirSalida[MAX_LENGTH_PATH_FILE];
	int nivel;
};

void checkParametros(int cantParams, char **pParams, EntradaMain *pEntrada);
bool existeDirectorio(const char *directorio);
int esNumeroValido(char *input);
bool cargarPathAbsoluto(char *resultPath, const char *path);
int esDirectorio(const char* path);

void checkParametros(int cantParams, char **pParams, EntradaMain *pEntrada)
{
	if (cantParams == 2 && (strcmp(pParams[1], "--help") == 0 || strcmp(pParams[1], "-h") == 0))
	{
		cout << AYUDA << endl;
		cout << endl
			 << "Debe ejecutar el programa ./ejercicio2" << endl;
		cout << "Parametros path entrada, path salida, nivel paralelismo" << endl;
		exit(EXIT_SUCCESS);
	}
	if (cantParams != CANT_PARAMS)
	{
		cout << "Cantidad de parámetros invalido." << endl;
		exit(EXIT_FAILURE);
	}
	for (int i = 2; i <= CANT_PARAMS; i += 2)
	{
		if (strcmp(pParams[i - 1], "-entrada") == 0)
		{
			if (!cargarPathAbsoluto(pEntrada->dirEntrada, pParams[i]))
			{
				cout << "Directorio de entrada no válido." << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(pParams[i - 1], "-salida") == 0)
		{
			if (!cargarPathAbsoluto(pEntrada->dirSalida, pParams[i]))
			{
				cout << "Directorio de salida no válido." << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(pParams[i - 1], "-nivel") == 0)
		{
			int nivel = esNumeroValido(pParams[i]);
			if (nivel != -1)
				pEntrada->nivel = nivel;
			else
				exit(EXIT_FAILURE);
		}
		else
		{
			cout << "Opción inválida." << endl;
			exit(EXIT_FAILURE);
		}
	}
	if (pEntrada->dirEntrada == NULL || pEntrada->dirSalida == NULL || pEntrada->nivel == 0)
	{
		cout << "Opciones inválidas." << endl;
		exit(EXIT_FAILURE);
	}
}

/* Agrega en resultPath el path absoluto.
Retorna si el path es valido.*/
bool cargarPathAbsoluto(char *resultPath, const char *path)
{
	if (existeDirectorio(path))
	{
		strcpy(resultPath, realpath(path, NULL)); //sea relativo o no trae el path absoluto
		return resultPath != NULL;
	}
	return false;
}

bool existeDirectorio(const char *directorio)
{
	DIR *dir = opendir(directorio);
	if (dir)
	{
		closedir(dir); //existe
		return true;
	}
	return false;
}

int esNumeroValido(char *input)
{
	char *endp;
	int num = strtol(input, &endp, 10);
	if (*endp != '\0')
	{
		cout << "No es un número válido.";
		num = -1;
	}
	else if (num < REASONABLE_THREAD_MIN || num > REASONABLE_THREAD_MAX)
	{
		cout << "Argumento inválido para la cuenta de Hilos" << endl;
		cout << "El mínimo número de paralelimos es de: " << REASONABLE_THREAD_MIN << endl;
		cout << "El máximo número de paralelimos es de: " << REASONABLE_THREAD_MAX << endl;
		num = -1;
	}
	return num;
}

string getTiempo(const char *format)
{
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, format);
	return oss.str();
}

int esDirectorio(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

#endif