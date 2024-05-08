#include "helpers.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "choosedatedialog.h"
#include "drawdowngraph.h"

extern MainWindow* this_ptr;
extern Ui::MainWindow* ui_ptr;
bool play = false;
QTimer* timer;
QFile calendarFile(":/RC/GBPUSD.csv");
QTextStream in(&calendarFile);
QVector<Currency*> currencyVec;
bool stepMode = 0;
double depositSum;

void guiInit()
{
    this_ptr->setFixedSize(970,545);
    ui_ptr->errorLabel->setHidden(true);
    ui_ptr->usernameEdit->setFocus();
    ui_ptr->stackedWidget->setCurrentIndex(0);
    ui_ptr->balanceLabel->setText("0");
    initialiseTimeDateLabels();
    initialiseCurrencyObjects();

    //Disable Certain Menu Items
    ui_ptr->actionLogout->setDisabled(true);
    ui_ptr->actionChoose_Date->setDisabled(true);
    ui_ptr->actionReset_Timeline->setDisabled(true);
    ui_ptr->actionReset_Login->setDisabled(false);
    ui_ptr->actionEquity_Curve->setDisabled(true);
    ui_ptr->actionSettings->setDisabled(true);
}

void cleanup()
{
    if (calendarFile.isOpen())
    calendarFile.close();
    if (currencyVec.size() > 0)
    for (int i = 0; i < currencyVec.size(); i++)
        delete currencyVec[i];
    QApplication::exit();
}

void MenuBarConnectStatements()
{
    //Reset Login
    this_ptr->connect(ui_ptr->actionReset_Login, &QAction::triggered, [](){
        RegDeleteValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\TraderDash", "pwHash");
        if (ui_ptr->stackedWidget->currentIndex() == 0)
        {
            ui_ptr->usernameEdit->clear();
            ui_ptr->passwordEdit->clear();
            ui_ptr->errorLabel->setVisible(false);
            ui_ptr->usernameEdit->setFocus();
        }
        QMessageBox::information(this_ptr, "Login Reset", "Login has been Reset.");
    });

    //Log out
    this_ptr->connect(ui_ptr->actionLogout, &QAction::triggered, [](){
        if (ui_ptr->stackedWidget->currentIndex() == 1)
        {
            //Stop Timer
            ui_ptr->playButton->setIcon(QIcon(":/RC/PlayIcon.ico"));
            play = 0;
            delete timer;
            timer = nullptr;
        }

        ui_ptr->usernameEdit->clear();
        ui_ptr->passwordEdit->clear();
        ui_ptr->errorLabel->setVisible(false);
        ui_ptr->stackedWidget->setCurrentIndex(0);
        ui_ptr->usernameEdit->setFocus();

        //Disable Certain Menu Items
        ui_ptr->actionLogout->setDisabled(true);
        ui_ptr->actionChoose_Date->setDisabled(true);
        ui_ptr->actionReset_Timeline->setDisabled(true);
        ui_ptr->actionReset_Login->setDisabled(false);
        ui_ptr->actionEquity_Curve->setDisabled(true);
        ui_ptr->actionSettings->setDisabled(true);
    });

    //Exit
    this_ptr->connect(ui_ptr->actionExit, &QAction::triggered, [](){
        //Stop Timer
        delete timer;
        timer = nullptr;
        cleanup();
    });

    //About
    this_ptr->connect(ui_ptr->actionAbout, &QAction::triggered, [](){

        // Create your HTTPS link
        QString link1 = "<a href=\"https://gitdhamani.github.io\">https://gitdhamani.github.io</a>";
        QString link2 = "<a href=\"mailto:m.dhamani@hotmail.com\">m.dhamani@hotmail.com</a>";


        // Set up your QMessageBox with rich text
        QMessageBox msgBox;
        msgBox.setWindowTitle("About");
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText("<span style=\"font-family: Arial; font-size: 10pt;\">Trading Dashboard v1.0.0<br>Developed by M. Dhamani<br>Github: " + link1 + "<br>Email: &nbsp;" + link2 + "</span>");
        msgBox.exec();
    });

    //Settings
    this_ptr->connect(ui_ptr->actionSettings, &QAction::triggered, [](){
        //Launch Gui
        SettingsDialog* dialog = new SettingsDialog(this_ptr);
        dialog->exec();
    });

    //Reset Timeline
    this_ptr->connect(ui_ptr->actionReset_Timeline, &QAction::triggered, [](){
        //Reset Clock
        calendarFile.seek(0);
        in.setDevice((QIODevice*)(&calendarFile));
        readNewDate();

        //Reset Currency Labels
        for (auto e: currencyVec)
            e->resetLabels();

        //Wipe Trades
        this_ptr->tradeVector.clear();
        ui_ptr->tableWidget->clearContents();
        ui_ptr->tableWidget->setRowCount(0);

        //Clear Balance
        depositSum = 0;
        ui_ptr->balanceLabel->setText("0");

        //Reset Line Series
        this_ptr->drawDownSeries->clear();
    });

    //Choose Date
    this_ptr->connect(ui_ptr->actionChoose_Date, &QAction::triggered, [](){

        ChooseDateDialog* dateDialog = new ChooseDateDialog(this_ptr);
        dateDialog->exec();

        //Reset Currency Labels
        if (this_ptr->jumpDate != "")
        for (auto e: currencyVec)
            e->newLabelDate(this_ptr->jumpDate);

        //Reset Clock
        if (this_ptr->jumpDate != "")
        readNewDate();

        this_ptr->jumpDate = "";
    });

    //Equity Curve
    this_ptr->connect(ui_ptr->actionEquity_Curve, &QAction::triggered, [](){
        DrawDownGraph* drawdown = new DrawDownGraph();
        drawdown->exec();
    });
}

void LoginPageConnectStatements()
{
    //Connect Statement for the Login Page Login Button
    this_ptr->connect(ui_ptr->loginButton, &QPushButton::clicked, [](){ processLoginPassword(); });
}

void MainPageConnectStatements()
{
    //Add Funds Button
    this_ptr->connect(ui_ptr->fundsButton, &QPushButton::clicked, [](){ addFunds(); });

    //Play/Pause Button
    this_ptr->connect(ui_ptr->playButton, &QPushButton::clicked, [](){
        if (!play)
        startClock();
        else
        killClock();
    });

    //TimeStep Checkbox Button
    this_ptr->connect(ui_ptr->stepButton, &QPushButton::clicked, [](){
        if (!stepMode)
        {
            ui_ptr->stepButton->setIcon(QIcon(":/RC/StepOn.ico"));
            stepMode = 1;
        }
        else
        {
            ui_ptr->stepButton->setIcon(QIcon(":/RC/StepOff.ico"));
            stepMode = 0;
        }
    });
}

void startClock()
{
    //Check balance hasn't been blown:
     double currentBalance = ui_ptr->balanceLabel->text().toDouble();
     if (currentBalance < 0) return;

    ui_ptr->playButton->setIcon(QIcon(":/RC/PauseIcon.ico"));
    play = 1;
    timer = new QTimer;
    timer->setInterval(this_ptr->timestepInterval);

    if (stepMode){
        updateClock();
        ui_ptr->playButton->setIcon(QIcon(":/RC/PlayIcon.ico"));
        play = 0;
        updateBalance();
        return;
    }
    timer->start();
    QObject::connect(timer, &QTimer::timeout, []() {
        updateClock();
        updateBalance();
    });
}

void updateBalance()
{
     //qDebug() << "Update Balance Called";
    // Iterate through each row in the table

    double rowProfit;
    double totalProfit = 0;

    for (int row = 0; row < ui_ptr->tableWidget->rowCount(); ++row) {
        rowProfit = ui_ptr->tableWidget->item(row, 5)->text().toDouble();
        totalProfit += rowProfit;
    }

    //Set Label Text
     ui_ptr->balanceLabel->setText(QString::number(depositSum + totalProfit));

    if ((depositSum + totalProfit) < 0)
    {
        ui_ptr->playButton->setIcon(QIcon(":/RC/PlayIcon.ico"));
        play = 0;
        delete timer;
        timer = nullptr;
        QMessageBox::warning(0, "Broker Call", "Account Blown. Deposit Funds to Continue.");
    }
}

void killClock()
{
    ui_ptr->playButton->setIcon(QIcon(":/RC/PlayIcon.ico"));
    play = 0;
    timer->stop(); //To stop timer at some point.
    delete timer;
    timer = nullptr;
}

void addFunds()
{
    //Get Value in Balance Label
    double currentBalance = ui_ptr->balanceLabel->text().toDouble();
    currentBalance += 1000;
    depositSum += 1000;
    ui_ptr->balanceLabel->setText(QString::number(currentBalance));

    //Update Line Series
    int recordNum = this_ptr->drawDownSeries->count();
    this_ptr->drawDownSeries->append(recordNum,currentBalance);
}

void initialiseTimeDateLabels()
{
        if (!calendarFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << calendarFile.errorString();
            return;
        }
        readNewDate();
}

void readNewDate()
{
        QString line = in.readLine();

        int lineCount = 0;
        if (this_ptr->jumpDate != "")
        {
            while (true)
            {
                lineCount++;
                line = in.readLine();
                QStringList values = line.split(",");
                if (values[0].contains(this_ptr->jumpDate))
                    {
                        if (values[1].contains(this_ptr->jumpTime + ":"))
                        break;

                        if (values[1].left(2) > this_ptr->jumpTime)
                        {
                            qDebug() << "Hour not Found in Clock";
                            break;
                        }
                    }
            }
        }

        // Split the line by comma
        QStringList values = line.split(",");

        // Ensure there are at least three values
        if (values.size() < 3)
        {
            qDebug() << "Insufficient values in the file.";
            return;
        }

        // Extract the first two values
        QString value1 = values[0];
        QString value2 = values[1];

        // Extract the original date value
        QString originalDate = values[0];

        // Split the original date by period
        QStringList dateComponents = originalDate.split(".");

        // Ensure there are three components (year, month, day)
        if (dateComponents.size() != 3)
        {
            qDebug() << "Invalid date format in the file.";
            return;
        }

        // Rearrange the date components to the desired format (day.month.year)
        QString rearrangedDate = dateComponents[2] + "." + dateComponents[1] + "." + dateComponents[0];

        ui_ptr->dateLabel->setText(rearrangedDate);
        ui_ptr->timeLabel->setText(value2);
}


void initialiseCurrencyObjects()
{
    const QString currencyNames[24] =
        {
            "GBPUSD", "EURUSD", "EURGBP", "USDJPY", "USDCHF", "AUDUSD", "USDCAD", "NZDUSD",
            "GBPAUD", "GBPCAD", "GBPCHF", "GBPJPY", "EURAUD", "EURCAD", "EURNZD", "NZDCAD",
            "AUDCAD", "EURCHF", "AUDNZD", "EURJPY", "NZDJPY", "AUDJPY", "CADCHF", "CADJPY"
        };

        // Create and connect Currency objects in a loop
        for (int i = 0; i < 24; ++i)
        {
            Currency* currency = new Currency(currencyNames[i]);
            currencyVec.push_back(currency);
            QObject::connect(this_ptr, SIGNAL(mySignal()), currency, SLOT(handleSignal()));
            QObject::connect(this_ptr, SIGNAL(updatePricePerPip()), currency, SLOT(updatePricePerPip()));
        }

}

void updateClock()
{
    readNewDate();
    emit this_ptr->mySignal();
}

void processLoginPassword()
{
    //Checks to see if a Password Registry Key pwHash exists, if not hashes the password and adds it.
    //If password key already exists the password hashes are compared.
    //If hashed password comparison incorrect, user will get an error message.
    //If hashed password comparison is correct, user will get access to the 2nd page.

    //Get Password from Edit Field
    std::string enteredPW = ui_ptr->passwordEdit->text().toStdString();

    //Do not allow empty Passwords
    if (enteredPW.empty()) return;

    //Hash it using C++ 11 standard Hash.
    size_t enteredPWHash = std::hash<std::string>{}(enteredPW);
    std::string enteredPWHashStr = QString::number(enteredPWHash).toStdString();

    //Check if Key exists
    QString regPWString = QString::fromStdString(RegGetKeyValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\TraderDash", "pwHash"));

    //Check if Key Exists
    if (regPWString != "Key Not Found")
    {
        //Key Exists, Compare Hashes
        if (regPWString.toStdString().compare(enteredPWHashStr)==0)
        {
            ui_ptr->stackedWidget->setCurrentIndex(1);
            //Enable Certain Menu Items
            ui_ptr->actionLogout->setDisabled(false);
            ui_ptr->actionChoose_Date->setDisabled(false);
            ui_ptr->actionReset_Timeline->setDisabled(false);
            ui_ptr->actionReset_Login->setDisabled(true);
            ui_ptr->actionEquity_Curve->setDisabled(false);
            ui_ptr->actionSettings->setDisabled(false);
        }
        else
            ui_ptr->errorLabel->setHidden(false);
        return;
    }
    else
    {
        //Key doesn't exist, create it
        RegAddKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\TraderDash", "pwHash", enteredPWHashStr);
        ui_ptr->stackedWidget->setCurrentIndex(1);
        //Enable Certain Menu Items
        ui_ptr->actionLogout->setDisabled(false);
        ui_ptr->actionChoose_Date->setDisabled(false);
        ui_ptr->actionReset_Timeline->setDisabled(false);
        ui_ptr->actionReset_Login->setDisabled(true);
        ui_ptr->actionEquity_Curve->setDisabled(false);
        ui_ptr->actionSettings->setDisabled(false);
        return;
    }
}


std::string RegGetKeyValue(std::string key, std::string subkey)
{
    HKEY hKey;
    DWORD dwType, dwSize;
    char buffer[700]; //Max Value Length: You can change this.

    // Extract root key from key path
    std::string rootKey = key.substr(0, key.find('\\'));

    // Convert root key handle to HKEY
    HKEY hRootKey;
    if (rootKey == "HKEY_CLASSES_ROOT") {
        hRootKey = HKEY_CLASSES_ROOT;
    }
    else if (rootKey == "HKEY_CURRENT_CONFIG") {
        hRootKey = HKEY_CURRENT_CONFIG;
    }
    else if (rootKey == "HKEY_CURRENT_USER") {
        hRootKey = HKEY_CURRENT_USER;
    }
    else if (rootKey == "HKEY_LOCAL_MACHINE") {
        hRootKey = HKEY_LOCAL_MACHINE;
    }
    else if (rootKey == "HKEY_USERS") {
        hRootKey = HKEY_USERS;
    }
    else {
        return "Invalid root key handle.";
    }

    //Remove Root key from the string
    key.erase(0, rootKey.length() + 1);  //The +1 accounts for the null terminator at the end of the rootkey.

    // open the registry key
    LONG result = RegOpenKeyExA(hRootKey, key.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        //qDebug() << "Failed to open registry key.";
        return "Key Not Found";
    }

    // get the value of the registry key
    dwSize = sizeof(buffer);
    result = RegQueryValueExA(hKey, subkey.c_str(), NULL, &dwType, (LPBYTE)&buffer, &dwSize);

    if (result != ERROR_SUCCESS)
    {
        //qDebug() << "Failed to retrieve registry value.";
        return "Key Not Found";
    }

    // close the registry key
    RegCloseKey(hKey);

    return buffer;
}


int RegAddKey(std::string key, std::string subkey, std::string value)
{
    HKEY hKey;
    DWORD dwDisposition;

    // Extract root key from key path
    std::string rootKey = key.substr(0, key.find('\\'));

    // Convert root key handle to HKEY
    HKEY hRootKey;
    if (rootKey == "HKEY_CLASSES_ROOT") {
        hRootKey = HKEY_CLASSES_ROOT;
    }
    else if (rootKey == "HKEY_CURRENT_CONFIG") {
        hRootKey = HKEY_CURRENT_CONFIG;
    }
    else if (rootKey == "HKEY_CURRENT_USER") {
        hRootKey = HKEY_CURRENT_USER;
    }
    else if (rootKey == "HKEY_LOCAL_MACHINE") {
        hRootKey = HKEY_LOCAL_MACHINE;
    }
    else if (rootKey == "HKEY_USERS") {
        hRootKey = HKEY_USERS;
    }
    else {
        //return "Invalid root key handle.";
        return 0;
    }

    //Remove Root key from the string
    key.erase(0, rootKey.length() + 1);

    // create the registry key
    LONG result = RegCreateKeyExA(hRootKey, key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);

    if (result != ERROR_SUCCESS)
    {
        //qDebug() << "Failed to create registry key.";
    }

    // set a value for the registry key
    const char* valueData = value.c_str();
    result = RegSetValueExA(hKey, subkey.c_str(), 0, REG_SZ, (const BYTE*)valueData, strlen(valueData) + 1);

    if (result != ERROR_SUCCESS)
    {
        //qDebug() << "Failed to set registry key.";
    }

    // close the registry key
    RegCloseKey(hKey);
    return 1;

}

int RegDeleteValue(std::string key, std::string value)
{
    HKEY hKey;
    DWORD dwType, dwSize;

    // Extract root key from key path
    std::string rootKey = key.substr(0, key.find('\\'));

    // Convert root key handle to HKEY
    HKEY hRootKey;
    if (rootKey == "HKEY_CLASSES_ROOT") {
        hRootKey = HKEY_CLASSES_ROOT;
    }
    else if (rootKey == "HKEY_CURRENT_CONFIG") {
        hRootKey = HKEY_CURRENT_CONFIG;
    }
    else if (rootKey == "HKEY_CURRENT_USER") {
        hRootKey = HKEY_CURRENT_USER;
    }
    else if (rootKey == "HKEY_LOCAL_MACHINE") {
        hRootKey = HKEY_LOCAL_MACHINE;
    }
    else if (rootKey == "HKEY_USERS") {
        hRootKey = HKEY_USERS;
    }
    else {
        return 0;
    }

    //Remove Root key from the string
    key.erase(0, rootKey.length() + 1);  //The +1 accounts for the null terminator at the end of the rootkey.

    // open the registry key
    LONG result = RegOpenKeyExA(hRootKey, key.c_str(), 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        //qDebug() << "Failed to Open registry key.";
    }


    LONG deleteStatus = RegDeleteValueA(hKey, value.c_str());
    if (deleteStatus != ERROR_SUCCESS) {
        //qDebug() << "Error deleting value: " << deleteStatus << std::endl;
        RegCloseKey(hKey);
        return 0;
    }


    RegCloseKey(hKey);
    return 1; //Success

}
