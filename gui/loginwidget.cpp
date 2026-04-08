#include "loginwidget.h"
#include <QHBoxLayout>
#include <QLabel>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent,Qt::Window) {
    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginBtn = new QPushButton("Login", this);
    registerBtn = new QPushButton("Register", this);
    logoutBtn = new QPushButton("Logout", this);

    setupLayout();
    setupConnections();
    setWindowTitle("用户登录");  // 弹窗标题
    resize(400, 60);            // 设置窗口大小
}

void LoginWidget::setupLayout() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(new QLabel("Username:"));
    layout->addWidget(usernameEdit);
    layout->addWidget(new QLabel("Password:"));
    layout->addWidget(passwordEdit);
    layout->addWidget(loginBtn);
    layout->addWidget(registerBtn);
    layout->addWidget(logoutBtn);
}

void LoginWidget::setupConnections() {
    connect(loginBtn, &QPushButton::clicked, [this]() {
        emit loginRequested(usernameEdit->text(), passwordEdit->text());
    });
    connect(registerBtn, &QPushButton::clicked, [this]() {
        emit registerRequested(usernameEdit->text(), passwordEdit->text());
    });
    connect(logoutBtn, &QPushButton::clicked, [this]() {
        emit logoutRequested();
    });
}
