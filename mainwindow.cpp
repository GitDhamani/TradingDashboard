#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "helpers.h"
#include "newtradedialog.h"

Ui::MainWindow* ui_ptr;
MainWindow* this_ptr;
extern bool play;
extern QTimer* timer;
extern void updateBalance();
extern double depositSum;
bool closeProperly = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Externalise the main pointers
    this_ptr = this;
    ui_ptr = ui;

    //Initialise the GUI
    guiInit();

    //Connect Statements for the Menu Bar
    MenuBarConnectStatements();

    //Connect Statements for the Login Page
    LoginPageConnectStatements();

    //Connect Statements for the Main Page
    MainPageConnectStatements();

    //Initialise the TableWidget
    initialiseTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    // Create the context menu
    QMenu contextMenu("Context Menu", this);
    contextMenu.addAction("Place Trade");

    //Check if any Trades exist
    if (tradeVector.size() != 0)
    {
        contextMenu.addSeparator();
        contextMenu.addAction("Close All Trades");
    }

    contextMenu.setStyleSheet("QMenu::item {padding: 2px 35px 2px 5px;} \
                              QMenu::item:selected {background-color: rgb(0, 85, 127); color: rgb(255, 255, 255);} \
                              QMenu::item:hover {background-color: rgb(0, 85, 127); color: rgb(255, 255, 255);} \
                              QMenu::separator {background-color: rgb(225, 225, 225); height: 2px; margin: 5px 0px;}");

    // Show the Menu
    QAction* selectedAction = contextMenu.exec(ui_ptr->tableWidget->mapToGlobal(pos));

    //Handle nothing being clicked
    if (selectedAction == nullptr) return;

    // Handle the selected action (if any)
    if (selectedAction->text() == "Place Trade")
    {
        //No Trades unless there is a balance above the trade fee of brokerFee * 2
        double getBalance = ui->balanceLabel->text().toDouble();
        if (getBalance < (2 * this->brokerFee))
        {
            QMessageBox::warning(this, "Warning", "You do not have sufficient funds to make a trade.\nPlease make a Deposit.");
            return;
        }

        placeNewTrade();
    }
    else if (selectedAction->text() == "Close All Trades")
    {
        clearTrades();
    }
    else
    {
        // No action was selected
    }
}

void MainWindow::placeNewTrade()
{
    //Stop Timer
    ui_ptr->playButton->setIcon(QIcon(":/RC/PlayIcon.ico"));
    play = 0;
    delete timer;
    timer = nullptr;

    //Make a temp copy of the trade vector
    QVector<QVector<QString>> tradeVectorCopy = tradeVector;

    NewTradeDialog* dialog = new NewTradeDialog(this);
    dialog->exec();
    if (closeProperly == 0)
    {
        //They exited the Trade Dialog.
        tradeVector = tradeVectorCopy;
        return;
    }
    addNewTrade();
}

void MainWindow::initialiseTable()
{
        //Prepare Table Headers
        headers << "Trade" << "Pair" << "Price" << "Stop" << "Target" << "Profit";

        //Initialise the table
        ui->tableWidget->setFont(QFont("Arial", 10, QFont::Bold));
        ui->tableWidget->horizontalHeader()->setStyleSheet("color: #484848;");
        int rows = tradeVector.size();
        int cols = headers.length();

        ui->tableWidget->setColumnCount(cols);
        ui->tableWidget->setRowCount(rows);
        ui->tableWidget->setAlternatingRowColors(true);
        ui->tableWidget->setColumnWidth(0, 150);
        ui->tableWidget->verticalHeader()->setVisible(false);   //Turns off Row Numbers.

        //Let the columns stretch to all the available space, even when the GUI is resized.
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        //Load Headers
        ui->tableWidget->setHorizontalHeaderLabels(headers);
}

void MainWindow::addNewTrade()
{
    int origRowNum = ui->tableWidget->rowCount();
    int rows = tradeVector.size();
    int cols = headers.length();
    ui->tableWidget->setRowCount(rows);
    if (rows == 0) return;

    for(int i = origRowNum; i < rows; i++)
        for(int j = 0; j < cols; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            item->setFont(QFont("Arial",9,-1,false));
            ui->tableWidget->setItem(i, j, item);
            ui->tableWidget->item(i,j)->setText(tradeVector[i][j]);
        }
        updateBalance();
}

void MainWindow::clearTrades()
{
    tradeVector.clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    depositSum = ui->balanceLabel->text().toDouble();
}
