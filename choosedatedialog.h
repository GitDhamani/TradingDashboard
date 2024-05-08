#ifndef CHOOSEDATEDIALOG_H
#define CHOOSEDATEDIALOG_H

#include <QDialog>

namespace Ui {
class ChooseDateDialog;
}

class ChooseDateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseDateDialog(QWidget *parent = nullptr);
    ~ChooseDateDialog();

private:
    Ui::ChooseDateDialog *ui;
};

#endif // CHOOSEDATEDIALOG_H
