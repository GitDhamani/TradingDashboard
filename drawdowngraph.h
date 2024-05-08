#ifndef DRAWDOWNGRAPH_H
#define DRAWDOWNGRAPH_H

#include <QDialog>
#include <QtCharts/QChart>
using namespace QtCharts;

namespace Ui {
class DrawDownGraph;
}

class DrawDownGraph : public QDialog
{
    Q_OBJECT

public:
    explicit DrawDownGraph(QWidget *parent = nullptr);
    ~DrawDownGraph();


private:
    Ui::DrawDownGraph *ui;
};

#endif // DRAWDOWNGRAPH_H
