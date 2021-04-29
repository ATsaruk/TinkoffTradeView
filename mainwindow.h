#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Data/Stock/stockkey.h"
#include "Plotter/chartplotter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    const Data::StockKey getStockKey();

private:
    Ui::MainWindow *ui;

    Plotter::ChartPlotter *plotter;

private slots:
    void on_actUpdate_triggered();
    void on_actExit_triggered();
};
#endif // MAINWINDOW_H
