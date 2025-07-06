#include "map.h"
    #include "extra.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include <stdbool.h>
    #include <limits.h>
    #include <ctype.h>
    #include "regresion.h"
    #define MAX_LINE_LENGTH 4096
    #define MAX_FIELDS      128

    void limpiar_espacios(char* str);

    Map* categorias = NULL; 
    Map* productos = NULL;  


    char* get_producto_unificado(const char* producto_original) {
    char buffer[MAX_PRODUCTO];
    strncpy(buffer, producto_original, MAX_PRODUCTO - 1);
    buffer[MAX_PRODUCTO - 1] = '\0';
    limpiar_espacios(buffer);
    for (char* p = buffer; *p; ++p)
        *p = tolower(*p);

    MapPair* existente = map_search(productos, buffer);
    if (existente) return existente->key;

    char* copia = strdup(buffer);
    map_insert(productos, copia, NULL);
    return copia;
}

char* get_categoria_unificada(const char* categoria_original) {
    char buffer[MAX_CATEGORIA];
    strncpy(buffer, categoria_original, MAX_CATEGORIA - 1);
    buffer[MAX_CATEGORIA - 1] = '\0';
    limpiar_espacios(buffer);
    for (char* p = buffer; *p; ++p)
        *p = tolower(*p);

    MapPair* existente = map_search(categorias, buffer);
    if (existente) return existente->key;

    char* copia = strdup(buffer);
    map_insert(categorias, copia, NULL);
    return copia;
}




    int compare_keys(void* a, void* b) {
        return strcmp((char*)a, (char*)b);
    }
    int string_lower_than(void* a, void* b) {
        return strcmp((char*)a, (char*)b) < 0;
    }
    int is_equal_string(void* a, void* b) {
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

  void limpiar_espacios(char* str) {
    // Elimina espacios al inicio
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    // Elimina espacios al final
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

    // Mueve la cadena limpia al inicio del buffer original
    if (start != str) memmove(str, start, strlen(start) + 1);
}

void cargarDatasetDesdeCSV(Map* mapa, const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo: %s\n", nombreArchivo);
        return;
    }

    char** campos;
    int insumosCargados = 0;

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        // Limpiar espacios en los primeros 3 campos (fecha, categoría, producto)
        for (int i = 0; i < 3 && campos[i]; i++) {
            limpiar_espacios(campos[i]);
        }

        // Crear nuevo insumo
        Insumo* nuevoInsumo = (Insumo*)malloc(sizeof(Insumo));
        if (!nuevoInsumo) {
            printf("Error de memoria al crear insumo.\n");
            break;
        }

        // Copiar campos (ya limpios)
        strncpy(nuevoInsumo->fecha, campos[0], sizeof(nuevoInsumo->fecha) - 1);
        nuevoInsumo->fecha[sizeof(nuevoInsumo->fecha) - 1] = '\0';

        strncpy(nuevoInsumo->categoria, campos[1], sizeof(nuevoInsumo->categoria) - 1);
        nuevoInsumo->categoria[sizeof(nuevoInsumo->categoria) - 1] = '\0';

        strncpy(nuevoInsumo->producto, campos[2], sizeof(nuevoInsumo->producto) - 1);
        nuevoInsumo->producto[sizeof(nuevoInsumo->producto) - 1] = '\0';

        // Convertir cantidad y valor total
        nuevoInsumo->cantidad = atoi(campos[3]);
        nuevoInsumo->valor_total = atoi(campos[4]);

        // Insertar en el mapa agrupador
        insertar_insumo(mapa, nuevoInsumo);
        insumosCargados++;

        // Liberar campos si es necesario
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
        // Obtener claves unificadas
        char* categoria_key = get_categoria_unificada(insumo->categoria);

        // Buscar si la categoría ya existe en el mapa
        MapPair* pair = map_search(map, categoria_key);

        if (pair != NULL) {
            // La categoría existe → accedemos a la lista de insumos
            List* lista = (List*)pair->value;
            // NO agrupar por producto, solo agregar el insumo tal cual
            list_pushBack(lista, insumo);
        } else {
            // La categoría no existe → crear nueva lista y agregar insumo
            List* nueva_lista = list_create();
            list_pushBack(nueva_lista, insumo);
            map_insert(map, categoria_key, nueva_lista);
        }
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
    printf("\n--- BOLETÍN MENSUAL AGRUPADO POR CATEGORÍA, PRODUCTO Y FECHA ---\n");

    // Rango fijo: julio 2025
    struct tm tm_ini = {0}, tm_fin = {0};
    strptime("2025-07-01", "%Y-%m-%d", &tm_ini);
    strptime("2025-07-31", "%Y-%m-%d", &tm_fin);
    time_t t_ini = mktime(&tm_ini);
    time_t t_fin = mktime(&tm_fin);

    Map* resumen = map_create(compare_keys);
    map_set_equal(resumen, is_equal_string);
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

            // Normaliza producto y categoría
            char prod_key[MAX_PRODUCTO], cat_key[MAX_CATEGORIA], fecha_key[11], clave_compuesta[256];
            strncpy(prod_key, insumo->producto, MAX_PRODUCTO - 1);
            prod_key[MAX_PRODUCTO - 1] = '\0';
            limpiar_espacios(prod_key);
            for (char* p = prod_key; *p; ++p) *p = tolower(*p);

            strncpy(cat_key, insumo->categoria, MAX_CATEGORIA - 1);
            cat_key[MAX_CATEGORIA - 1] = '\0';
            limpiar_espacios(cat_key);
            for (char* p = cat_key; *p; ++p) *p = tolower(*p);

            strncpy(fecha_key, insumo->fecha, 10);
            fecha_key[10] = '\0';

            snprintf(clave_compuesta, sizeof(clave_compuesta), "%s|%s", prod_key, fecha_key);

            // Mapa de categorías
            Map* submap = NULL;
            MapPair* submap_pair = map_search(resumen, cat_key);
            if (submap_pair) {
                submap = (Map*)submap_pair->value;
            } else {
                submap = map_create(compare_keys);
                map_set_equal(submap, is_equal_string);
                map_insert(resumen, strdup(cat_key), submap);
            }

            // Agrupa por clave compuesta (producto|fecha)
            MapPair* prod_pair = map_search(submap, clave_compuesta);
            Insumo* acumulado = prod_pair ? (Insumo*)prod_pair->value : NULL;

            if (!acumulado) {
                acumulado = malloc(sizeof(Insumo));
                strncpy(acumulado->categoria, cat_key, MAX_CATEGORIA);
                strncpy(acumulado->producto, prod_key, MAX_PRODUCTO);
                strncpy(acumulado->fecha, fecha_key, 11);
                acumulado->cantidad = insumo->cantidad;
                acumulado->valor_total = insumo->valor_total;
                map_insert(submap, strdup(clave_compuesta), acumulado);
            } else {
                acumulado->cantidad += insumo->cantidad;
                acumulado->valor_total += insumo->valor_total;
            }

            total += insumo->valor_total;
            insumo = list_next(lista);
        }
        cat_pair = map_next(mapa_principal);
    }

    printf("\nTotal gastado en el mes: $%d\n", total);

    // Imprimir agrupado por categoría, producto y fecha
    MapPair* cat_it = map_first(resumen);
    while (cat_it != NULL) {
        printf("\nCategoría: %s\n", (char*)cat_it->key);
        Map* submap = (Map*)cat_it->value;
        MapPair* prod_it = map_first(submap);
        while (prod_it != NULL) {
            Insumo* insumo = (Insumo*)prod_it->value;
            // Separa producto y fecha de la clave compuesta
            char* sep = strchr((char*)prod_it->key, '|');
            if (sep) {
                *sep = '\0';
                printf("  - %s (%s): %d unidades - $%d\n",
                    (char*)prod_it->key, sep+1, insumo->cantidad, insumo->valor_total);
                *sep = '|'; // restaurar
            } else {
                printf("  - %s: %d unidades - $%d\n",
                    (char*)prod_it->key, insumo->cantidad, insumo->valor_total);
            }
            prod_it = map_next(submap);
        }
        cat_it = map_next(resumen);
    }

    // Liberar memoria del resumen
    cat_it = map_first(resumen);
    while (cat_it != NULL) {
        Map* submap = (Map*)cat_it->value;
        MapPair* prod_it = map_first(submap);
        while (prod_it != NULL) {
            free(prod_it->value);
            prod_it = map_next(submap);
        }
        map_clean(submap);
        free(submap);
        cat_it = map_next(resumen);
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

