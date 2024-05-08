#ifndef CURRENCY_H
#define CURRENCY_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QObject>

class Currency: public QObject
{
    Q_OBJECT

public:
    Currency(QString Name);
    ~Currency();
    QString currencyName;
    QString currencyFilePath;
    double currencyValue = 0;
    void updateLabel();
    QFile* fileHandle;
    QTextStream in2;
    void setWhiteBackground();
    double pipValue = 0.0001;
    double pricePerPip;
    QString tradeType;
    double orderPrice, stopPrice, targetPrice;
    int orderRow;
    int pipMultiplier = 10000;
    void updateTradeTable();
    void calcPricePerPip();
    void updateRowProfit();
    void markTradeResult(QString);
    void resetLabels();
    void newLabelDate(QString date);

public slots:
    void handleSignal();
    void updatePricePerPip();
};

#endif // CURRENCY_H
