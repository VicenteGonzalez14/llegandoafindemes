#ifndef EXTRA_H
#define EXTRA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define MAX_INSUMOS 10000

typedef struct {
    char fecha[11];
    char categoria[20];
    char producto[20];
    int cantidad;
    int valor_total;
} Insumo;

extern Insumo insumos[MAX_INSUMOS];
extern int totalInsumos;

        
void mostrarBoletinSemanal();
void mostrarBoletinMensual();
float predecirGastoSemanal();





char **leer_linea_csv(FILE *archivo, char separador);

List *split_string(const char *str, const char *delim);

// Función para limpiar la pantalla
void limpiarPantalla();

void presioneTeclaParaContinuar();

void cargarDatasetDesdeCSV(const char *nombreArchivo);


#endif