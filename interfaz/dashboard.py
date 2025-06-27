import pandas as pd
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score

# Cargar el dataset simulado (ajusta esta ruta si es necesario)
df = pd.read_csv('../modelo/insumos_con_predicciones.csv', parse_dates=['fecha'])

# Calcular la semana del año
df['semana'] = df['fecha'].dt.isocalendar().week

# Agrupar por semana y calcular el promedio de costo
df_semana = df.groupby('semana').agg({'costo_total': 'mean'}).reset_index()

# Definir variables para regresión lineal simple
X = df_semana[['semana']]
y = df_semana['costo_total']

# Entrenar el modelo
modelo = LinearRegression()
modelo.fit(X, y)
y_pred = modelo.predict(X)

# Coeficientes
pendiente = modelo.coef_[0]
intercepto = modelo.intercept_
r2 = r2_score(y, y_pred)

# Mostrar resultados
print(f"Modelo: y = {pendiente:.2f} * semana + {intercepto:.2f}")
print(f"R² = {r2:.4f}")

# Gráfico
plt.figure(figsize=(9, 5))
plt.scatter(X, y, color='blue', label='Promedio real por semana')
plt.plot(X, y_pred, color='red', label='Regresión lineal simple')
plt.title('Regresión lineal simple: Gasto promedio semanal')
plt.xlabel('Semana del año')
plt.ylabel('Costo total promedio')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
