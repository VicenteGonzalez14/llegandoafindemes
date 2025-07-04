#include "extra.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include "regresion.h"
#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128


int compare_keys(void* a, void* b) {
    return strcmp((char*)a, (char*)b);
}
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

void mostrarBoletinMensual(Map* mapa_principal) {
    printf("\n--- BOLETÍN MENSUAL AGRUPADO POR CATEGORÍA ---\n");

    time_t t_fin = time(NULL);
    time_t t_ini = t_fin - (30 * 24 * 60 * 60);

    Map* resumen = map_create(is_equal_string);
    int total = 0;

    MapPair* cat_pair = map_first(mapa_principal);
    while (cat_pair != NULL) {
        List* lista = (List*)cat_pair->value;
        Insumo* insumo = list_first(lista);

        while (insumo != NULL) {
            struct tm tm = {0};
            strptime(insumo->fecha, "%Y-%m-%d", &tm);
            time_t t_insumo = mktime(&tm);
            if (t_insumo < t_ini || t_insumo > t_fin) {
                insumo = list_next(lista);
                continue;
            }

            total += insumo->valor_total;

            // Buscar o crear submapa por categoría
            MapPair* submap = map_search(resumen, insumo->categoria);
            Map* productos = submap ? (Map*)submap->value : NULL;

            if (!productos) {
                productos = map_create(compare_keys);
                map_insert(resumen, strdup(insumo->categoria), productos);
            }

            // Agrupar por producto dentro de la categoría
            MapPair* prod_pair = map_search(productos, insumo->producto);
            Insumo* acumulado = prod_pair ? (Insumo*)prod_pair->value : NULL;

            if (!acumulado) {
                acumulado = malloc(sizeof(Insumo));
                *acumulado = *insumo;
                map_insert(productos, strdup(insumo->producto), acumulado);
            } else {
                acumulado->cantidad += insumo->cantidad;
                acumulado->valor_total += insumo->valor_total;
            }

            insumo = list_next(lista);
        }
        cat_pair = map_next(mapa_principal);
    }

    printf("\nTotal gastado en el mes: $%d\n", total);

    MapPair* r = map_first(resumen);
    while (r != NULL) {
        printf("\nCategoría: %s\n", (char*)r->key);
        Map* productos = (Map*)r->value;
        MapPair* p = map_first(productos);
        while (p != NULL) {
            Insumo* i = (Insumo*)p->value;
            printf("  - %s: %d unidades - $%d\n", i->producto, i->cantidad, i->valor_total);
            p = map_next(productos);
        }
        r = map_next(resumen);
    }

    // Limpiar memoria
    r = map_first(resumen);
    while (r != NULL) {
        Map* productos = (Map*)r->value;
        MapPair* p = map_first(productos);
        while (p != NULL) {
            free(p->value);
            free(p->key);
            p = map_next(productos);
        }
        map_clean(productos);
        free(productos);
        free(r->key);
        r = map_next(resumen);
    }
    map_clean(resumen);
}


float predecirGastoSemanalDesdeMapa(Map* mapa) {
    Map* gasto_por_semana = map_create(is_equal_string);
    time_t t_actual = time(NULL);

    MapPair* categoria_pair = map_first(mapa);
    while (categoria_pair != NULL) {
        List* lista = (List*)categoria_pair->value;
        Insumo* insumo = list_first(lista);
        while (insumo != NULL) {
            struct tm tm_insumo = {0};
            strptime(insumo->fecha, "%Y-%m-%d", &tm_insumo);
            time_t t_insumo = mktime(&tm_insumo);
            int semana = (int)(difftime(t_actual, t_insumo) / (60*60*24*7));

            char clave[20];
            sprintf(clave, "semana_%d", semana);

            MapPair* p = map_search(gasto_por_semana, clave);
            if (p) *(int*)p->value += insumo->valor_total;
            else {
                int* gasto = malloc(sizeof(int));
                *gasto = insumo->valor_total;
                map_insert(gasto_por_semana, strdup(clave), gasto);
            }
            insumo = list_next(lista);
        }
        categoria_pair = map_next(mapa);
    }

    int n = map_size(gasto_por_semana);
    if (n < 2) {
        map_clean(gasto_por_semana);
        return -1;
    }

    double x[n], y[n];
    int i = 0;
    MapPair* pair = map_first(gasto_por_semana);
    while (pair != NULL && i < n) {
        x[i] = i + 1;
        y[i] = *(int*)pair->value;
        i++;
        pair = map_next(gasto_por_semana);
    }

    ModeloLineal modelo = calcular_regresion(x, y, n);
    map_clean(gasto_por_semana);

    return modelo.pendiente * (n + 1) + modelo.intercepto;
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

