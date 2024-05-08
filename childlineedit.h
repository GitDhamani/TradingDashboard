#ifndef CHILDLINEEDIT_H
#define CHILDLINEEDIT_H

#include <QLineEdit>
#include <QWidget>

class childLineEdit : public QLineEdit
{
public:
    childLineEdit(QWidget* parent = nullptr);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // CHILDLINEEDIT_H
