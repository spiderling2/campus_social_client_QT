#include "mainwindow.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QAction>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    network = new NetworkClient(this);

    loginWidget = new LoginWidget(this);
    eventWidget = new EventWidget(this);
    chatWidget = new ChatWidget(this);
    fileWidget = new FileWidget(this);

    setupMenu();      // 顶部菜单栏
    setupLayout();    // 左中右布局
    setupConnections(); // 信号槽连接

    // 初始未登录状态禁用聊天和事件模块
    eventWidget->setEnabled(false);
    chatWidget->setEnabled(false);
    fileWidget->setEnabled(false);
}

void MainWindow::setupMenu() {
    QMenuBar* menuBar = new QMenuBar(this);

    QMenu* userMenu = menuBar->addMenu("用户");
    QAction* loginAction = userMenu->addAction("登录");
    QAction* registerAction = userMenu->addAction("注册");
    QAction* logoutAction = userMenu->addAction("退出登录");

    setMenuBar(menuBar);

    // 登录弹窗
    connect(loginAction, &QAction::triggered, this, [this]() {
        // 使用现有 LoginWidget
        loginWidget->show();
    });

    // 注册弹窗
    connect(registerAction, &QAction::triggered, this, [this]() {
        loginWidget->show();
    });

    // 退出登录
    connect(logoutAction, &QAction::triggered, loginWidget, &LoginWidget::logoutRequested);
}

void MainWindow::setupLayout() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // 左右可拖动布局
    QSplitter* hSplitter = new QSplitter(Qt::Horizontal, central);

    // 左侧事件列表
    hSplitter->addWidget(eventWidget);

    // 右侧聊天+文件
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(chatWidget, 3);
    rightLayout->addWidget(fileWidget, 1);

    QWidget* rightContainer = new QWidget();
    rightContainer->setLayout(rightLayout);
    hSplitter->addWidget(rightContainer);

    hSplitter->setStretchFactor(0, 1); // 左边占比
    hSplitter->setStretchFactor(1, 3); // 右边占比

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(hSplitter);
}

void MainWindow::setupConnections() {
    // 登录/注册/登出
    connect(loginWidget, &LoginWidget::loginRequested,
            network, &NetworkClient::login);
    connect(loginWidget, &LoginWidget::registerRequested,
            network, &NetworkClient::registerUser);
    connect(loginWidget, &LoginWidget::logoutRequested,
            network, &NetworkClient::logout);

    // 登录状态控制界面启用/禁用
    connect(network, &NetworkClient::loginSuccess, this, [this]() {
        eventWidget->setEnabled(true);
        chatWidget->setEnabled(true);
        fileWidget->setEnabled(true);
    });
    connect(network, &NetworkClient::logoutSuccess, this, [this]() {
        eventWidget->setEnabled(false);
        chatWidget->setEnabled(false);
        fileWidget->setEnabled(false);
    });

    // 事件操作
    connect(eventWidget, &EventWidget::createEventRequested,
            network, &NetworkClient::createEvent);
    connect(eventWidget, &EventWidget::joinEventRequested,
            network, &NetworkClient::joinEvent);

    // 聊天和文件发送
    connect(chatWidget, &ChatWidget::sendMessageRequested,
            network, &NetworkClient::sendMessage);
    connect(fileWidget, &FileWidget::sendFileRequested,
            network, &NetworkClient::sendFile);

    // 接收消息显示
    connect(network, &NetworkClient::messageReceived,
            chatWidget, &ChatWidget::appendMessage);

    // 选择事件切换聊天内容
    // connect(eventWidget, &EventWidget::eventSelected,
    //         chatWidget, &ChatWidget::switchToEvent);
}
