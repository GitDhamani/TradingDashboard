#include "choosedatedialog.h"
#include "ui_choosedatedialog.h"
#include <QDebug>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

extern MainWindow* this_ptr;
extern Ui::MainWindow* ui_ptr;

ChooseDateDialog::ChooseDateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseDateDialog)
{
    ui->setupUi(this);
    ui->errorLabel->setVisible(false);

    //Set minimum date on calendar, so cannot trade in the relative past.
    QDate minimumDate = QDate::fromString(ui_ptr->dateLabel->text(), "dd.MM.yyyy");

    // Add one day to the minimum date
    minimumDate = minimumDate.addDays(1);
    ui->calendarWidget->setMinimumDate(minimumDate);

    connect(ui->pushButton, &QPushButton::clicked, [this](){
        QDate date = ui->calendarWidget->selectedDate();
        QString formattedDate = date.toString("yyyy.MM.dd");
        //qDebug() << formattedDate;

        //Check if it's a weekend
        bool error = 0;
        if (date.toString().contains("Sat")) error = 1;
        if (date.toString().contains("Sun")) error = 1;
        if (error)
        {
            ui->errorLabel->setVisible(true);
            return;
        }

        this_ptr->jumpDate = formattedDate;
        this_ptr->jumpTime = ui->timeEdit->text();
        this->close();
    });
}

ChooseDateDialog::~ChooseDateDialog()
{
    delete ui;
}
