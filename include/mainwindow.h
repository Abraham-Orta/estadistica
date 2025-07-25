#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QChartView;
class QTableWidget;
class QTextEdit;
class QComboBox;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void cargarCSV();
    void calcularMedia();
    void calcularMediana();
    void calcularModa();
    void calcularVarianza();
    void calcularDesviacion();
    void analizarBivariado();

private:
    void configurarUI();
    void conectarEventos();
    void actualizarTabla();
    void actualizarUIAfterCargar();
    template<typename Func>
    void calcularMetrica(const QString& nombre, Func funcion);
    void actualizarGrafica();
    void mostrarHistograma(const QVector<double>& datosVar);
    void mostrarScatterPlot(const QVector<double>& x, const QVector<double>& y);

    Ui::MainWindow *ui;
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
};
#endif // MAINWINDOW_H
