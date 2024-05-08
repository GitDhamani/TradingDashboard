#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "currency.h"
extern MainWindow* this_ptr;
extern Ui::MainWindow* ui_ptr;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    //Set up Initial Values
    ui->timestepBox->setCurrentText(QString::number(this_ptr->timestepInterval));
    ui->feeEdit->setText(QString::number(this_ptr->brokerFee));
    ui->stopEdit->setText(QString::number(this_ptr->defaultStopPips));
    ui->targetEdit->setText(QString::number(this_ptr->defaultTargetPips));

    //Standard Lot
    if (this_ptr->lotSize == 100000)
    ui->lotsizeBox->setCurrentText("Standard");

    //Mini Lot
    if (this_ptr->lotSize == 10000)
    ui->lotsizeBox->setCurrentText("Mini");

    //Micro Lot
    if (this_ptr->lotSize == 1000)
    ui->lotsizeBox->setCurrentText("Micro");

    //Connect Statements
    connect(ui->pushButton, &QPushButton::clicked, [this](){
        this_ptr->timestepInterval = ui->timestepBox->currentText().toInt();
        this_ptr->brokerFee = ui->feeEdit->text().toInt();
        this_ptr->defaultStopPips = ui->stopEdit->text().toInt();
        this_ptr->defaultTargetPips = ui->targetEdit->text().toInt();

        //Standard Lot
        if (ui->lotsizeBox->currentIndex() == 0)
        this_ptr->lotSize = 100000;

        //Mini Lot
        if (ui->lotsizeBox->currentIndex() == 1)
        this_ptr->lotSize = 10000;

        //Micro Lot
        if (ui->lotsizeBox->currentIndex() == 2)
        this_ptr->lotSize = 1000;

        //Emit Signal to the Currency Objects to refresh their Price Per Pip values
        emit this_ptr->updatePricePerPip();

        this->close();
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
