#ifndef EXTRA_H
#define EXTRA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

typedef struct {
    char categoria[50];
    int cantidad;
    int valorTotal;
    char fecha[11]; // Formato YYYY-MM-DD
} Insumo;

extern Insumo insumos[];   // arreglo global con los insumos
extern int totalInsumos;   // cantidad de insumos guardados
        
void mostrarBoletinSemanal();
void mostrarBoletinMensual();



char **leer_linea_csv(FILE *archivo, char separador);

List *split_string(const char *str, const char *delim);

// Función para limpiar la pantalla
void limpiarPantalla();

void presioneTeclaParaContinuar();

void cargarDatasetDesdeCSV(const char *nombreArchivo);


#endif