#ifndef REGRESION_H
#define REGRESION_H

// Estructura para almacenar los resultados de la regresión
typedef struct {
    double pendiente;
    double intercepto;
} ModeloLineal;

// Función para calcular la regresión lineal simple
ModeloLineal calcular_regresion(double x[], double y[], int n);

#endif
