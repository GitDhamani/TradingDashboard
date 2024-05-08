#ifndef NEWTRADEDIALOG_H
#define NEWTRADEDIALOG_H

#include <QDialog>

namespace Ui {
class NewTradeDialog;
}

class NewTradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTradeDialog(QWidget *parent = nullptr);
    ~NewTradeDialog();
    void loadPrice();
    void placeTrade();

private:
    Ui::NewTradeDialog *ui;
};

#endif // NEWTRADEDIALOG_H
