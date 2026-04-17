#include "chatwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include<QString>
#include <QRegularExpression>
#include"../service/userservice.h"
ChatWidget::ChatWidget(QWidget* parent) : QWidget(parent) {
    chatArea = new QTextBrowser(this);
    msgEdit = new RichTextEdit(this);
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
    connect(sendBtn, &QPushButton::clicked, this, &ChatWidget::onSendClicked);

}
void ChatWidget::appendMessage(const QVariantMap &msg) {

    QString user="["+msg["username"].toString()+"]";
    QString content=msg["content"].toString();

    QString decodedContent = QString::fromUtf8(content.toUtf8());
    QString msg_final = QString("<b>%1:</b> %2").arg(user, content);

    chatArea->append(msg_final);
}


void ChatWidget::switchToEvent(const QString& eventName) {
    currentEvent = eventName;
    chatArea->append(QString("系统：已切换到事件 '%1'").arg(currentEvent));

    //清空消息区域
}


void ChatWidget::onSendClicked() {
    QString textContent = msgEdit->toPlainText().trimmed();

    QList<QString> files = msgEdit->getAttachedFiles();

    if (!files.isEmpty()) {
        // 文件发送（图片/视频/音频/文档）

        emit sendMessageRequested(currentEvent,files);
        msgEdit->clearAttachedFiles();
    } else if (!textContent.isEmpty()) {
        // 普通文本发送
        QString sendContent = textContent;
        QString userName = UserService::instance().get_username();
        emit sendMessageRequested(currentEvent, sendContent);

    } else {
        qDebug() << "消息为空，未发送";
        return;
    }

    msgEdit->clear(); // 清空编辑器
}
