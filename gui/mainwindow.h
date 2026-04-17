#pragma once
#include <QMainWindow>
#include <QToolButton>
#include "LoginWidget.h"
#include "EventWidget.h"
#include "ChatWidget.h"
#include "FileWidget.h"
#include "../network/NetworkClient.h"
#include <QCloseEvent>
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);


private:
    LoginWidget* loginWidget;
    EventWidget* eventWidget;
    ChatWidget* chatWidget;
    FileWidget* fileWidget;
    NetworkClient* network;
    QToolButton* profileBtn;

    void setupLayout();
    void setupConnections();
    void setupMenu();
    void setupProfileMenu();
     void closeEvent(QCloseEvent* event) override;
};
