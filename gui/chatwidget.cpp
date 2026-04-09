#include "chatwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include<QString>
#include"../service/userservice.h"
ChatWidget::ChatWidget(QWidget* parent) : QWidget(parent) {
    chatArea = new QTextBrowser(this);
    msgEdit = new QLineEdit(this);
    sendBtn = new QPushButton("Send", this);

    setupLayout();
    setupConnections();
    switchToEvent("default_event");
}

void ChatWidget::setupLayout() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(chatArea);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Message:"));
    inputLayout->addWidget(msgEdit);
    inputLayout->addWidget(sendBtn);

    layout->addLayout(inputLayout);
}

void ChatWidget::setupConnections() {
    connect(sendBtn, &QPushButton::clicked, [this]() {
        const QString message = msgEdit->text().trimmed();
        if (message.isEmpty()) {
            return;
        }
        QString userName=UserService::instance().get_username();
        emit sendMessageRequested(userName,currentEvent, message);
        msgEdit->clear();
    });
}

void ChatWidget::appendMessage(const QString &msg) {


    chatArea->append(msg);
}


void ChatWidget::switchToEvent(const QString& eventName) {
    currentEvent = eventName;
    qDebug()<<"current event is"<<eventName;
    chatArea->append(QString("系统：已切换到事件 '%1'").arg(currentEvent));
}
