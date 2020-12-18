#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <mutex>
#include <cstdlib>
#include <queue>
#include <memory>
#include <sstream>
#include <experimental/filesystem>

#include "utils.h"

using namespace std;
namespace fs = experimental::filesystem;

//Variables hilos e input
HiloData parametros[REASONABLE_THREAD_MAX];
EntradaMain opc;

//Variables para calcular primer y último archivo
mutex mutexNumProgreso;
int numProgreso;
int cantArchivosTotal;
char primerArchivo[MAX_LENGTH_PATH_FILE];
char ultimoArchivo[MAX_LENGTH_PATH_FILE];

//Variables para el mínimo y máximo
ResumenReporteArchivo minimoArchivo;
ResumenReporteArchivo maximoArchivo;

#define MODO_TEST false //true para simular espera de hilos y más info

//Definicion de Funciones
void mostrarInput();
void crearThreads();
void cargarTareas(char *);
void crearThreads();
void recursivoEncolarArchivos(queue<string> &, const char*);
void dividirTareas(queue<string> &);
void procesarArchivos(HiloData *);
void guardarResultadosThreads();
void mostrarResultados();

int main(int argc, char **argv)
{
	checkParametros(argc, argv, &opc); //separo los params en el struct opc
	mostrarInput();
	cargarTareas(opc.dirEntrada);
	crearThreads();
	guardarResultadosThreads();
	mostrarResultados();

	return EXIT_SUCCESS;
}

void mostrarInput()
{
	cout << DIVIDER << endl;
	cout << "Entrada: " << opc.dirEntrada << endl;
	cout << "Salida: " << opc.dirSalida << endl;
	cout << "Paralelismo: " << opc.nivel << endl;
}

void crearThreads()
{
	if (MODO_TEST)
		cout << DIVIDER << endl << "Orden de archivos analizados" << endl;

	queue<thread> colaHilos;
	for (int i = 0; i < opc.nivel; ++i)
		colaHilos.push(thread(procesarArchivos, &(parametros[i])));
	while (!colaHilos.empty())
	{
		colaHilos.front().join();
		colaHilos.pop();
	}
}

void recursivoEncolarArchivos(queue<string> &colaArchivos, const char* path)
{
	for (const auto & entry : fs::directory_iterator(path)){
		string p = entry.path();
		if (esDirectorio(p.c_str()))
			recursivoEncolarArchivos(colaArchivos,p.c_str());
		else
			colaArchivos.push(p);
	}
}

void cargarTareas(char *path)
{
	queue<string> colaArchivos;
	recursivoEncolarArchivos(colaArchivos,path);
	dividirTareas(colaArchivos);
}

void dividirTareas(queue<string> &colaArchivos)
{
	//Vemos la cantidad de filas que le corresponderán a cada hilo
	cantArchivosTotal = colaArchivos.size();
	if(cantArchivosTotal < opc.nivel)
	{
		cout << "Cantidad de archivos insuficientes (" << cantArchivosTotal << ")"<< endl;
		exit(EXIT_FAILURE);
	}
	int rows_for_thread = cantArchivosTotal / opc.nivel;
	int resto = cantArchivosTotal % opc.nivel;

	int i = 0;
	int *vp = (int *)malloc(opc.nivel * sizeof(int *));

	for (i = 0; i < opc.nivel; i++)
	{
		vp[i] = rows_for_thread;
		if (resto > 0)
		{
			vp[i]++;
			resto--;
		}
	}
	int from_row = 0;
	int to_row = vp[0] - 1;
	for (i = 0; i < opc.nivel; i++)
	{
		parametros[i].from_row = from_row;
		parametros[i].to_row = to_row;
		parametros[i].cant_archivos = vp[i];

		int ind = 0;
		for (int r = from_row; r <= to_row; r++)
		{
			strcpy(parametros[i].reportes[ind].nombre, colaArchivos.front().c_str());
			colaArchivos.pop();
			ind++;
		}
		from_row = from_row + vp[i];
		to_row = from_row + vp[i + 1] - 1;
	}
}

void procesarArchivos(HiloData *my_param)
{
	stringstream name_t;
	FILE *file;

	int vocal = 0;
	int consonante = 0;
	int others = 0;
	char ch;

	my_param->threadId = this_thread::get_id();
	strcpy(my_param->tiempoInicio, getTiempo(FORMAT_HORA).c_str());

	for (int i = 0; i < my_param->cant_archivos; i++)
	{
		mutexNumProgreso.lock();
		if (MODO_TEST)
		{
			this_thread::sleep_for(std::chrono::milliseconds(500));
			cout << numProgreso + 1 << ") " << my_param->reportes[i].nombre << endl;
		}
		if (++numProgreso == 1)
			strcpy(primerArchivo, my_param->reportes[i].nombre);
		else if (numProgreso == cantArchivosTotal)
			strcpy(ultimoArchivo, my_param->reportes[i].nombre);
		mutexNumProgreso.unlock();

		vocal = 0;
		consonante = 0;
		others = 0;

		/* Apertura del archivo a procesar */
		file = fopen((const char *)my_param->reportes[i].nombre, "r");
		if (file)
		{
			while ((ch = getc(file)) != EOF)
			{
				/* Condition for vowel */
				if (ch == 97 || ch == 101 || ch == 105 || ch == 111 || ch == 117 ||
					ch == 65 || ch == 69 || ch == 73 || ch == 79 || ch == 85)
					vocal++;
				else if ((ch >= 97 && ch <= 122) || (ch >= 65 && ch <= 90))
					consonante++;
				else if (ch == 10)
				{
					//
				}
				else
					others++;
			}
			fclose(file);
		}

		my_param->reportes[i].cantVocales = vocal;
		my_param->reportes[i].cantConsonantes = consonante;
		my_param->reportes[i].cantOtros = others;
	}
	if (MODO_TEST)
		this_thread::sleep_for(std::chrono::milliseconds(2000));
	strcpy(my_param->tiempoFinal, getTiempo(FORMAT_HORA).c_str());
}

void guardarResultadosThreads()
{
	string descripcion = "/Hilo_";
	string nombre_salida = opc.dirSalida + descripcion + getTiempo(FORMAT_FECHA_HORA) + ".log";
	ofstream Guardar(nombre_salida, ios::app);

	if (MODO_TEST)
		cout << endl << "Calculando máximo y mínimo" << endl;

	for (int i = 0; i < opc.nivel; i++)
	{
		Guardar << DIVIDER << "Theard " << i << " (" << parametros[i].threadId << ") " << DIVIDER << endl;
		Guardar << "Hora de inicio: " << parametros[i].tiempoInicio << endl;
		Guardar << "Hora de finalización: " << parametros[i].tiempoFinal << endl << endl;

		for (int j = 0; j < parametros[i].cant_archivos; j++)
		{
			ReporteArchivo reporte = parametros[i].reportes[j];
			int totalCaracteres = reporte.cantVocales + reporte.cantConsonantes + reporte.cantOtros;
			Guardar << "Archivo: " << reporte.nombre << endl;
			Guardar << "Cantidad vocales: " << reporte.cantVocales << endl;
			Guardar << "Cantidad consonantes: " << reporte.cantConsonantes << endl;
			Guardar << "Cantidad \"otros caracteres\": " << reporte.cantOtros << endl;
			Guardar << "Cantidad total: " << totalCaracteres << endl
					<< endl;

			if ((minimoArchivo.totalCaracteres == 0 && strlen(minimoArchivo.nombre) == 0) || minimoArchivo.totalCaracteres > totalCaracteres)
			{
				if (MODO_TEST)
					cout << "Min:" << totalCaracteres << " " << reporte.nombre << endl;
				strcpy(minimoArchivo.nombre, reporte.nombre);
				minimoArchivo.totalCaracteres = totalCaracteres;
			}
			if ((maximoArchivo.totalCaracteres == 0 && strlen(maximoArchivo.nombre) == 0) || maximoArchivo.totalCaracteres < totalCaracteres)
			{
				if (MODO_TEST)
					cout << "Max:" << totalCaracteres << " " << reporte.nombre << endl;
				strcpy(maximoArchivo.nombre, reporte.nombre);
				maximoArchivo.totalCaracteres = totalCaracteres;
			}
		}
	}
	Guardar.close();
}

void mostrarResultados()
{
	cout << DIVIDER << endl
		 << "Cantidad de archivos total: " << cantArchivosTotal << endl
		 << DIVIDER << endl;
	for (int i = 0; i < opc.nivel; i++)
	{
		cout << "Thread " << i << " (" << parametros[i].threadId << ")" << endl;
		cout << "Cantidad de archivos procesados: " << parametros[i].cant_archivos << endl;
		for (int j = 0; j < parametros[i].cant_archivos; j++)
			cout << parametros[i].reportes[j].nombre << endl;
		cout << endl;
	}

	cout << DIVIDER << endl;
	cout << "Primer archivo analizado:" <<endl<< primerArchivo << endl;
	cout << "Último Archivo analizado:" <<endl<< ultimoArchivo << endl;
	cout << DIVIDER << endl;
	cout << "Archivo de menor caracteres totales (" << minimoArchivo.totalCaracteres << "):" << endl;
	cout << minimoArchivo.nombre << endl;
	cout << DIVIDER << endl;
	cout << "Archivo de mayor caracteres totales (" << maximoArchivo.totalCaracteres << "):" << endl;
	cout << maximoArchivo.nombre <<endl<<DIVIDER<<endl;
}