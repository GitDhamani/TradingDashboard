#include "newtradedialog.h"
#include "ui_newtradedialog.h"
#include "./ui_mainwindow.h"
#include "mainwindow.h"

#include <QMessageBox>
#include <QDebug>

extern Ui::MainWindow* ui_ptr;
extern MainWindow* this_ptr;
extern void updateBalance();
extern double depositSum;
extern bool closeProperly;

NewTradeDialog::NewTradeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewTradeDialog)
{
    ui->setupUi(this);
    closeProperly = 0;

    QList<QString> currencyPairs = {"AUDCAD", "AUDJPY", "AUDNZD", "AUDUSD", "CADCHF", "CADJPY", "EURAUD", "EURCAD", "EURCHF", "EURGBP", "EURJPY", "EURNZD", "EURUSD", "GBPAUD", "GBPCAD", "GBPCHF", "GBPJPY", "GBPUSD", "NZDCAD", "NZDJPY", "NZDUSD", "USDCAD", "USDCHF", "USDJPY"};
    ui->pairCombo->addItems(currencyPairs);
    ui->tradeCombo->setFocus();

    //Load the Price Label with the currently selected price of the default Pair
    loadPrice();

    //Load the Account Balance Label
    ui->balanceLabel->setText(ui_ptr->balanceLabel->text());

    //Connect for the FX Pair Combo Box
    connect(ui->pairCombo, &QComboBox::currentTextChanged, [this](){ loadPrice(); });

    //Connect for the Trade Type Combo Box
    connect(ui->tradeCombo, &QComboBox::currentTextChanged, [this](){ loadPrice(); });

    //Connect for the Push Button
    connect(ui->tradeButton, &QPushButton::clicked, [this](){

        QString errorMsg;

        //Validate input
        if (ui->tradeCombo->currentText() == "Long")
        {
            //If stop is above Price, error
            if (ui->stopEdit->text().toDouble() > ui->priceLabel->text().toDouble())
                errorMsg = "Stop Price should not be Above than Current Price.";

            //If target is below Price, error
            if (ui->targetEdit->text().toDouble() < ui->priceLabel->text().toDouble())
                errorMsg = "Target Price should not be Lower than Current Price.";
        }

        //Validate input
        if (ui->tradeCombo->currentText() == "Short")
        {
            //If stop is below Price, error
            if (ui->stopEdit->text().toDouble() < ui->priceLabel->text().toDouble())
                errorMsg = "Stop Price should not be Lower than Current Price.";

            //If target is above Price, error
            if (ui->targetEdit->text().toDouble() > ui->priceLabel->text().toDouble())
                errorMsg = "Target Price should not be Above than Current Price.";
        }

        if (errorMsg != "")
        {
            QMessageBox::warning(this, "Warning", errorMsg);
            loadPrice();
        }

        if (ui->balanceLabel->text().toDouble() < this_ptr->brokerFee*2)
        {
            QMessageBox::warning(this, "Warning", "Insufficient Funds in Account.");
            return;
        }

        int maxItems = ui->pairCombo->count();
        if (ui->pairCombo->currentIndex() <= maxItems-1)
        placeTrade();

    });

    connect(ui->closeButton, &QPushButton::clicked, [this](){
        closeProperly = 1;
        this->close();
    });

    connect(ui->skipButton, &QPushButton::clicked, [this](){
        //Nudge the Currency Pair Box to the next Pair
        int maxItems = ui->pairCombo->count();
        if (ui->pairCombo->currentIndex() <= maxItems-2)
        ui->pairCombo->setCurrentIndex(ui->pairCombo->currentIndex() + 1);
    });
}

NewTradeDialog::~NewTradeDialog()
{
    delete ui;
}

void NewTradeDialog::loadPrice()
{
    //Load Initial Price of the Selected default Pair Combo
    QString currentPair = ui->pairCombo->currentText();

    //Find all your label pointers
    // Prepare the regular expression pattern to match label names
    QString pattern = QString("label%1\\d+").arg(currentPair);

    // Compile the regular expression
    QRegularExpression regex(pattern);

    // Find children labels that match the pattern
    QList<QLabel*> labels = this_ptr->findChildren<QLabel*>(regex);
    QString currentPrice = labels[1]->text() + labels[2]->text();
    ui->priceLabel->setText(currentPrice);

    //Typical Trade 10 pip stop, and 30 pip target.
    double currentPriceDouble = currentPrice.toDouble();
    double stopPrice, targetPrice;
    QString tradeType = ui->tradeCombo->currentText();

    if (tradeType == "Long")
    {
        if (!currentPair.contains("JPY"))
        {
            stopPrice = currentPriceDouble - (this_ptr->defaultStopPips)/10000.0;
            targetPrice = currentPriceDouble + (this_ptr->defaultTargetPips)/10000.0;
        }

        if (currentPair.contains("JPY"))
        {
            stopPrice = currentPriceDouble - this_ptr->defaultStopPips/100.0;
            targetPrice = currentPriceDouble + this_ptr->defaultTargetPips/100.0;
        }
    }

    if (tradeType == "Short")
    {
        if (!currentPair.contains("JPY"))
        {
            stopPrice = currentPriceDouble + this_ptr->defaultStopPips/10000.0;
            targetPrice = currentPriceDouble - this_ptr->defaultTargetPips/10000.0;
        }

        if (currentPair.contains("JPY"))
        {
            stopPrice = currentPriceDouble + this_ptr->defaultStopPips/100.0;
            targetPrice = currentPriceDouble - this_ptr->defaultTargetPips/100.0;
        }
    }

    ui->stopEdit->setText(QString::number(stopPrice));
    ui->targetEdit->setText(QString::number(targetPrice));
}

void NewTradeDialog::placeTrade()
{
    this_ptr->tradeVector.push_back({ui->tradeCombo->currentText(), ui->pairCombo->currentText(), ui->priceLabel->text(), ui->stopEdit->text(), ui->targetEdit->text(), QString::number(this_ptr->brokerFee * -2)});

    //Nudge the Currency Pair Box to the next Pair
    int maxItems = ui->pairCombo->count();
    if (ui->pairCombo->currentIndex() <= maxItems-2)
    ui->pairCombo->setCurrentIndex(ui->pairCombo->currentIndex() + 1);

    //Deduct Brokers Fee from Dialog Account Balance
    ui->balanceLabel->setText(QString::number(ui->balanceLabel->text().toDouble() - this_ptr->brokerFee*2));

    updateBalance();
}
