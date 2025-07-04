#ifndef EXTRA_H
#define EXTRA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 
#include "list.h"
#include "map.h"

// Constantes
#define MAX_LINEA_CSV 1024  // Para lectura de CSV
#define MAX_CATEGORIA 20
#define MAX_PRODUCTO  20
#define MAX_FECHA     11

typedef struct {
    char fecha[MAX_FECHA];
    char categoria[MAX_CATEGORIA];
    char producto[MAX_PRODUCTO];
    int cantidad;
    int valor_total;
} Insumo;

// Funciones del Mapa 
int compare_keys(void* key1, void* key2);  // Para strcmp
void insertar_insumo(Map* map, Insumo* insumo);
List* obtener_insumos_por_categoria(Map* map, const char* categoria);
void liberar_mapa(Map* map);

// CSV y Datos
char** leer_linea_csv(FILE *archivo, char separador);
void cargarDatasetDesdeCSV(Map* mapa, const char* nombreArchivo);
List* split_string(const char *str, const char *delim);

// Visualización
void mostrar_insumo(const Insumo* insumo);  // Nueva función útil
void mostrar_insumos_por_categoria(Map* map, const char* categoria);
void mostrarBoletinSemanal(Map* map);       // Ahora recibe Map*
void mostrarBoletinMensual(Map* map);       // Ahora recibe Map*

// Utilidades
void limpiarPantalla();
void presioneTeclaParaContinuar();
float predecirGastoSemanal(Map* map);       // Actualizada

// Comparadores 
int string_lower_than(void *a, void *b);
int insumo_categoria_lower_than(void* a, void* b);

#endif