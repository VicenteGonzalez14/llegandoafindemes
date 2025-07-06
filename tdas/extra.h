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
#define MAX_LINEA_CSV 1024  // Longitud máxima para una línea CSV
#define MAX_CATEGORIA 50
#define MAX_PRODUCTO  128 
#define MAX_FECHA     11    // Formato: YYYY-MM-DD + '\0'

extern Map* categorias;
extern Map* productos;



typedef struct {
    char fecha[MAX_FECHA];
    char categoria[MAX_CATEGORIA];
    char producto[MAX_PRODUCTO];
    int cantidad;
    int valor_total;
} Insumo;

// Estructura temporal para ordenamiento
typedef struct {
    Insumo* insumo;
    time_t timestamp;
} InsumoConTimestamp;

// --------------------- MAPAS ---------------------
int compare_keys(void* key1, void* key2);  // strcmp
int string_lower_than(void *a, void *b);   // strcmp ordenado
int is_equal_string(void *a, void *b);     // strcmp igualdad
int compare_keys(void* a, void* b);


void insertar_insumo(Map* map, Insumo* insumo);
List* obtener_insumos_por_categoria(Map* map, const char* categoria);
void liberar_mapa(Map* map);

// --------------------- CSV ---------------------
char** leer_linea_csv(FILE *archivo, char separador);
void cargarDatasetDesdeCSV(Map* mapa, const char* nombreArchivo);
void guardarMapaEnCSV(Map* mapa, const char* nombreArchivo);

// --------------------- VISUALIZACIÓN ---------------------
void mostrar_insumo(const Insumo* insumo);  // Recomendado: para uso común
void mostrar_insumos_por_categoria(Map* map, const char* categoria);
void mostrarBoletinSemanal(Map* map);
void mostrarBoletinMensual(Map* map);

// --------------------- UTILIDADES ---------------------
void limpiarPantalla();
void presioneTeclaParaContinuar();
void buscarInsumosEnRangoDeFechas(Map* map, const char* fecha_inicio, const char* fecha_fin, int orden);

// --------------------- PREDICCIÓN ---------------------
float predecirGastoSemanalDesdeMapa(Map* map);

// --------------------- COMPARADORES ---------------------
int insumo_categoria_lower_than(void* a, void* b);
int compare_insumos_by_fecha(void* a, void* b);
int compare_insumos_by_producto(void* a, void* b);
int compare_insumos_by_valor(void* a, void* b);

// --------------------- LIBERACIÓN ---------------------
void liberar_insumo(void* data);  // Para list_clean

#endif
