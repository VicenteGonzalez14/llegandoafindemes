#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "tdas/extra.h"
#include "tdas/list.h"

#define HASHMAP_INICIAL 1000
#define FACTOR_CARGA 0.75  // Factor de carga para el redimensionamiento de la tabla

// Instancia global del HashMap
HashMap hashMap = {0};
int saldo = 0;
Insumo insumos[MAX_INSUMOS];
int totalInsumos = 0;

// Funciones de hash para valores enteros
unsigned int hashCantidad(int cantidad) {
    cantidad = cantidad ^ (cantidad >> 16);
    cantidad = cantidad * 0x85ebca6b;
    cantidad = cantidad ^ (cantidad >> 13);
    cantidad = cantidad * 0xc2b2ae35;
    cantidad = cantidad ^ (cantidad >> 16);
    return cantidad % HASH_SIZE;
}

unsigned int hashValorTotal(int valor) {
    valor = valor ^ (valor >> 16);
    valor = valor * 0x85ebca6b;
    valor = valor ^ (valor >> 13);
    valor = valor * 0xc2b2ae35;
    valor = valor ^ (valor >> 16);
    return valor % HASH_SIZE;
}

unsigned int hashStr(const char* clave) {
    unsigned int hash_value = 5381; // Número inicial estándar para djb2
    while (*clave) {
        hash_value = ((hash_value << 5) + hash_value) + (unsigned char)(*clave);
        clave++;
    }
    return hash_value % HASH_SIZE;
}

unsigned int hashFecha(const char* fecha) {
    int anio, mes, dia;
    sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia);
    
    unsigned int hash_value = anio;
    hash_value = (hash_value * 31) + mes;
    hash_value = (hash_value * 31) + dia;
    hash_value = hash_value ^ (hash_value >> 16);
    hash_value = hash_value * 0x85ebca6b;
    hash_value = hash_value ^ (hash_value >> 13);
    hash_value = hash_value * 0xc2b2ae35;
    hash_value = hash_value ^ (hash_value >> 16);

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
    return hashStr((const char*)ptr);
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
    FILE *archivo = fopen("insumos.csv", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return;
    }

    char fecha[11];
    int tipo;
    char *categorias[] = {
        "Alimentos", "Higiene", "Vestimenta", 
        "Tecnologia", "Electrodomesticos", "Mascotas"
    };
    char categoria[30];
    char producto[100];
    int cantidad;
    int costo;

    // Ingresar fecha
    printf("Ingrese la fecha (YYYY-MM-DD): ");
    scanf("%10s", fecha);
    if (!esFechaValida(fecha)) {
        printf("Fecha inválida. Por favor ingrese una fecha en formato YYYY-MM-DD.\n");
        fclose(archivo);
        return;
    }

    // Seleccionar categoría
    printf("Seleccione el tipo de insumo:\n");
    for (int i = 0; i < 6; i++) {
        printf("%d. %s\n", i + 1, categorias[i]);
    }
    printf("Opción: ");
    scanf("%d", &tipo);
    if (tipo < 1 || tipo > 6) {
        printf("Opción inválida. Cancelando ingreso.\n");
        fclose(archivo);
        return;
    }
    strcpy(categoria, categorias[tipo - 1]);

    getchar(); // Limpiar buffer del scanf
    printf("Ingrese el nombre del producto: ");
    fgets(producto, sizeof(producto), stdin);
    producto[strcspn(producto, "\n")] = 0; // Eliminar salto de línea

    // Ingresar cantidad y costo
    printf("Ingrese la cantidad: ");
    scanf("%d", &cantidad);
    if (cantidad <= 0) {
        printf("La cantidad debe ser mayor que cero.\n");
        fclose(archivo);
        return;
    }

    printf("Ingrese el costo total: ");
    scanf("%d", &costo);
    if (costo <= 0) {
        printf("El costo debe ser mayor que cero.\n");
        fclose(archivo);
        return;
    }

    // Verificar si es necesario redimensionar el HashMap
    if (hashMap.elementos >= hashMap.capacidad * FACTOR_CARGA) {
        redimensionarHashMap(&hashMap, hashMap.capacidad * 2);
    }

    // Guardar en archivo CSV
    fprintf(archivo, "%s,%s,%s,%d,%d\n", fecha, categoria, producto, cantidad, costo);
    fclose(archivo);

    // Crear insumo para agregar al hashMap
    Insumo nuevo;
    strcpy(nuevo.fecha, fecha);
    strcpy(nuevo.categoria, categoria);
    strcpy(nuevo.producto, producto);
    nuevo.cantidad = cantidad;
    nuevo.valor_total = costo;

    // Insertar en tablas hash
    insertarEnTabla(hashMap.tabla_fecha,        hashFechaPtr,       nuevo.fecha,     nuevo);
    insertarEnTabla(hashMap.tabla_categoria,    hashStrPtr,         nuevo.categoria, nuevo);
    insertarEnTabla(hashMap.tabla_producto,     hashStrPtr,         nuevo.producto,  nuevo);
    insertarEnTabla(hashMap.tabla_cantidad,     hashCantidadPtr,    &nuevo.cantidad, nuevo);
    insertarEnTabla(hashMap.tabla_valor_total,  hashValorTotalPtr,  &nuevo.valor_total, nuevo);

    hashMap.elementos++;
    insumos[totalInsumos++] = nuevo;

    saldo -= costo;

    printf("Insumo agregado correctamente.\n");
    printf("Saldo actual: %d\n", saldo);
}

// Funciones getter para cada tipo de clave
const char* obtenerFecha(const Insumo* insumo) {return insumo->fecha; }
const char* obtenerCategoria(const Insumo* insumo) { return insumo->categoria; }
const char* obtenerProducto(const Insumo* insumo) { return insumo->producto; }
int obtenerCantidad(const Insumo* insumo) { return insumo->cantidad; }
int obtenerValorTotal(const Insumo* insumo) { return insumo->valor_total; }

// Función para liberar la memoria de la tabla hash
void liberarTabla(Nodo **tabla, int capacidad) {
    for (int i = 0; i < capacidad; i++) {
        Nodo *actual = tabla[i];
        while (actual) {
            Nodo *tmp = actual;
            actual = actual->siguiente;
            free(tmp);
        }
    }
}

// Función para redimensionar el HashMap
void redimensionarHashMap(HashMap *mapa, int nueva_capacidad) {
    Nodo **nueva_fecha = calloc(nueva_capacidad, sizeof(Nodo*));
    Nodo **nueva_categoria = calloc(nueva_capacidad, sizeof(Nodo*));
    Nodo **nueva_producto = calloc(nueva_capacidad, sizeof(Nodo*));
    Nodo **nueva_cantidad = calloc(nueva_capacidad, sizeof(Nodo*));
    Nodo **nueva_valor_total = calloc(nueva_capacidad, sizeof(Nodo*));

    if (!nueva_fecha || !nueva_categoria || !nueva_producto || !nueva_cantidad || !nueva_valor_total) {
        printf("Error al asignar memoria para las nuevas tablas.\n");
        free(nueva_fecha);
        free(nueva_categoria);
        free(nueva_producto);
        free(nueva_cantidad);
        free(nueva_valor_total);
        return;
    }

    rehashTablaStr(mapa->tabla_fecha, mapa->capacidad, nueva_fecha, nueva_capacidad, hashFecha, obtenerFecha);
    rehashTablaStr(mapa->tabla_categoria, mapa->capacidad, nueva_categoria, nueva_capacidad, hashStr, obtenerCategoria);
    rehashTablaStr(mapa->tabla_producto, mapa->capacidad, nueva_producto, nueva_capacidad, hashStr, obtenerProducto);
    rehashTablaInt(mapa->tabla_cantidad, mapa->capacidad, nueva_cantidad, nueva_capacidad, hashCantidad, obtenerCantidad);
    rehashTablaInt(mapa->tabla_valor_total, mapa->capacidad, nueva_valor_total, nueva_capacidad, hashValorTotal, obtenerValorTotal);

    liberarTabla(mapa->tabla_fecha, mapa->capacidad);
    liberarTabla(mapa->tabla_categoria, mapa->capacidad);
    liberarTabla(mapa->tabla_producto, mapa->capacidad);
    liberarTabla(mapa->tabla_cantidad, mapa->capacidad);
    liberarTabla(mapa->tabla_valor_total, mapa->capacidad);

    mapa->tabla_fecha = nueva_fecha;
    mapa->tabla_categoria = nueva_categoria;
    mapa->tabla_producto = nueva_producto;
    mapa->tabla_cantidad = nueva_cantidad;
    mapa->tabla_valor_total = nueva_valor_total;
    mapa->capacidad = nueva_capacidad;
}

// Rehashing para tablas de tipo string
void rehashTablaStr(Nodo** tablaVieja, int capacidadVieja, Nodo** tablaNueva, int nueva_capacidad, unsigned int (*func_hash)(const char*), const char* (*obtenerClave)(const Insumo*)) {
    for (int i = 0; i < capacidadVieja; i++) {
        Nodo* actual = tablaVieja[i];
        while (actual) {
            const char* clave = obtenerClave(&actual->insumo);
            unsigned int idx = func_hash(clave) % nueva_capacidad;
            Nodo* nuevoNodo = malloc(sizeof(Nodo));
            nuevoNodo->insumo = actual->insumo;
            nuevoNodo->siguiente = tablaNueva[idx];
            tablaNueva[idx] = nuevoNodo;
            actual = actual->siguiente;
        }
    }
}

// Rehashing para tablas de tipo int
void rehashTablaInt(Nodo** tablaVieja, int capacidadVieja, Nodo** tablaNueva, int nueva_capacidad, unsigned int (*func_hash)(int), int (*obtenerClave)(const Insumo*)) {
    for (int i = 0; i < capacidadVieja; i++) {
        Nodo* actual = tablaVieja[i];
        while (actual) {
            int clave = obtenerClave(&actual->insumo);
            unsigned int idx = func_hash(clave) % nueva_capacidad;
            Nodo* nuevoNodo = malloc(sizeof(Nodo));
            nuevoNodo->insumo = actual->insumo;
            nuevoNodo->siguiente = tablaNueva[idx];
            tablaNueva[idx] = nuevoNodo;
            actual = actual->siguiente;
        }
    }
}

int main() {
    setlocale(LC_ALL, "");

    // Inicializa las tablas hash y contadores ANTES de cargar el dataset
    hashMap.tabla_fecha = calloc(HASHMAP_INICIAL, sizeof(Nodo*));
    hashMap.tabla_categoria = calloc(HASHMAP_INICIAL, sizeof(Nodo*));
    hashMap.tabla_producto = calloc(HASHMAP_INICIAL, sizeof(Nodo*));
    hashMap.tabla_cantidad = calloc(HASHMAP_INICIAL, sizeof(Nodo*));
    hashMap.tabla_valor_total = calloc(HASHMAP_INICIAL, sizeof(Nodo*));
    hashMap.capacidad = HASHMAP_INICIAL;
    hashMap.elementos = 0;
    saldo = 0;
    totalInsumos = 0;

    cargarDatasetDesdeCSV("insumos.csv"); // Carga los insumos al iniciar

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
                guardarTodosLosInsumosEnCSV("insumos.csv");
                printf("Saliendo del programa. ¡Hasta luego!\n");
                break;
            default:
                printf("Opción inválida. Intente nuevamente.\n");
        }

    } while(opcion != 5);

    return 0;
}


