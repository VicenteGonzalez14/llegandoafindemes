#include "extra.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
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
    linea[strcspn(linea, "\r\n")] = '\0';

    char *ptr = linea;
    while (*ptr && idx < MAX_FIELDS - 1) {
        char *start;

        if (*ptr == '\"') {
            // campo entrecomillado
            ptr++;              // saltar la comilla inicial
            start = ptr;
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

void cargarDatasetDesdeCSV(Map* mapa, const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "r");  // Corregido: comillas dobles para string
    if (!archivo) {
        printf("No se pudo abrir el archivo: %s\n", nombreArchivo);
        return;
    }

    char** campos;
    int fila = 0;
    int insumosCargados = 0;

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {  // Corregido: archivo, no archive
        if (fila++ == 0) continue;  // Saltar cabecera

        // Crear nuevo insumo
        Insumo* nuevoInsumo = (Insumo*)malloc(sizeof(Insumo));
        if (!nuevoInsumo) {
            printf("Error de memoria al crear insumo.\n");
            break;
        }

        // Asignar valores desde el CSV (con validación)
        strncpy(nuevoInsumo->fecha, campos[0], sizeof(nuevoInsumo->fecha) - 1);
        strncpy(nuevoInsumo->categoria, campos[1], sizeof(nuevoInsumo->categoria) - 1);
        strncpy(nuevoInsumo->producto, campos[2], sizeof(nuevoInsumo->producto) - 1);
        nuevoInsumo->cantidad = atoi(campos[3]);
        nuevoInsumo->valor_total = atoi(campos[4]);

        // Insertar en el mapa
        insertar_insumo(mapa, nuevoInsumo);
        insumosCargados++;

        // Liberar memoria de los campos del CSV (si es necesario)
        for (int i = 0; campos[i]; i++) {
            free(campos[i]);
        }
        free(campos);
    }

    fclose(archivo);
    printf("Se cargaron %d insumos desde el archivo.\n", insumosCargados);
}

List *split_string(const char *str, const char *delim) {
  List *result = list_create();
  char *token = strtok((char *)str, delim);

  while (token != NULL) {
    while (*token == ' ') {
      token++;
    }
    char *end = token + strlen(token) - 1;
    while (*end == ' ' && end > token) {
      *end = '\0';
      end--;
    }
    char *new_token = strdup(token);
    list_pushBack(result, new_token);
    token = strtok(NULL, delim);
  }
  return result;
}

// Libera un insumo individual (para usar en list_clean)
// Función para liberar un insumo
void liberar_insumo(void* data) {
    free((Insumo*)data);
}

// Función para liberar el mapa completo
void liberar_mapa(Map* map) {
    MapPair* pair = map_first(map);
    while (pair != NULL) {
        List* lista = (List*)pair->value;
        list_clean(lista);  // Asume que list.h tiene esta función
        free(lista);
        free(pair->key);  // Liberar la clave (categoría)
        pair = map_next(map);
    }
    map_clean(map);
}

// Función para insertar insumos (versión con Map genérico)
void insertar_insumo(Map* map, Insumo* insumo) {
    char* clave = strdup(insumo->categoria);
    MapPair* pair = map_search(map, clave);
    
    if (pair != NULL) {
        // La categoría existe: agregar a la lista existente
        List* lista = (List*)pair->value;
        list_pushBack(lista, insumo);
    } else {
        // Nueva categoría: crear lista y agregar al mapa
        List* nueva_lista = list_create();
        list_pushBack(nueva_lista, insumo);
        map_insert(map, clave, nueva_lista);
    }
    free(clave);
}

void buscarInsumosEnRangoDeFechas(Map* map, const char* fecha_inicio, const char* fecha_fin) {
    // Convertir fechas a timestamp
    struct tm tm_inicio = {0}, tm_fin = {0};
    strptime(fecha_inicio, "%Y-%m-%d", &tm_inicio);
    strptime(fecha_fin, "%Y-%m-%d", &tm_fin);
    
    time_t t_inicio = mktime(&tm_inicio);
    time_t t_fin = mktime(&tm_fin);

    // Recorrer todas las categorías del mapa
    MapPair* pair = map_first(map);
    while (pair != NULL) {
        List* lista_insumos = (List*)pair->value;
        Insumo* insumo = list_first(lista_insumos);
        
        while (insumo != NULL) {
            struct tm tm_insumo = {0};
            strptime(insumo->fecha, "%Y-%m-%d", &tm_insumo);
            time_t t_insumo = mktime(&tm_insumo);

            if (t_insumo >= t_inicio && t_insumo <= t_fin) {
                printf("Insumo [%s]: %s - %d unidades - $%d\n", 
                       insumo->categoria, insumo->producto, 
                       insumo->cantidad, insumo->valor_total);
            }
            insumo = list_next(lista_insumos);
        }
        pair = map_next(map);
    }
}
void mostrarBoletinSemanal(Map* map) {
    printf("\n--- BOLETINES SEMANALES ---\n");

    // Obtener rango de fechas extremas
    time_t t_min = LONG_MAX, t_max = 0;
    MapPair* pair = map_first(map);
    
    while (pair != NULL) {
        List* lista = (List*)pair->value;
        Insumo* insumo = list_first(lista);
        
        while (insumo != NULL) {
            struct tm tm = {0};
            strptime(insumo->fecha, "%Y-%m-%d", &tm);
            time_t t = mktime(&tm);
            
            if (t < t_min) t_min = t;
            if (t > t_max) t_max = t;
            
            insumo = list_next(lista);
        }
        pair = map_next(map);
    }

    if (t_min == LONG_MAX) {
        printf("No hay insumos registrados.\n");
        return;
    }

    // Generar reporte semanal
    for (time_t t_ini = t_min; t_ini <= t_max; t_ini += 7 * 24 * 60 * 60) {
        struct tm tm_ini = *localtime(&t_ini);
        struct tm tm_fin = tm_ini;
        tm_fin.tm_mday += 6;
        mktime(&tm_fin);

        char fecha_inicio[11], fecha_fin[11];
        strftime(fecha_inicio, sizeof(fecha_inicio), "%Y-%m-%d", &tm_ini);
        strftime(fecha_fin, sizeof(fecha_fin), "%Y-%m-%d", &tm_fin);

        printf("\nSemana del %s al %s:\n", fecha_inicio, fecha_fin);
        buscarInsumosEnRangoDeFechas(map, fecha_inicio, fecha_fin);
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

// Función para verificar si el insumo ya está en el HashMap


int insumo_categoria_lower_than(void* a, void* b) {
    Insumo* ia = (Insumo*)a;
    Insumo* ib = (Insumo*)b;
    return strcmp(ia->categoria, ib->categoria) < 0;
}

