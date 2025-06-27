import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
import os

RUTA_CSV = '../modelo/insumos_con_predicciones.csv'
billetera = 0
saldo_label = None  # Para mostrar saldo actual

# Traducción de meses
meses_es = {
    'January': 'Enero', 'February': 'Febrero', 'March': 'Marzo',
    'April': 'Abril', 'May': 'Mayo', 'June': 'Junio',
    'July': 'Julio', 'August': 'Agosto', 'September': 'Septiembre',
    'October': 'Octubre', 'November': 'Noviembre', 'December': 'Diciembre'
}

def convertir_fechas(df):
    if 'fecha' in df.columns:
        df['fecha'] = pd.to_datetime(df['fecha'], errors='coerce')
        df = df.dropna(subset=['fecha'])
    return df

def actualizar_saldo():
    if saldo_label:
        saldo_label.config(text=f"Saldo: ${billetera:.2f}")

def agregar_dinero():
    global billetera
    monto = simpledialog.askfloat("Agregar Dinero", "Ingresa el monto a agregar:")
    if monto is not None and monto > 0:
        billetera += monto
        actualizar_saldo()
        ventana_mensaje("Éxito", f"Saldo actual: ${billetera:.2f}", "info")
    else:
        ventana_mensaje("Inválido", "Monto no válido.", "warning")

def agregar_insumo():
    def guardar_insumo():
        fecha = entrada_fecha.get()
        categoria = combo_categoria.get()
        producto = entrada_producto.get()
        cantidad = entrada_cantidad.get()
        costo = entrada_costo.get()

        try:
            cantidad = int(cantidad)
            costo = float(costo)
        except:
            ventana_mensaje("Error", "Cantidad y costo deben ser numéricos.", "error")
            return

        if not all([fecha, categoria, producto]):
            ventana_mensaje("Faltan datos", "Todos los campos son obligatorios.", "warning")
            return

        try:
            global billetera
            if costo > billetera:
                ventana_mensaje("Saldo insuficiente", f"No tienes suficiente dinero.\nSaldo: ${billetera:.2f}, Costo: ${costo:.2f}", "warning")
                return

            if os.path.exists(RUTA_CSV):
                df = pd.read_csv(RUTA_CSV, parse_dates=['fecha'])
            else:
                df = pd.DataFrame(columns=["fecha", "categoria", "producto", "cantidad", "costo_total"])

            nuevo = pd.DataFrame([{
                "fecha": fecha,
                "categoria": categoria,
                "producto": producto,
                "cantidad": cantidad,
                "costo_total": costo
            }])
            df = pd.concat([df, nuevo], ignore_index=True)
            df.to_csv(RUTA_CSV, index=False)

            billetera -= costo
            actualizar_saldo()
            ventana_mensaje("Insumo agregado", f"Insumo agregado correctamente.\nSaldo restante: ${billetera:.2f}", "info")
            ventana_insumo.destroy()
        except Exception as e:
            ventana_mensaje("Error", f"No se pudo agregar el insumo: {e}", "error")

    ventana_insumo = tk.Toplevel()
    ventana_insumo.title("Agregar insumo")
    ventana_insumo.geometry("350x300")

    tk.Label(ventana_insumo, text="Fecha (YYYY-MM-DD):").pack()
    entrada_fecha = tk.Entry(ventana_insumo)
    entrada_fecha.pack()

    tk.Label(ventana_insumo, text="Categoría:").pack()
    combo_categoria = ttk.Combobox(ventana_insumo, values=["Alimentos", "Higiene", "Tecnología", "Mascotas", "Otros"])
    combo_categoria.pack()

    tk.Label(ventana_insumo, text="Producto:").pack()
    entrada_producto = tk.Entry(ventana_insumo)
    entrada_producto.pack()

    tk.Label(ventana_insumo, text="Cantidad:").pack()
    entrada_cantidad = tk.Entry(ventana_insumo)
    entrada_cantidad.pack()

    tk.Label(ventana_insumo, text="Costo total:").pack()
    entrada_costo = tk.Entry(ventana_insumo)
    entrada_costo.pack()

    tk.Button(ventana_insumo, text="Guardar", command=guardar_insumo).pack(pady=10)


def mostrar_boletin_mensual():
    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return
        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)

        df['mes_nombre'] = df['fecha'].dt.strftime('%B').map(meses_es)

        lista_mensual = df.groupby(['mes_nombre', 'producto'])['costo_total'].sum().reset_index()
        resumen_texto = ""
        for mes, grupo in lista_mensual.groupby('mes_nombre'):
            resumen_texto += f"{mes}:\n"
            for _, row in grupo.iterrows():
                resumen_texto += f"  - {row['producto']} (${int(row['costo_total'])})\n"
            resumen_texto += "\n"

        ranking = df.groupby(['mes_nombre', 'producto'])['costo_total'].sum().reset_index()
        top3 = ranking.sort_values(['mes_nombre', 'costo_total'], ascending=[True, False]).groupby('mes_nombre').head(3)

        mensaje = ""
        for mes, grupo in top3.groupby('mes_nombre'):
            mensaje += f"{mes}:\n"
            for i, (_, row) in enumerate(grupo.iterrows(), start=1):
                mensaje += f"  {i}.- {row['producto']} (${int(row['costo_total'])})\n"
            mensaje += "\n"

        ventana_mensaje("Listado mensual", resumen_texto.strip(), "info")
        ventana_mensaje("Top 3 insumos más costosos por mes", mensaje.strip(), "info")

    except Exception as e:
        ventana_mensaje("Error", f"No se pudo generar el boletín mensual: {e}", "error")

    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return
        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)
        df['mes_nombre'] = df['fecha'].dt.strftime('%B').map(meses_es)
        resumen = df.groupby('mes_nombre')['costo_total'].sum()
        resumen.plot(kind='bar', figsize=(8,5), title='Gasto mensual (meses en español)', color='skyblue')
        plt.xlabel('Mes')
        plt.ylabel('Costo total')
        plt.tight_layout()
        plt.show()

    except Exception as e:
        ventana_mensaje("Error", f"No se pudo generar el boletín mensual: {e}", "error")


def mostrar_boletin_semanal():
    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return
        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)

        df['semana'] = df['fecha'].dt.isocalendar().week

        lista = df.groupby(['semana', 'producto'])['costo_total'].sum().reset_index()
        resumen = ""
        for semana, grupo in lista.groupby('semana'):
            resumen += f"Semana {int(semana)}:\n"
            for _, row in grupo.iterrows():
                resumen += f"  - {row['producto']} (${int(row['costo_total'])})\n"
            resumen += "\n"

        top = lista.sort_values(['semana', 'costo_total'], ascending=[True, False]).groupby('semana').head(1)
        ranking = ""
        for _, row in top.iterrows():
            ranking += f"Semana {int(row['semana'])}: {row['producto']} (${int(row['costo_total'])})\n"

        ventana_mensaje("Insumos comprados por semana", resumen.strip(), "info")
        ventana_mensaje("Producto más costoso por semana", ranking.strip(), "info")

    except Exception as e:
        ventana_mensaje("Error", f"No se pudo generar el boletín semanal: {e}", "error")

    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return
        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)
        df['semana'] = df['fecha'].dt.isocalendar().week
        resumen = df.groupby('semana')['costo_total'].sum()
        resumen.plot(kind='bar', figsize=(8,5), title='Gasto semanal (número de semana)')
        plt.xlabel('Semana del año')
        plt.ylabel('Costo total')
        plt.tight_layout()
        plt.show()

    except Exception as e:
        ventana_mensaje("Error", f"No se pudo generar el boletín semanal: {e}", "error")

def mostrar_regresion_simple():
    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return
        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)
        df['semana'] = df['fecha'].dt.isocalendar().week
        df_semana = df.groupby('semana').agg({'costo_total': 'mean'}).reset_index()

        X = df_semana[['semana']]
        y = df_semana['costo_total']
        modelo = LinearRegression()
        modelo.fit(X, y)
        y_pred = modelo.predict(X)

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
    except Exception as e:
        ventana_mensaje("Error", f"No se pudo mostrar la regresión: {e}", "error")

def mostrar_prediccion_proximo_mes():
    try:
        if not os.path.exists(RUTA_CSV):
            ventana_mensaje("Advertencia", "No hay datos para mostrar.", "warning")
            return

        df = pd.read_csv(RUTA_CSV)
        df = convertir_fechas(df)
        if 'fecha' not in df.columns or 'categoria' not in df.columns or 'costo_total' not in df.columns:
            ventana_mensaje("Error", "El archivo no tiene las columnas necesarias.", "error")
            return

        df['mes'] = df['fecha'].dt.month
        df['anio'] = df['fecha'].dt.year

        # Tomar solo los últimos 12 meses para la predicción
        df = df.sort_values('fecha')
        ultimos_12 = df[df['fecha'] >= (df['fecha'].max() - pd.DateOffset(months=12))]

        mensajes = ""
        for categoria in ultimos_12['categoria'].unique():
            datos = ultimos_12[ultimos_12['categoria'] == categoria]
            if datos['mes'].nunique() < 2:
                continue  # No hay suficientes datos para predecir

            # Agrupar por año y mes
            gastos_mes = datos.groupby(['anio', 'mes'])['costo_total'].sum().reset_index()
            gastos_mes['mes_num'] = range(1, len(gastos_mes) + 1)

            X = gastos_mes[['mes_num']]
            y = gastos_mes['costo_total']
            modelo = LinearRegression()
            modelo.fit(X, y)
            pred_prox = modelo.predict([[X['mes_num'].max() + 1]])[0]
            actual = y.iloc[-1]
            if actual == 0:
                continue
            variacion = ((pred_prox - actual) / actual) * 100

            if variacion > 0:
                mensajes += f"Para la categoría '{categoria}', se estima un AUMENTO del {variacion:.1f}% en insumos el próximo mes.\n\n"
            else:
                mensajes += f"Para la categoría '{categoria}', se estima una DISMINUCIÓN del {abs(variacion):.1f}% en insumos el próximo mes.\n\n"

        if mensajes:
            ventana_mensaje("Predicción de gastos para el próximo mes", f"\n{mensajes.strip()}\n", "info")
        else:
            ventana_mensaje("Predicción de gastos para el próximo mes", "\nNo hay suficientes datos para realizar una predicción.\n", "info")

    except Exception as e:
        ventana_mensaje("Error", f"\nNo se pudo calcular la predicción: {e}\n", "error")

def salir():
    ventana.quit()

def ventana_mensaje(titulo, mensaje, tipo="info"):
    colores = {
        "info":  ("#e3f2fd", "#1565c0", "💡"),
        "warning": ("#fffde7", "#f9a825", "⚠️"),
        "error": ("#ffebee", "#c62828", "❌")
    }
    bg, fg, icono = colores.get(tipo, colores["info"])
    popup = tk.Toplevel(ventana)
    popup.title(titulo)
    popup.configure(bg=bg)
    popup.geometry("480x320")
    popup.resizable(False, False)
    popup.grab_set()

    tk.Label(popup, text=icono, font=("Arial", 32), bg=bg).pack(pady=(14, 0))

    # Si el mensaje es largo, usar Text + Scrollbar
    if len(mensaje) > 300 or "\n" in mensaje:
        frame_text = tk.Frame(popup, bg=bg)
        frame_text.pack(pady=(10, 10), padx=10, fill="both", expand=True)
        text = tk.Text(frame_text, font=("Arial", 13), bg=bg, fg=fg, wrap="word", height=8, relief="flat", borderwidth=0)
        text.insert("1.0", mensaje)
        text.config(state="disabled")
        text.pack(side="left", fill="both", expand=True)
        scroll = tk.Scrollbar(frame_text, command=text.yview)
        scroll.pack(side="right", fill="y")
        text.config(yscrollcommand=scroll.set)
    else:
        tk.Label(popup, text=mensaje, font=("Arial", 13), bg=bg, fg=fg, wraplength=420, justify="center").pack(pady=(12, 18))

    tk.Button(popup, text="Cerrar", command=popup.destroy, bg=fg, fg="white", font=("Arial", 11, "bold"), relief="flat", width=12, cursor="hand2").pack(pady=(0, 10))
    popup.transient(ventana)
    popup.wait_window(popup)

def on_enter(e):
    e.widget['background'] = '#b3e0ff'

def on_leave(e):
    e.widget['background'] = '#e1e1e1'

ventana = tk.Tk()
ventana.title("Sistema de Gestión de Gastos")
ventana.geometry("480x600")
ventana.configure(bg="#f7fbfc")

frame = tk.Frame(ventana, bg="#f7fbfc")
frame.pack(pady=30)

saldo_label = tk.Label(frame, text=f"Saldo: ${billetera:.2f}", font=("Arial", 14, "bold"), bg="#f7fbfc", fg="#2e7d32")
saldo_label.pack(pady=(0, 18))

tk.Label(frame, text="Menú principal", font=("Arial", 20, "bold"), bg="#f7fbfc", fg="#1565c0").pack(pady=(0, 18))

# Línea separadora
tk.Frame(frame, height=2, width=380, bg="#90caf9").pack(pady=(0, 18))

estilo_btn = {
    'width': 34, 'height': 2, 'bg': '#e1e1e1', 'font': ('Arial', 12, 'bold'), 'bd': 0, 'relief': 'ridge', 'cursor': 'hand2'
}

botones = [
    ("💰 Agregar dinero a la billetera", agregar_dinero),
    ("➕ Agregar nuevo insumo", agregar_insumo),
    ("📅 Mostrar boletín mensual", mostrar_boletin_mensual),
    ("📈 Mostrar boletín semanal", mostrar_boletin_semanal),
    ("📊 Ver regresión simple", mostrar_regresion_simple),
    ("🔮 Mostrar predicción próximo mes", mostrar_prediccion_proximo_mes),
    ("❌ Salir", salir)
]

for texto, comando in botones:
    btn = tk.Button(frame, text=texto, command=comando, **estilo_btn)
    btn.pack(pady=7)
    btn.bind("<Enter>", on_enter)
    btn.bind("<Leave>", on_leave)

ventana.mainloop()
