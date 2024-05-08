#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include "drawdowngraph.h"
using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QVector<QVector<QString>> tradeVector;
    QStringList headers;
    void placeNewTrade();
    void initialiseTable();
    void addNewTrade();
    void clearTrades();
    int brokerFee = 10;
    int timestepInterval = 1000;
    int defaultStopPips = 10;
    int defaultTargetPips = 30;
    int lotSize = 100000;
    QString jumpDate, jumpTime;
    QLineSeries* drawDownSeries = new QLineSeries(this);

signals:
    void mySignal();
    void updatePricePerPip();

private slots:
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
