#include "drawdowngraph.h"
#include "ui_drawdowngraph.h"
#include "mainwindow.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QPointF>
using namespace QtCharts;
extern MainWindow* this_ptr;

QChart* chart;
QChartView* view;

DrawDownGraph::DrawDownGraph(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DrawDownGraph)
{
    ui->setupUi(this);
    setFixedSize(750,450);
    chart = new QChart();
    chart->addSeries(this_ptr->drawDownSeries);
    chart->setVisible(true);
    chart->legend()->hide();  //Hides the little coloured Square
    chart->createDefaultAxes();
    chart->axisX()->setTitleText("Trade Count");
    chart->axisY()->setTitleText("Account Balance");

    //Determine Max and Min Values of the Series

    // Initialize max and min with the first data point
    QPointF maxPoint = this_ptr->drawDownSeries->at(0);
    QPointF minPoint = this_ptr->drawDownSeries->at(0);

    // Iterate over each point in the series to find the maximum and minimum
    for (int i = 1; i < this_ptr->drawDownSeries->count(); ++i) {
        QPointF point = this_ptr->drawDownSeries->at(i);
        if (point.y() > maxPoint.y())
            maxPoint = point;

        if (point.y() < minPoint.y())
            minPoint = point;
    }

    chart->axes(Qt::Vertical).first()->setRange(minPoint.y(),maxPoint.y());
    chart->axes(Qt::Horizontal).first()->setRange(0, this_ptr->drawDownSeries->count());
    chart->setTitle("Equity Curve");

    //Create a View and Initialise it with the Chart Data
    view = new QChartView(chart, this);
    view->setVisible(true);
    view->setRenderHint(QPainter::Antialiasing);

    //Add the View to our Vertical Layout Container.
    ui->verticalLayout->addWidget(view);
}

DrawDownGraph::~DrawDownGraph()
{
    delete ui;
    //chart->deleteLater();
    delete view;
    delete chart;
}
