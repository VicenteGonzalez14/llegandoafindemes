#include "extra.h"
#include <time.h>
#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128

Insumo insumos[MAX_INSUMOS];
int totalInsumos = 0;

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

void cargarDatasetDesdeCSV(const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "insumos.csv");
    if (!archivo) {
        printf("No se pudo abrir el archivo: %s\n", insumos);
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
        // strcpy(insumos[totalInsumos].producto, campos[2]); 
        insumos[totalInsumos].cantidad = atoi(campos[3]);
        insumos[totalInsumos].valor_total = atoi(campos[4]);

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
            printf("- %s: %d unidades, $%d\n", insumos[i].categoria, insumos[i].cantidad, insumos[i].valor_total);
            totalGastado += insumos[i].valor_total;

            if (insumos[i].valor_total > maxGasto) {
                maxGasto = insumos[i].valor_total;
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

    // Matriz para predicción (máx. 6 semanas)
    int semanaActual = 1;
    int semanas[6] = {0};
    int gastosPorSemana[6] = {0};
    int semanaGastos[6] = {0};
    int contadorSemanas = 0;

    for (int i = 0; i < totalInsumos; i++) {
        if (estaEnUltimosNDias(insumos[i].fecha, 30)) {
            printf("- %s: %d unidades, $%d\n", insumos[i].categoria, insumos[i].cantidad, insumos[i].valor_total);
            totalGastado += insumos[i].valor_total;

            // Top 3
            int encontrado = 0;
            for (int j = 0; j < 3; j++) {
                if (strcmp(top3[j].categoria, insumos[i].categoria) == 0) {
                    top3[j].gasto += insumos[i].valor_total;
                    encontrado = 1;
                    break;
                }
            }
            if (!encontrado) {
                int minIdx = 0;
                for (int j = 1; j < 3; j++) {
                    if (top3[j].gasto < top3[minIdx].gasto)
                        minIdx = j;
                }
                if (insumos[i].valor_total > top3[minIdx].gasto) {
                    strcpy(top3[minIdx].categoria, insumos[i].categoria);
                    top3[minIdx].gasto = insumos[i].valor_total;
                }
            }

            // Agrupar gasto semanalmente (muy simple: 7 días por bloque)
            int diasDesdeHoy = estaEnUltimosNDias(insumos[i].fecha, 30); // Reutiliza días para estimar semana
            int semana = (30 - diasDesdeHoy) / 7;

            if (semana >= 0 && semana < 6) {
                gastosPorSemana[semana] += insumos[i].valor_total;
                semanaGastos[semana] = 1;
            }
        }
    }

    printf("Total gastado este mes: $%d\n", totalGastado);
    printf("Top 3 insumos del mes:\n");
    for (int i = 0; i < 3; i++) {
        if (top3[i].gasto > 0)
            printf("• %s: $%d\n", top3[i].categoria, top3[i].gasto);
    }

    // --- REGRESIÓN LINEAL PREDICTIVA ---
    // Construir arrays de semanas y gastos
    int x[6], y[6], n = 0;
    for (int i = 0; i < 6; i++) {
        if (semanaGastos[i]) {
            x[n] = i + 1;
            y[n] = gastosPorSemana[i];
            n++;
        }
    }

    if (n >= 2) { // se necesita al menos 2 semanas para estimar
        int sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
        for (int i = 0; i < n; i++) {
            sum_x += x[i];
            sum_y += y[i];
            sum_xy += x[i] * y[i];
            sum_x2 += x[i] * x[i];
        }

        float a = (n * sum_xy - sum_x * sum_y) * 1.0 / (n * sum_x2 - sum_x * sum_x);
        float b = (sum_y - a * sum_x) / n;

        float prediccion = a * (x[n-1] + 1) + b;

        printf("\nPredicción de gasto para la próxima semana: $%.0f\n", prediccion);
        if (prediccion > 25000)
            printf("Advertencia: tu ritmo de gasto es elevado.\n");
        else
            printf("Estás gastando a un ritmo moderado.\n");
    } else {
        printf("\nNo hay suficientes datos para predecir el gasto semanal.\n");
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