#include "extra.h"






#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128

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


#include "extra.h"
#include <time.h>

#define MAX_INSUMOS 100

// Arreglo global y contador de insumos
Insumo insumos[MAX_INSUMOS];
int totalInsumos = 0;

// Función auxiliar para determinar si una fecha está en los últimos N días
int estaEnUltimosNDias(char fechaStr[], int dias) {
    struct tm fecha = {0};
    int anio, mes, dia;
    sscanf(fechaStr, "%d-%d-%d", &anio, &mes, &dia);
    fecha.tm_year = anio - 1900; // Años desde 1900
    fecha.tm_mon = mes - 1;      // Meses desde 0
    fecha.tm_mday = dia;         // Día del mes 
    
    time_t t_actual = time(NULL);
    time_t t_fecha = mktime(&fecha);
    double diferencia = difftime(t_actual, t_fecha) / (60 * 60 * 24);

    return diferencia >= 0 && diferencia <= dias;
}

void mostrarBoletinSemanal() {
    printf("\n--- BOLETÍN SEMANAL ---\n");

    int totalGastado = 0;
    int maxGasto = 0;
    char principalInsumo[50] = "";

    for (int i = 0; i < totalInsumos; i++) {
        if (estaEnUltimosNDias(insumos[i].fecha, 7)) {
            printf("- %s: %d unidades, $%d\n", insumos[i].categoria, insumos[i].cantidad, insumos[i].valorTotal);
            totalGastado += insumos[i].valorTotal;

            if (insumos[i].valorTotal > maxGasto) {
                maxGasto = insumos[i].valorTotal;
                strcpy(principalInsumo, insumos[i].categoria);
            }
        }
    }

    printf("Total gastado esta semana: $%d\n", totalGastado);
    if (strlen(principalInsumo) > 0)
        printf("Principal insumo de la semana: %s\n", principalInsumo);
    else
        printf("No hay insumos registrados esta semana.\n");
}

void mostrarBoletinMensual() {
    printf("\n--- BOLETÍN MENSUAL ---\n");

    typedef struct {
        char categoria[50];
        int gasto;
    } TopInsumo;

    TopInsumo top3[3] = {{"", 0}, {"", 0}, {"", 0}};
    int totalGastado = 0;

    for (int i = 0; i < totalInsumos; i++) {
        if (estaEnUltimosNDias(insumos[i].fecha, 30)) {
            printf("- %s: %d unidades, $%d\n", insumos[i].categoria, insumos[i].cantidad, insumos[i].valorTotal);
            totalGastado += insumos[i].valorTotal;

            // Buscar si ya está en top
            int encontrado = 0;
            for (int j = 0; j < 3; j++) {
                if (strcmp(top3[j].categoria, insumos[i].categoria) == 0) {
                    top3[j].gasto += insumos[i].valorTotal;
                    encontrado = 1;
                    break;
                }
            }

            // Si no está, ver si reemplaza alguno
            if (!encontrado) {
                int minIdx = 0;
                for (int j = 1; j < 3; j++) {
                    if (top3[j].gasto < top3[minIdx].gasto) minIdx = j;
                }
                if (insumos[i].valorTotal > top3[minIdx].gasto) {
                    strcpy(top3[minIdx].categoria, insumos[i].categoria);
                    top3[minIdx].gasto = insumos[i].valorTotal;
                }
            }
        }
    }

    printf("Total gastado este mes: $%d\n", totalGastado);
    printf("Top 3 insumos del mes:\n");
    for (int i = 0; i < 3; i++) {
        if (top3[i].gasto > 0)
            printf("• %s: $%d\n", top3[i].categoria, top3[i].gasto);
    }
}

// Función para limpiar la pantalla
void limpiarPantalla() { system("clear"); }

void presioneTeclaParaContinuar() {
  puts("Presione una tecla para continuar...");
  getchar(); // Consume el '\n' del buffer de entrada
  getchar(); // Espera a que el usuario presione una tecla
}