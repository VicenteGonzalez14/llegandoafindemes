#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tdas/extra.h"


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

// Función para mostrar boletín semanal
void mostrarBoletinSemanal() {
    printf("\n[Función mostrarBoletinSemanal aún no implementada]\n");
}

// Función para mostrar boletín mensual
void mostrarBoletinMensual() {
    printf("\n[Función mostrarBoletinMensual aún no implementada]\n");
}
