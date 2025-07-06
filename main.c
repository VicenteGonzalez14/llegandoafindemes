#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "tdas/map.h"
#include "tdas/extra.h"
#include "tdas/list.h"

int saldo = 0;

// Valida formato fecha YYYY-MM-DD
int esFechaValida(const char* fecha) {
    int anio, mes, dia;
    if (sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia) != 3)
        return 0;
    if (anio < 1900 || anio > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    return 1;
}

// Cargar dinero
void cargarDinero() {
    int monto;
    printf("\nIngrese monto a cargar: ");
    scanf("%d", &monto);
    if (monto <= 0) {
        printf("Monto inválido.\n");
        return;
    }
    saldo += monto;
    printf("Saldo actual: $%d\n", saldo);
}

// Agregar insumo
void agregarInsumo(Map* mapa) {
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
    printf("Insumo agregado correctamente.\n");
}

int main() {
    setlocale(LC_ALL, "");

    Map* mapa = map_create(compare_keys);
    categorias = map_create(is_equal_string);
    productos = map_create(is_equal_string);
    saldo = 0;

    cargarDatasetDesdeCSV(mapa, "insumos.csv");

    int opcion;
    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1. Cargar dinero\n");
        printf("2. Agregar insumo\n");
        printf("3. Mostrar boletín mensual\n");
        printf("4. Predecir gasto semanal\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                cargarDinero();
                break;
            case 2:
                agregarInsumo(mapa);
                break;
            case 3:
                mostrarBoletinMensual(mapa);
                break;
            case 4: {
                float prediccion = predecirGastoSemanalDesdeMapa(mapa);
                if (prediccion >= 0)
                    printf("Gasto estimado para la próxima semana: $%.2f\n", prediccion);
                else
                    printf("No hay suficientes datos para predecir.\n");
                break;
            }
            case 5:
                printf("Saliendo del programa...\n");
                break;
            default:
                printf("Opción inválida.\n");
        }

        if (opcion != 5) {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

    } while(opcion != 5);

    liberar_mapa(mapa);
    
    return 0;
}