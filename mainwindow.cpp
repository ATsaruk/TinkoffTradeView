#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Core/globals.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Инициализируем singleton
    Glo.init();

    plotter = new Plotter::ChartPlotter;
    setCentralWidget(plotter);

    plotter->setDrawStockKey(getStockKey());
}

MainWindow::~MainWindow()
{
    delete ui;
}

const Data::StockKey MainWindow::getStockKey()
{
    Data::StockKey key("BBG000B9XRY4", Data::StockKey::INTERVAL::MIN15);    //temp
    return key;
}

void MainWindow::on_actUpdate_triggered()
{
    plotter->setDrawStockKey(getStockKey());
}

void MainWindow::on_actExit_triggered()
{
    close();
}

/*void MainWindow::on_pbFigiList_clicked()
{
    //taskManager->broker->getStocks();
}

void MainWindow::on_pbApple_clicked()
{
    NEW_TASK<TaskLoadStocksFromBroker, StockKey>(getStockKey());
}

void MainWindow::on_pbLoadAppleDb_clicked()
{
    NEW_TASK<TaskLoadStockFromDb, StockKey>(getStockKey());
}

void MainWindow::on_pbRepaintAll_clicked()
{
    plotter->setDrawStockKey(getStockKey());
}

void MainWindow::on_pbCommandGetStock_clicked()
{
    NEW_TASK<CommandLoadStock, StockKey>(getStockKey());
}*/

