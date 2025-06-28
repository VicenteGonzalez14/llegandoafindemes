#include "extra.h"
#include "regresion.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128


int string_lower_than(void* a, void* b) {
    return strcmp((char*)a, (char*)b) < 0;
}
int is_equal_string(void *a, void *b) {
    return strcmp((char*)a, (char*)b) == 0;
}

char **leer_linea_csv(FILE *archivo, char separador) {
    static char linea[MAX_LINE_LENGTH];
    static char *campos[MAX_FIELDS];
    int idx = 0;

    if (fgets(linea, MAX_LINE_LENGTH, archivo) == NULL)
        return NULL;  // fin de fichero

    // quitar salto de línea
    linea[strcspn(linea, "\r\n")] = '\0';

    char *ptr = linea;
    while (*ptr && idx < MAX_FIELDS - 1) {
        char *start;

        if (*ptr == '\"') {
            // campo entrecomillado
            ptr++;              // saltar la comilla inicial
            start = ptr;

            // compactar contenido: convertir "" → " y copiar el resto
            char *dest = ptr;
            while (*ptr) {
                if (*ptr == '\"' && *(ptr + 1) == '\"') {
                    *dest++ = '\"';  // una comilla literal
                    ptr += 2;        // saltar ambas
                }
                else if (*ptr == '\"') {
                    ptr++;           // fin del campo
                    break;
                }
                else {
                    *dest++ = *ptr++;
                }
            }
            *dest = '\0';        // terminar cadena

            // ahora ptr apunta justo después de la comilla de cierre
            if (*ptr == separador) ptr++;
        }
        else {
            // campo sin comillas
            start = ptr;
            while (*ptr && *ptr != separador)
                ptr++;
            if (*ptr == separador) {
                *ptr = '\0';
                ptr++;
            }
        }

        campos[idx++] = start;
    }

    campos[idx] = NULL;
    return campos;
}

char *strptime(const char *s, const char *format, struct tm *tm) {
    if (strcmp(format, "%Y-%m-%d") == 0) {
        int year, month, day;
        if (sscanf(s, "%d-%d-%d", &year, &month, &day) == 3) {
            tm->tm_year = year - 1900;
            tm->tm_mon = month - 1;
            tm->tm_mday = day;
            return (char*)s + 10;
        }
    }
    return NULL;
}

void cargarDatasetDesdeCSV(const char *nombreArchivo) {
    
    FILE *archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo: %s\n", nombreArchivo);
        return;
    }

    char **campos;
    int fila = 0;

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        if (fila++ == 0) continue;  // Saltar la cabecera

        if (totalInsumos >= MAX_INSUMOS) {
            printf("Límite de insumos alcanzado.\n");
            break;
        }

        strcpy(insumos[totalInsumos].fecha, campos[0]);
        strcpy(insumos[totalInsumos].categoria, campos[1]);
        strcpy(insumos[totalInsumos].producto, campos[2]); 
        insumos[totalInsumos].cantidad = atoi(campos[3]);
        insumos[totalInsumos].valor_total = atoi(campos[4]);

        // Insertar en las tablas hash
        insertarEnTabla(hashMap.tabla_fecha,      hashFechaPtr,     insumos[totalInsumos].fecha,       insumos[totalInsumos]);
        insertarEnTabla(hashMap.tabla_categoria,  hashStrPtr,       insumos[totalInsumos].categoria,  insumos[totalInsumos]);
        insertarEnTabla(hashMap.tabla_producto,   hashStrPtr,       insumos[totalInsumos].producto,   insumos[totalInsumos]);
        insertarEnTabla(hashMap.tabla_cantidad,   hashCantidadPtr,  &insumos[totalInsumos].cantidad,  insumos[totalInsumos]);
        insertarEnTabla(hashMap.tabla_valor_total,hashValorTotalPtr,&insumos[totalInsumos].valor_total,insumos[totalInsumos]);

        totalInsumos++;
    }

    fclose(archivo);
    printf("Se cargaron %d insumos desde el archivo.\n", totalInsumos);
}


List *split_string(const char *str, const char *delim) {
  List *result = list_create();
  char *token = strtok((char *)str, delim);

  while (token != NULL) {
    // Eliminar espacios en blanco al inicio del token
    while (*token == ' ') {
      token++;
    }

    // Eliminar espacios en blanco al final del token
    char *end = token + strlen(token) - 1;
    while (*end == ' ' && end > token) {
      *end = '\0';
      end--;
    }

    // Copiar el token en un nuevo string
    char *new_token = strdup(token);

    // Agregar el nuevo string a la lista
    list_pushBack(result, new_token);

    // Obtener el siguiente token
    token = strtok(NULL, delim);
  }

  return result;
}
void insertarEnTabla(Nodo* tabla[], unsigned int (*func_hash)(const void*), const void* clave, Insumo insumo) {
    unsigned int idx = func_hash(clave) % hashMap.capacidad;
    Nodo* nuevo = malloc(sizeof(Nodo));
    nuevo->insumo = insumo;
    nuevo->siguiente = tabla[idx];
    tabla[idx] = nuevo;
}

void buscarInsumosPorCategoria(const char *categoria) {
    unsigned int idx = hashStr(categoria);  // Usar hashStr para generar el índice
    Nodo *nodo = hashMap.tabla_categoria[idx];
    while (nodo) {
        if (strcmp(nodo->insumo.categoria, categoria) == 0) {
            // Mostrar los detalles del insumo
            printf("Insumo encontrado: %s - %s - %d - $%d\n", 
                   nodo->insumo.fecha, nodo->insumo.producto, 
                   nodo->insumo.cantidad, nodo->insumo.valor_total);
        }
        nodo = nodo->siguiente;
    }
}
void buscarInsumosPorFecha(const char *fecha) {
    unsigned int idx = hashFecha(fecha);  // Genera el índice usando la función hashFecha
    Nodo *nodo = hashMap.tabla_fecha[idx];
    while (nodo) {
        if (strcmp(nodo->insumo.fecha, fecha) == 0) {
            // Mostrar los detalles del insumo
            printf("Insumo encontrado: %s - %s - %d - $%d\n", 
                   nodo->insumo.producto, nodo->insumo.categoria, 
                   nodo->insumo.cantidad, nodo->insumo.valor_total);
        }
        nodo = nodo->siguiente;
    }
}


void buscarInsumosEnRangoDeFechas(const char* fecha_inicio, const char* fecha_fin) {
    // Convierte las fechas a formato timestamp para comparar
    struct tm tm_inicio = {0}, tm_fin = {0};
    strptime(fecha_inicio, "%Y-%m-%d", &tm_inicio);
    strptime(fecha_fin, "%Y-%m-%d", &tm_fin);
    
    time_t t_inicio = mktime(&tm_inicio);
    time_t t_fin = mktime(&tm_fin);

    // Recorre la tabla de fechas
    for (int i = 0; i < hashMap.capacidad; i++) {
        Nodo *nodo = hashMap.tabla_fecha[i];
        while (nodo) {
            struct tm tm_insumo = {0};
            strptime(nodo->insumo.fecha, "%Y-%m-%d", &tm_insumo);
            time_t t_insumo = mktime(&tm_insumo);

            if (t_insumo >= t_inicio && t_insumo <= t_fin) {
                printf("Insumo dentro del rango: %s - %d unidades - $%d\n", 
                        nodo->insumo.producto, nodo->insumo.cantidad, nodo->insumo.valor_total);
            }

            nodo = nodo->siguiente;
        }
    }
}

void mostrarBoletinSemanal() {
    printf("\n--- BOLETÍN SEMANAL ---\n");
    
    struct tm tm_ini = {0}, tm_fin = {0};
    strptime("2025-05-01", "%Y-%m-%d", &tm_ini);   // Inicio de mayo
    strptime("2025-07-31", "%Y-%m-%d", &tm_fin);   // Fin de julio
    time_t t_ini = mktime(&tm_ini);
    time_t t_fin = mktime(&tm_fin); +86399;


    char ultimo_mes[20] = "";
    for (int i = 0; i < hashMap.capacidad; i++) {
        Nodo *nodo = hashMap.tabla_fecha[i];
        while (nodo) {
            struct tm tm_insumo = {0};
            strptime(nodo->insumo.fecha, "%Y-%m-%d", &tm_insumo);
            time_t t_insumo = mktime(&tm_insumo);

            if (t_insumo >= t_ini && t_insumo <= t_fin) {
                // Formatear mes y día en español
                char mes_nombre[20];
                strftime(mes_nombre, sizeof(mes_nombre), "%B", &tm_insumo);

                // Imprimir el mes solo si cambia
                if (strcmp(ultimo_mes, mes_nombre) != 0) {
                    printf("%s:\n", mes_nombre);
                    strcpy(ultimo_mes, mes_nombre);
                }

                // Imprimir insumo en formato solicitado
                printf("  - El día %d de %s, realizó la compra de '%s'. Valor: $%d\n",
                    tm_insumo.tm_mday, mes_nombre, nodo->insumo.producto, nodo->insumo.valor_total);
            }
            nodo = nodo->siguiente;
        }
    }
    char categorias_mostradas[100][50];
    int categorias_count = 0;

    for (int i = 0; i < hashMap.capacidad; i++) {
        Nodo *nodo = hashMap.tabla_categoria[i];
        while (nodo) {
            int ya_mostrada = 0;
            for (int j = 0; j < categorias_count; j++) {
                if (strcmp(categorias_mostradas[j], nodo->insumo.categoria) == 0) {
                    ya_mostrada = 1;
                    break;
                }
            }
            if (!ya_mostrada) {
                strcpy(categorias_mostradas[categorias_count++], nodo->insumo.categoria);
                printf("\n--- Insumos de la categoría '%s' esta semana ---\n", nodo->insumo.categoria);
                buscarInsumosPorCategoria(nodo->insumo.categoria);
            }
            nodo = nodo->siguiente;
        }
    }
}


void mostrarBoletinMensual() {
    printf("\n--- BOLETÍN MENSUAL ---\n");

    struct tm tm_actual = {0};
    strptime("2025-07-31", "%Y-%m-%d", &tm_actual);
    time_t t_actual = mktime(&tm_actual);


    char fecha_fin[11], fecha_inicio[11];
    strftime(fecha_fin, sizeof(fecha_fin), "%Y-%m-%d", &tm_actual);

    // Retroceder 30 días
    t_actual -= 30 * 24 * 60 * 60;
    struct tm tm_inicio = *localtime(&t_actual);
    strftime(fecha_inicio, sizeof(fecha_inicio), "%Y-%m-%d", &tm_inicio);

    struct tm tm_ini = {0}, tm_fin = {0};
    strptime(fecha_inicio, "%Y-%m-%d", &tm_ini);
    strptime(fecha_fin, "%Y-%m-%d", &tm_fin);
    time_t t_ini = mktime(&tm_ini);
    time_t t_fin = mktime(&tm_fin);

    Map* gastoPorCategoria = map_create(is_equal_string);
    Map* gastoPorSemana = map_create(is_equal_string);
    Map* detallePorDia = map_create(is_equal_string);
    List* fechasOrdenadas = list_create();

    int totalGastado = 0;

    for (int i = 0; i < hashMap.capacidad; i++) {
        Nodo *nodo = hashMap.tabla_fecha[i];
        while (nodo) {
            struct tm fecha_insumo = {0};
            strptime(nodo->insumo.fecha, "%Y-%m-%d", &fecha_insumo);
            time_t t_insumo = mktime(&fecha_insumo);

            if (t_insumo >= t_ini && t_insumo <= t_fin) {
                int valor = nodo->insumo.valor_total;
                totalGastado += valor;

                // Categoría
                MapPair* par1 = map_search(gastoPorCategoria, nodo->insumo.categoria);
                int* gastoExistente = par1 ? (int*)par1->value : NULL;

                if (gastoExistente) {
                    *gastoExistente += valor;
                } else {
                    int* nuevo = malloc(sizeof(int));
                    *nuevo = valor;
                    map_insert(gastoPorCategoria, strdup(nodo->insumo.categoria), nuevo);
                }

                // Semana
                int semana = (fecha_insumo.tm_yday / 7) + 1;
                char claveSemana[10];
                sprintf(claveSemana, "S%d", semana);
                MapPair* par2 = map_search(gastoPorSemana, claveSemana);
                int* gastoSem = par2 ? (int*)par2->value : NULL;

                if (gastoSem) {
                    *gastoSem += valor;
                } else {
                    int* nuevo = malloc(sizeof(int));
                    *nuevo = valor;
                    map_insert(gastoPorSemana, strdup(claveSemana), nuevo);
                }

                // Detalle por día
                MapPair* par3 = map_search(detallePorDia, nodo->insumo.fecha);
                List* lista = par3 ? (List*)par3->value : NULL;

                if (!lista) {
                    lista = list_create();
                    map_insert(detallePorDia, strdup(nodo->insumo.fecha), lista);
                    list_sortedInsert(fechasOrdenadas, strdup(nodo->insumo.fecha), string_lower_than);
                }
                Insumo* nuevo = malloc(sizeof(Insumo));
                *nuevo = nodo->insumo;
                list_pushBack(lista, nuevo);
            }
            nodo = nodo->siguiente;
        }
    }

    printf("\nTotal gastado en los últimos 30 días: $%d\n", totalGastado);

    // Top 3 categorías
    printf("\nTop 3 categorías más gastadas:\n");
    char* topCat[3] = {NULL, NULL, NULL};
    int topGasto[3] = {0};

    MapPair* par = map_first(gastoPorCategoria);
    while (par) {
        int gasto = *(int*)par->value;
        for (int i = 0; i < 3; i++) {
            if (gasto > topGasto[i]) {
                for (int j = 2; j > i; j--) {
                    topGasto[j] = topGasto[j-1];
                    topCat[j] = topCat[j-1];
                }
                topGasto[i] = gasto;
                topCat[i] = par->key;
                break;
            }
        }
        par = map_next(gastoPorCategoria);
    }

    for (int i = 0; i < 3 && topCat[i]; i++)
        printf("%d. %s: $%d\n", i+1, topCat[i], topGasto[i]);

    // Gasto por semana
    printf("\nGasto por semana:\n");
    par = map_first(gastoPorSemana);
    while (par) {
        printf("%s: $%d\n", (char*)par->key, *(int*)par->value);
        par = map_next(gastoPorSemana);
    }

    // Gasto detallado
    printf("\nGasto diario detallado:\n");
    char mes_actual[20] = "";
    char* meses[] = {"enero", "febrero", "marzo", "abril", "mayo", "junio",
                     "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"};

    char* fecha = list_first(fechasOrdenadas);
    while (fecha) {
        int anio, mes, dia;
        sscanf(fecha, "%d-%d-%d", &anio, &mes, &dia);
        const char* nombreMes = meses[mes - 1];

        if (strcmp(mes_actual, nombreMes) != 0) {
            printf("\n%s:\n", nombreMes);
            strcpy(mes_actual, nombreMes);
        }

        MapPair* par4 = map_search(detallePorDia, fecha);
        if (!par4) {
            fecha = list_next(fechasOrdenadas);
            continue;
        }
        List* lista = (List*)par4->value;

        Insumo* insumo = list_first(lista);
        while (insumo) {
            printf("- El día %d gastaste %d unidad en %s. Valor: $%d\n",
                dia, insumo->cantidad, insumo->producto, insumo->valor_total);
            insumo = list_next(lista);
        }

        fecha = list_next(fechasOrdenadas);
    }

        // ===== Predicción del gasto para el próximo mes (agosto) usando regresión lineal =====
    printf("\n\n--- Predicción de gasto para el próximo mes ---\n");

    // Paso 1: Mapear gastos por mes
    int gastosMensuales[12] = {0}; // Índice 0: enero, 1: febrero, ..., 11: diciembre

    for (int i = 0; i < hashMap.capacidad; i++) {
        Nodo* nodo = hashMap.tabla_fecha[i];
        while (nodo) {
            struct tm fecha_insumo = {0};
            strptime(nodo->insumo.fecha, "%Y-%m-%d", &fecha_insumo);
            int mes = fecha_insumo.tm_mon; // 0-based (0=enero)

            gastosMensuales[mes] += nodo->insumo.valor_total;
            nodo = nodo->siguiente;
        }
    }

    // Paso 2: Llenar arreglos de x (meses) e y (gastos)
    double x[12], y[12];
    int n = 0;
    for (int i = 0; i < 12; i++) {
        if (gastosMensuales[i] > 0) {
            x[n] = i + 1; // Mes 1 a 12
            y[n] = gastosMensuales[i];
            n++;
        }
    }

    if (n >= 2) {
        ModeloLineal modelo = calcular_regresion(x, y, n);
        double gasto_predicho = modelo.pendiente * (x[n - 1] + 1) + modelo.intercepto;

        // Determinar el nombre del próximo mes
        const char* meses[] = {"enero", "febrero", "marzo", "abril", "mayo", "junio",
                               "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"};
        int siguiente_mes = (int)x[n - 1]; // x[n-1] ya es el número de mes actual (1-12)
        if (siguiente_mes < 12) siguiente_mes++;
        const char* mes_predicho = meses[siguiente_mes - 1]; // 0-indexado


        printf("Según el análisis de tus últimos %d meses, se predice un gasto de $%.0f para %s.\n",
               n, gasto_predicho, mes_predicho);
    } else {
        printf("No hay suficiente historial para hacer una predicción.\n");
    }

      

}


// Función para limpiar la pantalla
void limpiarPantalla() { system("clear"); }

void presioneTeclaParaContinuar() {
  puts("Presione una tecla para continuar...");
  getchar(); // Consume el '\n' del buffer de entrada
  getchar(); // Espera a que el usuario presione una tecla
}

void guardarInsumoEnCSV(const Insumo *insumo, const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "a");
    if (!archivo) return;
    fprintf(archivo, "%s,%s,%s,%d,%d\n",
            insumo->fecha,
            insumo->categoria,
            insumo->producto,
            insumo->cantidad,
            insumo->valor_total);
    fclose(archivo);
}


void guardarTodosLosInsumosEnCSV(const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "a"); // Usamos "a" para agregar insumos nuevos
    if (!archivo) {
        return;  // No se pudo abrir el archivo para escritura
    }

    // Abrir archivo en modo lectura para verificar duplicados
    FILE *archivoLectura = fopen(nombreArchivo, "r");
    if (!archivoLectura) {
        fclose(archivo);
        return;  // No se pudo abrir el archivo para lectura
    }

    char linea[255];
    char *fecha, *categoria, *producto;
    int cantidad, valor_total;

    // Leer el archivo y verificar si el insumo ya está en el archivo
    while (fgets(linea, sizeof(linea), archivoLectura)) {
        // Compara el insumo con los que ya están en el archivo
        sscanf(linea, "%s,%s,%s,%d,%d", fecha, categoria, producto, &cantidad, &valor_total);

        // Compara con los insumos en memoria (hashMap)
        for (int i = 0; i < totalInsumos; i++) {
            if (strcmp(insumos[i].fecha, fecha) == 0 && 
                strcmp(insumos[i].categoria, categoria) == 0 && 
                strcmp(insumos[i].producto, producto) == 0) {
                // Si el insumo ya está en el archivo, no lo agregamos
                fclose(archivoLectura);
                fclose(archivo);
                return;
            }
        }
    }

    for (int i = 0; i < totalInsumos; i++) {
        fprintf(archivo, "%s,%s,%s,%d,%d\n", 
                insumos[i].fecha,
                insumos[i].categoria,
                insumos[i].producto,
                insumos[i].cantidad,
                insumos[i].valor_total);
    }

    fclose(archivoLectura);  // Cerrar archivo de lectura
    fclose(archivo);         // Cerrar archivo de escritura
}
int insumo_categoria_lower_than(void* a, void* b) {
    Insumo* ia = (Insumo*)a;
    Insumo* ib = (Insumo*)b;
    return strcmp(ia->categoria, ib->categoria) < 0;
}

