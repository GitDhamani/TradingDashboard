#ifndef HELPERS_H
#define HELPERS_H

//Headers
#include <Windows.h>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QList>
#include <functional>
#include <QDebug>
#include <string>
#include <winreg.h>
#include <QSettings>
#include <QMessagebox>
#include <QTimer>
#include "currency.h"

//Prototypes
void LoginPageConnectStatements();
void MainPageConnectStatements();
void addFunds();
void processLoginPassword();
void guiInit();
std::string RegGetKeyValue(std::string key, std::string subkey);
int RegAddKey(std::string key, std::string subkey, std::string value);
int RegDeleteValue(std::string keypath, std::string value);
void MenuBarConnectStatements();
void startClock();
void killClock();
void updateClock();
void initialiseTimeDateLabels();
void initialiseCurrencyObjects();
void readNewDate();
void updateBalance();


#endif // HELPERS_H
