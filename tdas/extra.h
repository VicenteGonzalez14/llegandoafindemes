#ifndef EXTRA_H
#define EXTRA_H
#define HASH_SIZE 1000  // Tamaño de la tabla hash
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 
#include "list.h"

#define MAX_INSUMOS 10000

typedef struct {
    char fecha[11];
    char categoria[20];
    char producto[20];
    int cantidad;
    int valor_total;
} Insumo;

typedef struct Nodo {
    Insumo insumo;
    struct Nodo* siguiente;
} Nodo;

// Mapa hash para los insumos (una tabla de listas enlazadas)
typedef struct {
    Nodo **tabla_fecha;
    Nodo **tabla_categoria;
    Nodo **tabla_producto;
    Nodo **tabla_cantidad;
    Nodo **tabla_valor_total;
    int capacidad;
    int elementos;
} HashMap;

extern Insumo insumos[MAX_INSUMOS];
extern int totalInsumos;
extern HashMap hashMap; 



int string_lower_than(void *a, void *b);

void mostrarBoletinSemanal();
void mostrarBoletinMensual();
char **leer_linea_csv(FILE *archivo, char separador);
List *split_string(const char *str, const char *delim);
void limpiarPantalla();
void presioneTeclaParaContinuar();
void cargarDatasetDesdeCSV(const char *nombreArchivo);
void guardarInsumoEnCSV(const Insumo *insumo, const char *nombreArchivo);
void guardarTodosLosInsumosEnCSV(const char *nombreArchivo);
void insertarEnTabla(Nodo* tabla[], unsigned int (*func_hash)(const void*), const void* clave, Insumo insumo);
unsigned int hashFechaPtr(const void* ptr);
unsigned int hashStrPtr(const void* ptr);
unsigned int hashCantidadPtr(const void* ptr);
unsigned int hashValorTotalPtr(const void* ptr);
void redimensionarHashMap(HashMap *mapa, int nueva_capacidad);
void rehashTablaStr(Nodo** tablaVieja, int capacidadVieja, Nodo** tablaNueva, int nueva_capacidad, unsigned int (*func_hash)(const char*), const char* (*obtenerClave)(const Insumo*));
void rehashTablaInt(Nodo** tablaVieja, int capacidadVieja, Nodo** tablaNueva, int nueva_capacidad, unsigned int (*func_hash)(int), int (*obtenerClave)(const Insumo*));
unsigned int hashStr(const char* clave);
unsigned int hashFecha(const char* fecha);
#endif