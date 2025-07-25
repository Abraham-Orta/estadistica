#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <map>
#include <QtCharts>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QScatterSeries>
#include <QComboBox>
#include <QLabel>

namespace Estadistica {
    // Análisis Univariado
    double media(const QVector<double>& datos) {
        if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos");
        return std::accumulate(datos.begin(), datos.end(), 0.0) / datos.size();
    }

    double mediana(QVector<double> datos) {
        if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos");
        std::sort(datos.begin(), datos.end());
        int n = datos.size();
        return (n % 2 == 0) ? (datos[n/2 - 1] + datos[n/2]) / 2.0 : datos[n/2];
    }

    double varianza(const QVector<double>& datos, bool muestral = true) {
        if (datos.size() < 2) throw std::invalid_argument("Mínimo 2 datos");
        double mu = media(datos);
        double suma = std::accumulate(datos.begin(), datos.end(), 0.0,
                                    [mu](double acum, double val) { return acum + std::pow(val - mu, 2); });
        return suma / (datos.size() - (muestral ? 1 : 0));
    }

    double desviacionEstandar(const QVector<double>& datos, bool muestral = true) {
        return std::sqrt(varianza(datos, muestral));
    }

    QVector<double> moda(const QVector<double>& datos) {
        if (datos.isEmpty()) throw std::invalid_argument("Datos vacíos");
        std::map<double, int> frecuencias;
        for (double val : datos) frecuencias[val]++;
        int max_frec = std::max_element(frecuencias.begin(), frecuencias.end(),
                                    [](auto& a, auto& b) { return a.second < b.second; })->second;
        QVector<double> modas;
        for (auto& [valor, cuenta] : frecuencias) {
            if (cuenta == max_frec) modas.append(valor);
        }
        return (max_frec > 1) ? modas : QVector<double>();
    }

    // Análisis Bivariado
    double covarianza(const QVector<double>& x, const QVector<double>& y) {
        if (x.size() != y.size() || x.size() < 2) throw std::invalid_argument("Datos incompatibles");
        double mediaX = media(x), mediaY = media(y);
        double suma = 0.0;
        for (int i = 0; i < x.size(); ++i) {
            suma += (x[i] - mediaX) * (y[i] - mediaY);
        }
        return suma / (x.size() - 1);
    }

    double correlacionPearson(const QVector<double>& x, const QVector<double>& y) {
        double cov = covarianza(x, y);
        double desvX = desviacionEstandar(x);
        double desvY = desviacionEstandar(y);
        if (desvX == 0 || desvY == 0) throw std::invalid_argument("Desviación estándar cero");
        return cov / (desvX * desvY);
    }
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        configurarUI();
        conectarEventos();
    }

private slots:
    void cargarCSV() {
        QString ruta = QFileDialog::getOpenFileName(this, "Abrir CSV", "", "CSV (*.csv)");
        if (ruta.isEmpty()) return;

        datos.clear();
        QFile archivo(ruta);
        if (archivo.open(QIODevice::ReadOnly)) {
            QTextStream entrada(&archivo);
            bool primeraLinea = true;
            int numVariables = 0;

            while (!entrada.atEnd()) {
                QString linea = entrada.readLine().trimmed();
                if (linea.isEmpty()) continue;

                QStringList valores = linea.split(',');
                if (primeraLinea) {
                    numVariables = valores.size();
                    datos.resize(numVariables);
                    primeraLinea = false;
                }

                if (valores.size() != numVariables) {
                    QMessageBox::warning(this, "Error", "Formato de CSV inválido.");
                    datos.clear();
                    return;
                }

                for (int i = 0; i < numVariables; ++i) {
                    bool ok;
                    double num = valores[i].trimmed().toDouble(&ok);
                    if (ok) datos[i].append(num);
                }
            }

            actualizarTabla();
            actualizarUIAfterCargar();
            actualizarGrafica();
        } else {
            QMessageBox::critical(this, "Error", "Error al abrir el archivo");
        }
    }

    void calcularMedia() { calcularMetrica("Media", Estadistica::media); }
    void calcularMediana() { calcularMetrica("Mediana", Estadistica::mediana); }
    void calcularVarianza() { calcularMetrica("Varianza", [](auto d){ return Estadistica::varianza(d); }); }
    void calcularDesviacion() { calcularMetrica("Desviación", [](auto d){ return Estadistica::desviacionEstandar(d); }); }

    void calcularModa() {
        try {
            int idx = cmbVariableUni->currentIndex();
            if (idx < 0 || idx >= datos.size()) return;

            QVector<double> modas = Estadistica::moda(datos[idx]);
            if (modas.isEmpty()) {
                resultados->append("Moda: Sin moda (valores únicos)");
            } else {
                QStringList valores;
                for (double m : modas) valores << QString::number(m, 'f', 3);
                resultados->append(QString("Moda (Var %1): %2").arg(idx+1).arg(valores.join(", ")));
            }
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    }

    void analizarBivariado() {
        int xIdx = cmbVariableX->currentIndex();
        int yIdx = cmbVariableY->currentIndex();

        if (xIdx < 0 || yIdx < 0 || xIdx >= datos.size() || yIdx >= datos.size()) {
            QMessageBox::warning(this, "Error", "Seleccione variables válidas");
            return;
        }

        try {
            double cov = Estadistica::covarianza(datos[xIdx], datos[yIdx]);
            double corr = Estadistica::correlacionPearson(datos[xIdx], datos[yIdx]);
            resultados->append(QString("Covarianza (Var%1-Var%2): %3\nCorrelación: %4")
                                   .arg(xIdx+1).arg(yIdx+1)
                                   .arg(cov, 0, 'f', 3)
                                   .arg(corr, 0, 'f', 3));
            mostrarScatterPlot(datos[xIdx], datos[yIdx]);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    }

private:
    QVector<QVector<double>> datos;
    QChartView *chartView;
    QTableWidget *tabla;
    QTextEdit *resultados;
    QComboBox *cmbVariableUni;
    QComboBox *cmbVariableX;
    QComboBox *cmbVariableY;

    QPushButton *btnCargar;
    QPushButton *btnMedia;
    QPushButton *btnMediana;
    QPushButton *btnModa;
    QPushButton *btnVarianza;
    QPushButton *btnDesviacion;
    QPushButton *btnBivariado;

    void configurarUI() {
        setWindowTitle("Analizador Estadístico");
        setMinimumSize(1000, 800);

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
        setStyleSheet(estilo);

        QWidget *widgetCentral = new QWidget(this);
        QVBoxLayout *layoutPrincipal = new QVBoxLayout(widgetCentral);

        // Componentes
        tabla = new QTableWidget(this);
        chartView = new QChartView();
        chartView->setRenderHint(QPainter::Antialiasing);
        resultados = new QTextEdit(this);
        resultados->setReadOnly(true);

        // Botones
        btnCargar = new QPushButton("Cargar CSV", this);
        btnMedia = new QPushButton("Media", this);
        btnMediana = new QPushButton("Mediana", this);
        btnModa = new QPushButton("Moda", this);
        btnVarianza = new QPushButton("Varianza", this);
        btnDesviacion = new QPushButton("Desviación", this);
        btnBivariado = new QPushButton("Analizar Bivariado", this);

        // Comboboxes
        cmbVariableUni = new QComboBox(this);
        cmbVariableX = new QComboBox(this);
        cmbVariableY = new QComboBox(this);

        // Layouts
        QHBoxLayout *layoutControles = new QHBoxLayout();
        layoutControles->addWidget(btnCargar);
        layoutControles->addWidget(new QLabel("Variable:", this));
        layoutControles->addWidget(cmbVariableUni);

        QHBoxLayout *layoutBotonesUni = new QHBoxLayout();
        layoutBotonesUni->addWidget(btnMedia);
        layoutBotonesUni->addWidget(btnMediana);
        layoutBotonesUni->addWidget(btnModa);
        layoutBotonesUni->addWidget(btnVarianza);
        layoutBotonesUni->addWidget(btnDesviacion);

        QHBoxLayout *layoutBivariado = new QHBoxLayout();
        layoutBivariado->addWidget(new QLabel("X:", this));
        layoutBivariado->addWidget(cmbVariableX);
        layoutBivariado->addWidget(new QLabel("Y:", this));
        layoutBivariado->addWidget(cmbVariableY);
        layoutBivariado->addWidget(btnBivariado);

        // Ensamblado
        layoutPrincipal->addLayout(layoutControles);
        layoutPrincipal->addLayout(layoutBotonesUni);
        layoutPrincipal->addLayout(layoutBivariado);
        layoutPrincipal->addWidget(tabla);
        layoutPrincipal->addWidget(chartView);
        layoutPrincipal->addWidget(resultados);

        setCentralWidget(widgetCentral);
    }

    void conectarEventos() {
        connect(btnCargar, &QPushButton::clicked, this, &MainWindow::cargarCSV);
        connect(btnMedia, &QPushButton::clicked, this, &MainWindow::calcularMedia);
        connect(btnMediana, &QPushButton::clicked, this, &MainWindow::calcularMediana);
        connect(btnModa, &QPushButton::clicked, this, &MainWindow::calcularModa);
        connect(btnVarianza, &QPushButton::clicked, this, &MainWindow::calcularVarianza);
        connect(btnDesviacion, &QPushButton::clicked, this, &MainWindow::calcularDesviacion);
        connect(btnBivariado, &QPushButton::clicked, this, &MainWindow::analizarBivariado);
    }

    void actualizarTabla() {
        if (datos.isEmpty()) {
            tabla->setRowCount(0);
            tabla->setColumnCount(0);
            return;
        }

        int filas = datos[0].size();
        int columnas = datos.size();
        tabla->setRowCount(filas);
        tabla->setColumnCount(columnas);

        QStringList headers;
        for (int i = 0; i < columnas; ++i) {
            headers << QString("Var %1").arg(i+1);
            for (int j = 0; j < filas; ++j) {
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(datos[i][j], 'f', 3));
                tabla->setItem(j, i, item);
            }
        }
        tabla->setHorizontalHeaderLabels(headers);
    }

    void actualizarUIAfterCargar() {
        cmbVariableUni->clear();
        cmbVariableX->clear();
        cmbVariableY->clear();
        for (int i = 0; i < datos.size(); ++i) {
            QString nombre = QString("Variable %1").arg(i + 1);
            cmbVariableUni->addItem(nombre);
            cmbVariableX->addItem(nombre);
            cmbVariableY->addItem(nombre);
        }
    }

    template<typename Func>
    void calcularMetrica(const QString& nombre, Func funcion) {
        try {
            int idx = cmbVariableUni->currentIndex();
            if (idx < 0 || idx >= datos.size()) return;

            double resultado = funcion(datos[idx]);
            resultados->append(QString("%1 (Var %2): %3")
                                   .arg(nombre).arg(idx+1)
                                   .arg(resultado, 0, 'f', 3));
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    }

    void actualizarGrafica() {
        if (datos.isEmpty()) return;
        mostrarHistograma(datos[0]);
    }

    void mostrarHistograma(const QVector<double>& datosVar) {
        QChart *chart = new QChart();
        chart->setTitle("Histograma");
        chart->setTheme(QChart::ChartThemeDark);

        if (datosVar.isEmpty()) return;

        double min = *std::min_element(datosVar.begin(), datosVar.end());
        double max = *std::max_element(datosVar.begin(), datosVar.end());

        if (min == max) {
            min -= 1.0;
            max += 1.0;
        }

        const int bins = 10;
        double binWidth = (max - min) / bins;

        QBarSet *barSet = new QBarSet("Frecuencia");
        QVector<int> frecuencias(bins, 0);

        for (double valor : datosVar) {
            int indice = qBound(0, static_cast<int>((valor - min) / binWidth), bins - 1);
            frecuencias[indice]++;
        }

        for (int f : frecuencias) *barSet << f;
        barSet->setColor(QColor("#00B4D8"));

        QBarSeries *series = new QBarSeries();
        series->append(barSet);

        QStringList categorias;
        for (int i = 0; i < bins; ++i) {
            categorias << QString::number(min + i * binWidth, 'f', 2);
        }

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categorias);
        axisX->setLabelsBrush(QBrush(Qt::white));

        QValueAxis *axisY = new QValueAxis();
        axisY->setLabelFormat("%d");
        axisY->setLabelsBrush(QBrush(Qt::white));

        chart->addSeries(series);
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        chartView->setChart(chart);
    }

    void mostrarScatterPlot(const QVector<double>& x, const QVector<double>& y) {
        QScatterSeries *series = new QScatterSeries();
        series->setMarkerSize(10.0);
        series->setColor(QColor("#00B4D8"));

        for (int i = 0; i < x.size(); ++i) {
            series->append(x[i], y[i]);
        }

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->createDefaultAxes();
        chart->setTitle("Scatter Plot");
        chart->setTheme(QChart::ChartThemeDark);
        chart->axes(Qt::Horizontal).first()->setLabelsBrush(QBrush(Qt::white));
        chart->axes(Qt::Vertical).first()->setLabelsBrush(QBrush(Qt::white));

        chartView->setChart(chart);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow ventana;
    ventana.show();
    return app.exec();
}

#include "main.moc"
