#include "extra.h"
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
    printf("\n--- BOLETÍN SEMANAL (agrupado por semana) ---\n");

    // Creamos un arreglo para marcar qué semanas ya imprimimos (hasta 54 semanas por año)
    int semanas_impresas[54] = {0};

    for (int semana = 1; semana <= 53; semana++) {
        int encontrados = 0;
        // Recorremos toda la tabla hash de fechas
        for (int i = 0; i < hashMap.capacidad; i++) {
            Nodo *nodo = hashMap.tabla_fecha[i];
            while (nodo) {
                struct tm tm_insumo = {0};
                strptime(nodo->insumo.fecha, "%Y-%m-%d", &tm_insumo);
                int semana_insumo = tm_insumo.tm_yday / 7 + 1;

                if (semana_insumo == semana) {
                    if (!semanas_impresas[semana]) {
                        printf("\nSemana %d:\n", semana);
                        semanas_impresas[semana] = 1;
                    }
                    encontrados++;
                    char mes_nombre[20];
                    strftime(mes_nombre, sizeof(mes_nombre), "%B", &tm_insumo);

                    printf("  - El día %d de %s, realizó la compra de '%s'. Valor: $%d\n",
                        tm_insumo.tm_mday, mes_nombre, nodo->insumo.producto, nodo->insumo.valor_total);
                }
                nodo = nodo->siguiente;
            }
        }
        // Si quieres mostrar semanas sin insumos, puedes descomentar esto:
        // if (!encontrados) printf("\nSemana %d:\n  No hay insumos registrados.\n", semana);
    }
}

void mostrarBoletinMensual() {
    printf("\n--- BOLETÍN MENSUAL ---\n");

    time_t t_actual = time(NULL);
    struct tm tm_actual = *localtime(&t_actual);

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
}




float predecirGastoSemanal() {
    // Paso 1: agrupar los gastos semanales
    int semanasMax = 10;
    int semanaActual = 0;
    int gastoSemanal[semanasMax];
    memset(gastoSemanal, 0, sizeof(gastoSemanal));

    // Obtener la fecha actual
    time_t t_actual = time(NULL);
    struct tm *fecha_actual = localtime(&t_actual);

    for (int i = 0; i < totalInsumos; i++) {
        struct tm fecha = {0};
        int anio, mes, dia;
        sscanf(insumos[i].fecha, "%d-%d-%d", &anio, &mes, &dia);
        fecha.tm_year = anio - 1900;
        fecha.tm_mon = mes - 1;
        fecha.tm_mday = dia;

        time_t t_insumo = mktime(&fecha);
        double dias_diferencia = difftime(t_actual, t_insumo) / (60 * 60 * 24);

        int semana = (int)(dias_diferencia / 7);
        if (semana < semanasMax)
            gastoSemanal[semana] += insumos[i].valor_total;

        if (semana > semanaActual)
            semanaActual = semana;
    }

    // Paso 2: preparar datos para regresión lineal
    int n = 0;
    float x[semanasMax], y[semanasMax];

    for (int i = 0; i < semanasMax; i++) {
        if (gastoSemanal[i] > 0) {
            x[n] = i + 1;  // semana 1, 2, ...
            y[n] = gastoSemanal[i];
            n++;
        }
    }

    if (n < 2) return -1; // no hay suficientes datos

    // Regresión lineal: y = a*x + b
    float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    for (int i = 0; i < n; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    float a = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    float b = (sum_y - a * sum_x) / n;

    float siguiente_semana = x[n - 1] + 1;
    return a * siguiente_semana + b;
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
    FILE *archivo = fopen(nombreArchivo, "w");
    if (!archivo) return;
    // Puedes escribir cabecera si lo deseas:
    // fprintf(archivo, "fecha,categoria,producto,cantidad,valor_total\n");
    for (int i = 0; i < totalInsumos; i++) {
        fprintf(archivo, "%s,%s,%s,%d,%d\n",
                insumos[i].fecha,
                insumos[i].categoria,
                insumos[i].producto,
                insumos[i].cantidad,
                insumos[i].valor_total);
    }
    fclose(archivo);
}

