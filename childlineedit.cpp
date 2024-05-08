#include "childlineedit.h"
#include "./ui_mainwindow.h"
#include <QKeyEvent>

extern Ui::MainWindow* ui_ptr;
extern void processLoginPassword();

childLineEdit::childLineEdit(QWidget* parent) : QLineEdit(parent)
{ }

void childLineEdit::keyPressEvent(QKeyEvent *event)
{
    ui_ptr->errorLabel->setHidden(true);
    if (event->key() == (Qt::Key_Return))
    {
        processLoginPassword();
    }
    QLineEdit::keyPressEvent(event);
}
