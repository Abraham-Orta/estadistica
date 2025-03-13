// Inclusión de librerías necesarias
#include <QApplication>           // Clase base para aplicaciones Qt
#include <QMainWindow>            // Ventana principal
#include <QWidget>                // Componente base para elementos UI
#include <QVBoxLayout>            // Layout vertical
#include <QHBoxLayout>            // Layout horizontal
#include <QTableWidget>           // Tabla para mostrar datos
#include <QPushButton>            // Botones interactivos
#include <QTextEdit>              // Área de texto editable
#include <QFileDialog>            // Diálogo para selección de archivos
#include <QMessageBox>            // Mensajes emergentes
#include <QHeaderView>            // Configuración de cabeceras de tabla
#include <QVector>                // Contenedor dinámico tipo vector
#include <QFile>                  // Manejo de archivos
#include <QTextStream>            // Lectura/escritura de texto
#include <algorithm>              // Funciones STL (sort, min_element, etc.)
#include <cmath>                  // Funciones matemáticas
#include <map>                    // Contenedor asociativo para moda
#include <QtCharts>               // Módulo de gráficos
#include <QChartView>             // Vista de gráfico
#include <QBarSet>                // Conjunto de datos para barras
#include <QBarSeries>             // Serie de barras
#include <QBarCategoryAxis>       // Eje X con categorías
#include <QValueAxis>             // Eje Y numérico

// ======================= Lógica Estadística =======================
namespace Estadistica {
// Función para calcular la media (promedio) de un conjunto de datos
double media(const QVector<double>& datos) {
    if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos"); // Verifica si hay datos
    return std::accumulate(datos.begin(), datos.end(), 0.0) / datos.size(); // Suma y divide por el número de datos
}

// Función para calcular la mediana (valor central) de un conjunto de datos
double mediana(QVector<double> datos) {
    if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos"); // Verifica si hay datos
    std::sort(datos.begin(), datos.end()); // Ordena los datos
    int n = datos.size(); // Número de datos
    return (n % 2 == 0) ? (datos[n/2 - 1] + datos[n/2]) / 2.0 : datos[n/2]; // Calcula la mediana
}

// Función para calcular la varianza (dispersión de los datos)
double varianza(const QVector<double>& datos, bool muestral = true) {
    if (datos.size() < 2) throw std::invalid_argument("Mínimo 2 datos"); // Verifica si hay suficientes datos
    double mu = media(datos); // Calcula la media
    double suma = std::accumulate(datos.begin(), datos.end(), 0.0,
                                  [mu](double acum, double val) { return acum + std::pow(val - mu, 2); }); // Suma de cuadrados de diferencias
    return suma / (datos.size() - (muestral ? 1 : 0)); // Divide por n-1 (muestral) o n (poblacional)
}

// Función para calcular la desviación estándar (raíz cuadrada de la varianza)
double desviacionEstandar(const QVector<double>& datos, bool muestral = true) {
    return std::sqrt(varianza(datos, muestral)); // Raíz cuadrada de la varianza
}

// Función para calcular la moda (valor más frecuente)
QVector<double> moda(const QVector<double>& datos) {
    if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos"); // Verifica si hay datos

    std::map<double, int> frecuencias; // Mapa para contar frecuencias
    for (double val : datos) frecuencias[val]++; // Cuenta cuántas veces aparece cada valor

    int max_frec = std::max_element(frecuencias.begin(), frecuencias.end(),
                                    [](auto& a, auto& b) { return a.second < b.second; })->second; // Encuentra la frecuencia máxima

    QVector<double> modas; // Vector para almacenar las modas
    for (auto& [valor, cuenta] : frecuencias) {
        if (cuenta == max_frec) modas.append(valor); // Añade valores con la frecuencia máxima
    }

    return (max_frec > 1) ? modas : QVector<double>(); // Devuelve las modas o un vector vacío si no hay
}
}

// ======================= Interfaz Gráfica =======================
class MainWindow : public QMainWindow {
    Q_OBJECT // Macro necesaria para usar señales y slots

public:
    // Constructor de la ventana principal
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        configurarUI(); // Configura la interfaz de usuario
        conectarEventos(); // Conecta los eventos de los botones
    }

private slots:
    // Slot para cargar un archivo CSV
    void cargarCSV() {
        QString ruta = QFileDialog::getOpenFileName(this, "Abrir CSV", "", "CSV (*.csv)"); // Abre un diálogo para seleccionar archivo
        if (ruta.isEmpty()) return; // Si no se selecciona archivo, termina

        datos.clear(); // Limpia los datos anteriores
        QFile archivo(ruta); // Abre el archivo
        if (archivo.open(QIODevice::ReadOnly)) { // Verifica si se pudo abrir
            QTextStream entrada(&archivo); // Flujo de lectura
            while (!entrada.atEnd()) { // Lee línea por línea
                for (const QString& valor : entrada.readLine().split(',')) { // Divide por comas
                    bool ok;
                    double num = valor.trimmed().toDouble(&ok); // Convierte a número
                    if (ok) datos.append(num); // Si es válido, lo añade a los datos
                }
            }
            actualizarTabla(); // Actualiza la tabla con los nuevos datos
            actualizarGrafica(); // Actualiza el gráfico
        } else {
            QMessageBox::critical(this, "Error", "Error al abrir el archivo"); // Muestra un mensaje de error
        }
    }

    // Slots para calcular métricas estadísticas
    void calcularMedia() { calcularMetrica("Media", Estadistica::media); } // Calcula la media
    void calcularMediana() { calcularMetrica("Mediana", Estadistica::mediana); } // Calcula la mediana
    void calcularVarianza() { calcularMetrica("Varianza", [](auto d){ return Estadistica::varianza(d); }); } // Calcula la varianza
    void calcularDesviacion() { calcularMetrica("Desviación", [](auto d){ return Estadistica::desviacionEstandar(d); }); } // Calcula la desviación estándar

    // Slot para calcular la moda
    void calcularModa() {
        try {
            QVector<double> modas = Estadistica::moda(datos); // Calcula la moda
            if (modas.isEmpty()) {
                resultados->append("Moda: Sin moda (valores únicos)"); // Si no hay moda
            } else {
                QStringList valores;
                for (double m : modas) valores << QString::number(m, 'f', 3); // Formatea los valores
                resultados->append("Moda: " + valores.join(", ")); // Muestra las modas
            }
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what()); // Muestra un mensaje de error
        }
    }

private:
    // Función para configurar la interfaz de usuario
    void configurarUI() {
        setWindowTitle("Analizador Estadístico"); // Título de la ventana
        setMinimumSize(800, 600); // Tamaño mínimo de la ventana

        // Estilos CSS para la interfaz
        QString estilo = R"(
        QMainWindow { background-color: #2D2D2D; }
        QTableWidget {
            background-color: #1E1E1E;
            color: #FFFFFF;
            gridline-color: #3A3A3A;
            border: 1px solid #3A3A3A;
            border-radius: 4px;
            font-size: 12px;
        }
        QHeaderView::section {
            background-color: #333333;
            color: #00B4D8;
            padding: 6px;
            border: none;
        }
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #00B4D8, stop:1 #0096C7);
            color: white;
            border: none;
            border-radius: 5px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: bold;
            min-width: 80px;
        }
        QPushButton:hover { background-color: #00B4D8; }
        QPushButton:pressed { background-color: #0077B6; }
        QTextEdit {
            background-color: #1E1E1E;
            color: #FFFFFF;
            border: 1px solid #3A3A3A;
            border-radius: 4px;
            padding: 8px;
            font-family: 'Consolas';
        }
        QScrollBar:vertical { background: #1E1E1E; width: 12px; }
        QScrollBar::handle:vertical { background: #3A3A3A; min-height: 20px; border-radius: 6px; }
        )";
        this->setStyleSheet(estilo); // Aplica los estilos

        // Configuración de la interfaz
        QWidget *widgetCentral = new QWidget(this); // Widget central
        QVBoxLayout *layoutPrincipal = new QVBoxLayout(widgetCentral); // Layout vertical

        // Tabla para mostrar datos
        tabla = new QTableWidget(this);
        tabla->setColumnCount(1); // Una columna para los valores
        tabla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Ajusta el ancho de la columna
        tabla->setHorizontalHeaderLabels({"Valores"}); // Etiqueta de la columna
        tabla->setStyleSheet("alternate-background-color: #262626; selection-background-color: #00B4D8;"); // Estilos de la tabla

        // Vista del gráfico
        chartView = new QChartView();
        chartView->setRenderHint(QPainter::Antialiasing); // Suavizado de bordes
        chartView->setStyleSheet("background: transparent; border: none;"); // Estilos del gráfico

        // Botones para operaciones
        QHBoxLayout *layoutBotones = new QHBoxLayout();
        QPushButton *btnCargar = new QPushButton("Cargar CSV", this);
        QPushButton *btnMedia = new QPushButton("Media", this);
        QPushButton *btnMediana = new QPushButton("Mediana", this);
        QPushButton *btnModa = new QPushButton("Moda", this);
        QPushButton *btnVarianza = new QPushButton("Varianza", this);
        QPushButton *btnDesviacion = new QPushButton("Desviación", this);

        // Añadir botones al layout
        QList<QPushButton*> botones = {btnCargar, btnMedia, btnMediana,btnModa ,btnVarianza, btnDesviacion};
        for (QPushButton* btn : botones) {
            btn->setObjectName(btn->text().replace(" ", "")); // Asigna nombres únicos
            layoutBotones->addWidget(btn); // Añade al layout
        }

        // Área de texto para resultados
        resultados = new QTextEdit(this);
        resultados->setReadOnly(true); // Solo lectura

        // Ensamblar la interfaz
        layoutPrincipal->addWidget(tabla); // Añade la tabla
        layoutPrincipal->addWidget(chartView); // Añade el gráfico
        layoutPrincipal->addLayout(layoutBotones); // Añade los botones
        layoutPrincipal->addWidget(resultados); // Añade el área de resultados

        setCentralWidget(widgetCentral); // Establece el widget central
    }

    // Función para conectar eventos de los botones
    void conectarEventos() {
        connect(findChild<QPushButton*>("CargarCSV"), &QPushButton::clicked, this, &MainWindow::cargarCSV);
        connect(findChild<QPushButton*>("Media"), &QPushButton::clicked, this, &MainWindow::calcularMedia);
        connect(findChild<QPushButton*>("Mediana"), &QPushButton::clicked, this, &MainWindow::calcularMediana);
        connect(findChild<QPushButton*>("Moda"), &QPushButton::clicked, this, &MainWindow::calcularModa);
        connect(findChild<QPushButton*>("Varianza"), &QPushButton::clicked, this, &MainWindow::calcularVarianza);
        connect(findChild<QPushButton*>("Desviación"), &QPushButton::clicked, this, &MainWindow::calcularDesviacion);
    }

    // Función para actualizar la tabla con los datos
    void actualizarTabla() {
        tabla->setRowCount(datos.size()); // Establece el número de filas
        for (int i = 0; i < datos.size(); ++i) {
            tabla->setItem(i, 0, new QTableWidgetItem(QString::number(datos[i], 'f', 3))); // Añade los datos a la tabla
        }
    }

    // Plantilla para calcular métricas
    template<typename Func>
    void calcularMetrica(const QString& nombre, Func funcion) {
        try {
            double resultado = funcion(datos); // Calcula la métrica
            resultados->append(QString("%1: %2").arg(nombre).arg(resultado, 0, 'f', 3)); // Muestra el resultado
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what()); // Muestra un mensaje de error
        }
    }

    // Función para actualizar el gráfico
    void actualizarGrafica() {
        QChart *chart = new QChart(); // Crea un nuevo gráfico
        chart->setTitle("Distribución de datos"); // Título del gráfico
        chart->setTheme(QChart::ChartThemeDark); // Tema oscuro
        chart->setBackgroundBrush(QBrush(QColor("#2D2D2D"))); // Fondo del gráfico
        chart->setTitleBrush(QBrush(Qt::white)); // Color del título
        chart->legend()->setVisible(false); // Oculta la leyenda

        if (!datos.isEmpty()) { // Si hay datos
            double min = *std::min_element(datos.begin(), datos.end()); // Valor mínimo
            double max = *std::max_element(datos.begin(), datos.end()); // Valor máximo
            double binWidth = 0.0; // Ancho de cada intervalo
            const int bins = 10; // Número de intervalos

            if (min == max) { // Si todos los datos son iguales
                min -= 1.0; // Ajusta el mínimo
                max += 1.0; // Ajusta el máximo
                binWidth = 0.2; // Ancho fijo
            } else {
                binWidth = (max - min) / bins; // Calcula el ancho de los intervalos
            }

            QBarSet *barSet = new QBarSet("Frecuencia"); // Conjunto de barras
            QVector<int> frecuencias(bins, 0); // Vector para contar frecuencias

            for (double valor : datos) { // Recorre los datos
                int indice = qBound(0, static_cast<int>((valor - min) / binWidth), bins - 1); // Calcula el índice del intervalo
                frecuencias[indice]++; // Incrementa la frecuencia
            }

            for (int f : frecuencias) *barSet << f; // Añade las frecuencias al conjunto de barras
            barSet->setColor(QColor("#00B4D8")); // Color de las barras

            QBarSeries *series = new QBarSeries(); // Serie de barras
            series->append(barSet); // Añade el conjunto de barras

            QStringList categorias; // Etiquetas para el eje X
            for (int i = 0; i < bins; ++i) {
                categorias << QString::number(min + i * binWidth, 'f', 2); // Añade las etiquetas
            }

            QBarCategoryAxis *axisX = new QBarCategoryAxis(); // Eje X
            axisX->append(categorias); // Añade las categorías
            axisX->setLabelsBrush(QBrush(Qt::white)); // Color de las etiquetas

            QValueAxis *axisY = new QValueAxis(); // Eje Y
            axisY->setLabelFormat("%d"); // Formato de las etiquetas
            axisY->setLabelsBrush(QBrush(Qt::white)); // Color de las etiquetas

            chart->addSeries(series); // Añade la serie al gráfico
            chart->addAxis(axisX, Qt::AlignBottom); // Añade el eje X
            chart->addAxis(axisY, Qt::AlignLeft); // Añade el eje Y

            series->attachAxis(axisX); // Conecta la serie al eje X
            series->attachAxis(axisY); // Conecta la serie al eje Y
        }

        chartView->setChart(chart); // Establece el gráfico en la vista
    }

    // Miembros de la clase
    QChartView *chartView; // Vista del gráfico
    QTableWidget *tabla; // Tabla de datos
    QTextEdit *resultados; // Área de texto para resultados
    QVector<double> datos; // Vector para almacenar los datos
};

// ======================= Punto de Entrada =======================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // Inicializa la aplicación Qt
    MainWindow ventana; // Crea la ventana principal
    ventana.show(); // Muestra la ventana
    return app.exec(); // Inicia el bucle de eventos
}

#include "main.moc" // Inclusión de meta-objetos para señales y slots
