#include "currency.h"
extern MainWindow* this_ptr;
extern Ui::MainWindow* ui_ptr;
extern double depositSum;

QString formatCurrencyPair(const QString& currencyPair) {
    if (currencyPair.length() != 6) {
        qDebug() << "Invalid currency pair length. It should be 6 characters long.";
        return QString(); // Return an empty string indicating failure
    }

    QString formattedPair;
    formattedPair += currencyPair.left(3); // Extract the first 3 characters (e.g., GBP)
    formattedPair += " / ";
    formattedPair += currencyPair.right(3); // Extract the last 3 characters (e.g., USD)

    return formattedPair;
}

Currency::Currency(QString Name): currencyName{Name}
{
    currencyFilePath = ":/RC/" + currencyName + ".csv";
    fileHandle = new QFile(currencyFilePath);
    in2.setDevice((QIODevice*)(fileHandle));

    // Open the file
    if (!fileHandle->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << fileHandle->errorString();
        qDebug() << "The file is: " << currencyFilePath;
        return;
    }
    updateLabel();
    setWhiteBackground();
}

void Currency::updateLabel()
{
    // Read the contents of the file
    QString line = in2.readLine();

    int lineCount = 0;
    bool labelNotFound = 0;

    if (this_ptr->jumpDate != "")
    while (true)
    {
        lineCount++;
        line = in2.readLine();
        QStringList values = line.split(",");
        if (values[0].contains(this_ptr->jumpDate))
        {
            if (values[1].contains(this_ptr->jumpTime + ":"))
            break;

            if (values[1].left(2) > this_ptr->jumpTime)
            {
                //If the exact hour isn't found, load in the next nearest hour.
                qDebug() << currencyName << ": Error Hour not found";
                break;
            }
        }
    }

    // Split the line by comma
    QStringList values = line.split(",");

    // Ensure there are at least three values
    if (values.size() < 3) {
        qDebug() << "Insufficient values in the file.";
        return;
    }

    // Extract the first two values
    QString openPrice = values[2];

    QString openPrice1 = openPrice.left(5);
    QString openPrice2 = openPrice.right(3);

    //if currency pair is a JPY we need to split off the price a little differently
    if (currencyName.contains("JPY"))
    openPrice2 = (openPrice.left(8)).right(3);

    //Find all your label pointers
    // Prepare the regular expression pattern to match label names
    QString pattern = QString("label%1\\d+").arg(currencyName);

    // Compile the regular expression
    QRegularExpression regex(pattern);

    // Find children labels that match the pattern
    QList<QLabel*> labels = this_ptr->findChildren<QLabel*>(regex);

    //Fill the Vector
    QVector<QString> vec;
    vec.append(currencyName);
    vec.append(openPrice1);
    vec.append(openPrice2);

    labels[0]->setText(formatCurrencyPair(currencyName));
    labels[1]->setText(openPrice1);
    labels[2]->setText(openPrice2);

    QWidget* parentPtr = labels[0]->parentWidget();

    if (openPrice.toDouble() > currencyValue)
        //Green Background
        parentPtr->setStyleSheet("background-color: rgb(170, 255, 127); border-radius: 10px");
    else
        //Red Background
        parentPtr->setStyleSheet("background-color: rgb(255, 148, 134); border-radius: 10px");

    //Update Currency Value
    currencyValue = openPrice.toDouble();

    //Update Values in the Table Widget
    updateTradeTable();
}

void Currency::updateTradeTable()
{
    //If we're a JPY Pair, update PIPs to 0.01
    if (currencyName.contains("JPY"))
    {
        pipValue = 0.01;
        pipMultiplier = 100;
    }

    calcPricePerPip();

    //Scan the Table for Trades containing our Currency Pair
    // Iterate through each row in the table
    for (int row = 0; row < ui_ptr->tableWidget->rowCount(); ++row) {

        //We're ignoring any Row where the first column says Won or Lost
        QString firstColumn = ui_ptr->tableWidget->item(row, 0)->text();
        if (firstColumn.contains("Won") || firstColumn.contains("Lost"))
            continue; //Skip this Column

        // Get the item in the second column for the current row
        QTableWidgetItem *item = ui_ptr->tableWidget->item(row, 1); // Column index 1 corresponds to the second column

        if (item) {
            // Check if the item's text contains our Currency Name
            if (item->text().contains(currencyName)) {
                // Do something when "GBP" is found in the second column
                // For example, you can print some information or perform additional actions

                tradeType = ui_ptr->tableWidget->item(row, 0)->text();
                orderPrice = ui_ptr->tableWidget->item(row, 2)->text().toDouble();
                stopPrice = ui_ptr->tableWidget->item(row, 3)->text().toDouble();
                targetPrice = ui_ptr->tableWidget->item(row, 4)->text().toDouble();

                orderRow = row;

                updateRowProfit();

            }
        }
    }
}

void Currency::markTradeResult(QString result)
{
    if (result == "Winner")
    {
        int columnCount = ui_ptr->tableWidget->columnCount();
            for (int column = 0; column < columnCount; ++column) {
                QTableWidgetItem *item = ui_ptr->tableWidget->item(orderRow, column);
                if (item) item->setForeground(QColor("#00aa00"));
            }
    }

    if (result == "Loser")
    {
        int columnCount = ui_ptr->tableWidget->columnCount();
            for (int column = 0; column < columnCount; ++column) {
                QTableWidgetItem *item = ui_ptr->tableWidget->item(orderRow, column);
                if (item) item->setForeground(Qt::red);
            }

    }

    //Store Balance for Draw Down Graph
    double rowProfit;
    double totalProfit = 0;

    for (int row = 0; row < ui_ptr->tableWidget->rowCount(); ++row) {
        rowProfit = ui_ptr->tableWidget->item(row, 5)->text().toDouble();
        totalProfit += rowProfit;
    }

    int recordNum = this_ptr->drawDownSeries->count();
    this_ptr->drawDownSeries->append(recordNum,depositSum + totalProfit);
}

void Currency::resetLabels()
{
    fileHandle->seek(0);
    in2.setDevice((QIODevice*)(fileHandle));
    updateLabel();
    setWhiteBackground();
}

void Currency::newLabelDate(QString date)
{
    updateLabel();
    setWhiteBackground();
}

void Currency::updateRowProfit()
{
    //For Non JPY Pairs the pip difference is found by multiplying by 10,000.
    //for JPY Pairs the pip difference is found by multiplying by 100.

    if (tradeType == "Long")
    {
        if (currencyValue >= targetPrice)
        {

            //mark First column as Won
            ui_ptr->tableWidget->item(orderRow, 0)->setText("Won");
            double profitvalue = (targetPrice - orderPrice) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;

            //add (target price - order price) x priceperpip - 2 x brokerfee to profit column
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));

            if (profitvalue > 0)
            markTradeResult("Winner");
            else
                markTradeResult("Loser");
        }


        if (currencyValue <= stopPrice)
        {
            //mark First column as Lost
            ui_ptr->tableWidget->item(orderRow, 0)->setText("Lost");
            //markTradeResult("Loser");

            double profitvalue = (stopPrice - orderPrice) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;

            //add (stop price - order price) * priceperpip - 2 x brokefree to profit column
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));

            if (profitvalue > 0)
            markTradeResult("Winner");
            else
                markTradeResult("Loser");
        }

        if (currencyValue < targetPrice && currencyValue > stopPrice)
        {
            //Update profit column with: (current price - order price) x priceperpip - 2 x broker fee
            double profitvalue = (currencyValue - orderPrice) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));
        }

    }

    if (tradeType == "Short")
    {
        if (currencyValue <= targetPrice)
        {
            //mark First column as Won
            ui_ptr->tableWidget->item(orderRow, 0)->setText("Won");
            double profitvalue = (orderPrice - targetPrice) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;
            //markTradeResult("Winner");

            //add (order Price - target price) x priceperpip - 2 x brokerfee to profit column
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));

            if (profitvalue > 0)
            markTradeResult("Winner");
            else
                markTradeResult("Loser");
        }

        if (currencyValue >= stopPrice)
        {
            //mark First column as Lost
            ui_ptr->tableWidget->item(orderRow, 0)->setText("Lost");

            double profitvalue = (orderPrice - stopPrice) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;

            //add (order price - Stop price) * priceperpip - 2 x brokefree to profit column
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));

            if (profitvalue > 0)
            markTradeResult("Winner");
            else
                markTradeResult("Loser");
        }

        if (currencyValue > targetPrice && currencyValue < stopPrice)
        {
            //Update profit column with: (order price - Current Price) x priceperpip - 2 x broker fee
            double profitvalue = (orderPrice - currencyValue) * pipMultiplier * pricePerPip - 2 * this_ptr->brokerFee;
            ui_ptr->tableWidget->item(orderRow, 5)->setText(QString::number(profitvalue));
        }


    }
}

void Currency::calcPricePerPip()
{
        if (formatCurrencyPair(currencyName).contains("GBP /"))
        {
            pricePerPip = (pipValue * this_ptr->lotSize)/currencyValue;
            return;
        }

        if (formatCurrencyPair(currencyName).contains("/ GBP"))
        {
            pricePerPip = pipValue * this_ptr->lotSize;
            return;
        }

        if (!currencyName.contains("GBP"))
        {
            //Extract the Quote Currency
            QString quoteCurrency = currencyName.right(3);

            // Prepare the regular expression pattern to match label names
            QString pattern = QString("label%1\\d+").arg("GBP" + quoteCurrency);

            // Compile the regular expression
            QRegularExpression regex(pattern);

            // Find children labels that match the pattern
            QList<QLabel*> labels = this_ptr->findChildren<QLabel*>(regex);
            if (labels.size() == 0)
            {
                //If Reference Rate not found, set Price Per Pip to 10 as a default
                pricePerPip = 10;
                return;
            }

            //Get the Reference Rate
            double refRate = 1/(labels[1]->text().toDouble());

            pricePerPip = pipValue * this_ptr->lotSize * refRate;
       }
}

void Currency::setWhiteBackground()
{
    //Set Parent Widget to White Background to Start With
    // Find children labels that match the pattern
    // Prepare the regular expression pattern to match label names
    QString pattern = QString("label%1\\d+").arg(currencyName);

    // Compile the regular expression
    QRegularExpression regex(pattern);

    QList<QLabel*> labels = this_ptr->findChildren<QLabel*>(regex);
    QWidget* parentPtr = labels[0]->parentWidget();
    parentPtr->setStyleSheet("background-color: rgb(255, 255, 255); border-radius: 10px");
}

Currency::~Currency()
{
    // Close the file
    fileHandle->close();
}

void Currency::handleSignal()
{
    updateLabel();
}

void Currency::updatePricePerPip()
{
    calcPricePerPip();
}
