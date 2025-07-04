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

void liberar_insumo(void* data) {
    free((Insumo*)data);
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
    printf("\n--- BOLETÍN MENSUAL ---\n");

    // 1. Configurar rango de fechas (últimos 30 días)
    time_t t_fin = time(NULL);
    time_t t_ini = t_fin - (30 * 24 * 60 * 60);
    
    // 2. Estructuras auxiliares
    Map* gasto_por_categoria = map_create(compare_keys);
    Map* gasto_por_semana = map_create(compare_keys);
    Map* detalle_por_dia = map_create(compare_keys);
    List* fechas_ordenadas = list_create();
    int total_gastado = 0;

<<<<<<< Updated upstream
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
=======
    // 3. Procesar todos los insumos del mapa principal
    MapPair* categoria_pair = map_first(mapa_principal);
>>>>>>> Stashed changes
    while (categoria_pair != NULL) {
        List* insumos_categoria = (List*)categoria_pair->value;
        Insumo* insumo = list_first(insumos_categoria);

        while (insumo != NULL) {
            struct tm tm_insumo = {0};
            strptime(insumo->fecha, "%Y-%m-%d", &tm_insumo);
            time_t t_insumo = mktime(&tm_insumo);

            if (t_insumo >= t_ini && t_insumo <= t_fin) {
                total_gastado += insumo->valor_total;

                // 3.1. Agrupar por categoría
                MapPair* cat_pair = map_search(gasto_por_categoria, insumo->categoria);
                if (cat_pair) {
                    *(int*)cat_pair->value += insumo->valor_total;
                } else {
                    int* total = malloc(sizeof(int));
                    *total = insumo->valor_total;
                    map_insert(gasto_por_categoria, strdup(insumo->categoria), total);
                }

                // 3.2. Agrupar por semana
                int semana = tm_insumo.tm_yday / 7;
                char clave_semana[15];
                sprintf(clave_semana, "Semana %d", semana + 1);
                
                MapPair* sem_pair = map_search(gasto_por_semana, clave_semana);
                if (sem_pair) {
                    *(int*)sem_pair->value += insumo->valor_total;
                } else {
                    int* total = malloc(sizeof(int));
                    *total = insumo->valor_total;
                    map_insert(gasto_por_semana, strdup(clave_semana), total);
                }

                // 3.3. Detalle por día
                MapPair* dia_pair = map_search(detalle_por_dia, insumo->fecha);
                List* lista_dia = dia_pair ? (List*)dia_pair->value : NULL;
                
                if (!lista_dia) {
                    lista_dia = list_create();
                    map_insert(detalle_por_dia, strdup(insumo->fecha), lista_dia);
                    list_sortedInsert(fechas_ordenadas, strdup(insumo->fecha), string_lower_than);
                }
                
                Insumo* copia_insumo = malloc(sizeof(Insumo));
                *copia_insumo = *insumo; // Copia segura
                list_pushBack(lista_dia, copia_insumo);
            }
            insumo = list_next(insumos_categoria);
        }
        categoria_pair = map_next(mapa_principal);
    }

    // 4. Mostrar resultados
    printf("\nTotal gastado: $%d\n", total_gastado);

    // 4.1. Top 3 categorías
    printf("\n🔝 Top 3 categorías:\n");
    char* top_categorias[3] = {NULL};
    int top_montos[3] = {0};

    MapPair* pair = map_first(gasto_por_categoria);
    while (pair != NULL) {
        int monto = *(int*)pair->value;
        for (int i = 0; i < 3; i++) {
            if (monto > top_montos[i]) {
                for (int j = 2; j > i; j--) {
                    top_montos[j] = top_montos[j-1];
                    top_categorias[j] = top_categorias[j-1];
                }
                top_montos[i] = monto;
                top_categorias[i] = pair->key;
                break;
            }
        }
        pair = map_next(gasto_por_categoria);
    }

    for (int i = 0; i < 3 && top_categorias[i]; i++) {
        printf("%d. %-15s $%d\n", i+1, top_categorias[i], top_montos[i]);
    }

    // 4.2. Gasto por semana
    printf("\n📅 Gasto semanal:\n");
    pair = map_first(gasto_por_semana);
    while (pair != NULL) {
        printf("• %-12s: $%d\n", (char*)pair->key, *(int*)pair->value);
        pair = map_next(gasto_por_semana);
    }

    // 4.3. Detalle diario
    printf("\n📆 Detalle diario:\n");
    char* meses[] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", 
                     "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
    char mes_actual[20] = "";

    char* fecha_str = list_first(fechas_ordenadas);
    while (fecha_str != NULL) {
        struct tm tm = {0};
        strptime(fecha_str, "%Y-%m-%d", &tm);
        const char* mes_nombre = meses[tm.tm_mon];

        // Encabezado de mes
        if (strcmp(mes_actual, mes_nombre) != 0) {
            printf("\n%s %d:\n", mes_nombre, tm.tm_year + 1900);
            strcpy(mes_actual, mes_nombre);
        }

        // Detalle del día
        MapPair* dia_pair = map_search(detalle_por_dia, fecha_str);
        if (dia_pair) {
            List* insumos_dia = (List*)dia_pair->value;
            Insumo* insumo = list_first(insumos_dia);
            
            while (insumo != NULL) {
                printf("  • %02d/%02d: %-20s %2d x $%d\n", 
                       tm.tm_mday, tm.tm_mon + 1,
                       insumo->producto, 
                       insumo->cantidad,
                       insumo->valor_total);
                insumo = list_next(insumos_dia);
            }
        }
        fecha_str = list_next(fechas_ordenadas);
    }

    // 5. Liberar memoria
    map_clean(gasto_por_categoria);
    map_clean(gasto_por_semana);
    
    // Liberar detalle_por_dia (requiere liberar las listas internas primero)
    MapPair* dia_pair = map_first(detalle_por_dia);
    while (dia_pair != NULL) {
        List* lista = (List*)dia_pair->value;
        list_clean(lista, liberar_insumo); // liberar_insumo debe estar definida
        free(dia_pair->key);
        dia_pair = map_next(detalle_por_dia);
    }
    map_clean(detalle_por_dia);
    
    list_clean(fechas_ordenadas);
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

int insumo_categoria_lower_than(void* a, void* b) {
    Insumo* ia = (Insumo*)a;
    Insumo* ib = (Insumo*)b;
    return strcmp(ia->categoria, ib->categoria) < 0;
}

