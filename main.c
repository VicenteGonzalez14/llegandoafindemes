#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/list.h"

#define HASH_SIZE 1000  // Tamaño de la tabla hash

// Estructura de los insumos
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
    Nodo* tabla_fecha[HASH_SIZE];
    Nodo* tabla_categoria[HASH_SIZE];
    Nodo* tabla_producto[HASH_SIZE];
    Nodo* tabla_cantidad[HASH_SIZE];
    Nodo* tabla_valor_total[HASH_SIZE];
} HashMap;

// Función de hash simple
unsigned int hash(const char* clave) {
    unsigned int hash_value = 0;
    while (*clave) {
        hash_value = (hash_value * 31) + *clave++;
    }
    return hash_value % HASH_SIZE;
}




void cargarDinero();               // 1
void agregarInsumo();              // 2
void mostrarBoletinSemanal();     // 3
void mostrarBoletinMensual();     // 4

int main() {
    int opcion;

    do {
        printf("\n--- MENÚ PRINCIPAL ---\n");
        printf("1. Cargar dinero a la billetera\n");
        printf("2. Agregar insumo\n");
        printf("3. Mostrar boletín semanal\n");
        printf("4. Mostrar boletín mensual\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                cargarDinero();
                break;
            case 2:
                agregarInsumo();
                break;
            case 3:
                mostrarBoletinSemanal();
                break;
            case 4:
                mostrarBoletinMensual();
                break;
            case 5:
                printf("Saliendo del programa. ¡Hasta luego!\n");
                break;
            default:
                printf("Opción inválida. Intente nuevamente.\n");
        }

    } while(opcion != 5);

    return 0;
}

// Función para cargar dinero
void cargarDinero() {
    printf("\n[Función cargarDinero aún no implementada]\n");
    
}

// Función para agregar insumo
void agregarInsumo() {
    printf("\n[Función agregarInsumo aún no implementada]\n");
}
