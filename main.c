#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/list.h"

#define HASH_SIZE 1000  // Tamaño de la tabla hash



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

int saldo = 0;
void cargarDinero();               // 1
void agregarInsumo();              // 2
void mostrarBoletinSemanal();     // 3
void mostrarBoletinMensual();     // 4

// Funciones de hash para valores enteros
unsigned int hashCantidad(int cantidad) {
    return cantidad % HASH_SIZE;
}

unsigned int hashValorTotal(int valor) {
    return (valor / 10) % HASH_SIZE;
}

// Función de hash simple para cadenas de caracteres
unsigned int hash(const char* clave) {
    unsigned int hash_value = 0;
    while (*clave) {
        hash_value = (hash_value * 31) + *clave++;
    }
    return hash_value % HASH_SIZE;
}

unsigned int hashFecha(const char* fecha) {
    int anio, mes, dia;
    sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia);
    unsigned int hash_value = 0;
    hash_value = (hash_value * 31) + anio;
    hash_value = (hash_value * 31) + mes;
    hash_value = (hash_value * 31) + dia;
    return hash_value % HASH_SIZE;
}

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

// Instancia global del HashMap
HashMap hashMap;

// Función auxiliar para insertar un insumo en una tabla hash específica
void insertarEnTabla(Nodo* tabla[], unsigned int (*func_hash)(const void*), const void* clave, Insumo insumo) {
    unsigned int idx = func_hash(clave);
    Nodo* nuevoNodo = (Nodo*)malloc(sizeof(Nodo));
    if (!nuevoNodo) {
        printf("Error: No se pudo reservar memoria para el nuevo insumo.\n");
        return;
    }
    nuevoNodo->insumo = insumo;
    nuevoNodo->siguiente = tabla[idx];
    tabla[idx] = nuevoNodo;
}

// Funciones hash adaptadas para los tipos de clave
unsigned int hashCantidadPtr(const void* ptr) {
    int cantidad = *(const int*)ptr;
    return hashCantidad(cantidad);
}
unsigned int hashValorTotalPtr(const void* ptr) {
    int valor = *(const int*)ptr;
    return hashValorTotal(valor);
}
unsigned int hashStrPtr(const void* ptr) {
    return hash((const char*)ptr);
}
unsigned int hashFechaPtr(const void* ptr) {
    return hashFecha((const char*)ptr);
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

// Función para agregar insumo
void agregarInsumo() {
    Insumo nuevo;
    printf("\nIngrese la fecha (YYYY-MM-DD): ");
    scanf("%10s", nuevo.fecha);

    if (!esFechaValida(nuevo.fecha)) {
        printf("Fecha invalida. Debe tener formato YYYY-MM-DD y valores correctos.\n");
        return;
    }

    printf("Ingrese la categoria: ");
    scanf("%19s", nuevo.categoria);

    printf("Ingrese el nombre del producto: ");
    scanf("%19s", nuevo.producto);

    printf("Ingrese la cantidad: ");
    scanf("%d", &nuevo.cantidad);

    printf("Ingrese el valor total: ");
    scanf("%d", &nuevo.valor_total);

    if (nuevo.valor_total > saldo) {
        printf("Saldo insuficiente. No se puede agregar el insumo.\n");
        return;
    }

    saldo -= nuevo.valor_total;
    printf("Insumo agregado correctamente. Saldo restante: %d\n", saldo);

    // Insertar en las tablas hash
    insertarEnTabla(hashMap.tabla_fecha,      hashFechaPtr,     nuevo.fecha,      nuevo);
    insertarEnTabla(hashMap.tabla_categoria,  hashStrPtr,       nuevo.categoria,  nuevo);
    insertarEnTabla(hashMap.tabla_producto,   hashStrPtr,       nuevo.producto,   nuevo);
    insertarEnTabla(hashMap.tabla_cantidad,   hashCantidadPtr,  &nuevo.cantidad,  nuevo);
    insertarEnTabla(hashMap.tabla_valor_total,hashValorTotalPtr,&nuevo.valor_total,nuevo);
}


int main() {
    setlocale(LC_ALL, "");
    int opcion;

    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1. Cargar dinero a la billetera\n");
        printf("2. Agregar insumo\n");
        printf("3. Mostrar boletin semanal\n");
        printf("4. Mostrar boletin mensual\n");
        printf("5. Salir\n");
        printf("Seleccione una opcion: ");
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
                printf("Opción invalida. Intente nuevamente.\n");
        }

    } while(opcion != 5);

    return 0;
}
