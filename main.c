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
    int año, mes, dia;
    
    // Asumimos que la fecha está en formato YYYY-MM-DD
    sscanf(fecha, "%4d-%2d-%2d", &año, &mes, &dia);

    //usamos todos los componentes de la fecha para generar un hash
    unsigned int hash_value = 0;
    hash_value = (hash_value * 31) + año;  // Multiplicamos por un número primo
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
    int año, mes, dia;
    if (sscanf(fecha, "%4d-%2d-%2d", &año, &mes, &dia) != 3)
        return 0;
    if (año < 1900 || año > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    // Validación simple, puedes mejorarla para meses/días específicos
    return 1;
}

// Función para agregar insumo
void agregarInsumo() {
    Insumo nuevo;
    printf("\nIngrese la fecha (YYYY-MM-DD): ");
    scanf("%10s", nuevo.fecha);

    if (!esFechaValida(nuevo.fecha)) {
        printf("Fecha inválida. Debe tener formato YYYY-MM-DD y valores correctos.\n");
        return;
    }

    printf("Ingrese la categoría: ");
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
