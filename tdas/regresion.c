#include "regresion.h"
#include <stdio.h>

// Implementación de regresión lineal simple usando mínimos cuadrados
ModeloLineal calcular_regresion(double x[], double y[], int n) {
    ModeloLineal modelo;
    double suma_x = 0, suma_y = 0, suma_xy = 0, suma_x2 = 0;

    for (int i = 0; i < n; i++) {
        suma_x += x[i];
        suma_y += y[i];
        suma_xy += x[i] * y[i];
        suma_x2 += x[i] * x[i];
    }

    double denominador = n * suma_x2 - suma_x * suma_x;
    if (denominador == 0) {
        modelo.pendiente = 0;
        modelo.intercepto = 0;
        return modelo;
    }

    modelo.pendiente = (n * suma_xy - suma_x * suma_y) / denominador;
    modelo.intercepto = (suma_y - modelo.pendiente * suma_x) / n;

    return modelo;
}
