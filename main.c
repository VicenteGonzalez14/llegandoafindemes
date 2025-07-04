#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "extra.h"
#include "list.h"
#include "map.h"

int saldo = 0;

// Función para cargar dinero
void cargarDinero() {
    int monto;
    printf("\nIngrese el monto a cargar: ");
    scanf("%d", &monto);

    if (monto <= 0) {
        printf("El monto debe ser mayor que cero.\n");
        return;
    }

    saldo += monto;
    printf("Dinero cargado exitosamente. Saldo actual: %d\n", saldo);
}

// Función para validar fecha en formato YYYY-MM-DD
int esFechaValida(const char* fecha) {
    int anio, mes, dia;
    if (sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia) != 3)
        return 0;
    if (anio < 1900 || anio > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    return 1;
}

// Función para agregar insumo (versión simplificada)
void agregarInsumo(HashMap* mapa) {
    Insumo* nuevo = malloc(sizeof(Insumo));
    
    printf("\nIngrese fecha (YYYY-MM-DD): ");
    scanf("%10s", nuevo->fecha);
    if (!esFechaValida(nuevo->fecha)) {
        printf("Fecha inválida.\n");
        free(nuevo);
        return;
    }

    printf("Ingrese categoría: ");
    scanf("%19s", nuevo->categoria);
    
    printf("Ingrese producto: ");
    scanf("%19s", nuevo->producto);
    
    printf("Ingrese cantidad: ");
    scanf("%d", &nuevo->cantidad);
    
    printf("Ingrese valor total: ");
    scanf("%d", &nuevo->valor_total);

    insertar_insumo(mapa, nuevo);
    printf("Insumo agregado exitosamente.\n");
}

int main() {
    setlocale(LC_ALL, "");

    // Crear el mapa principal
    Map* mapa = map_create(compare_keys);
    saldo = 0;

    // Cargar datos iniciales
    cargarDatasetDesdeCSV(mapa, "insumos.csv");

    int opcion;
    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1. Cargar dinero a la billetera\n");
        printf("2. Agregar insumo\n");
        printf("3. Mostrar boletines semanales\n");
        printf("4. Mostrar boletin mensual\n");
        printf("5. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                cargarDinero();
                break;
            case 2:
                agregarInsumo(mapa);
                break;
            case 3:
                mostrarBoletinSemanal(mapa);
                break;
            case 4:
                mostrarBoletinMensual(mapa);
                break;
            case 5:
                // Guardar datos antes de salir
                // (Implementar según necesidad)
                printf("Saliendo del programa. ¡Hasta luego!\n");
                break;
            default:
                printf("Opción inválida. Intente nuevamente.\n");
        }

        presioneTeclaParaContinuar();
        limpiarPantalla();

    } while(opcion != 5);

    // Liberar memoria
    liberar_mapa(mapa);
    return 0;
}