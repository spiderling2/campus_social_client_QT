#include "chatwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include<QString>
#include <QRegularExpression>
#include"../service/userservice.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
ChatWidget::ChatWidget(QWidget* parent) : QWidget(parent) {
    chatArea = new QTextBrowser(this);
    msgEdit = new RichTextEdit(this);
    sendBtn = new QPushButton("Send",this);

    setupLayout();
    setupConnections();
    switchToEvent("default_event");
    chatArea->setOpenLinks(false);
    chatArea->setOpenExternalLinks(false);

    connect(chatArea, &QTextBrowser::anchorClicked,
            this, [=](const QUrl &url) {

                QString filePath = url.toLocalFile();

                if (!QFile::exists(filePath)) {
                    QMessageBox::warning(this, "文件不存在",
                                         QString("文件不存在或已被删除：\n%1").arg(filePath));
                    return;
                }

                QDesktopServices::openUrl(url);
            });
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

void ChatWidget::appendFile(const QString &username, const QString &filename, const QString &filePath) {
    QFileInfo info(filePath);
    QString ext = info.suffix().toLower();

    QString user = "[" + username + "]";
    QString msg_final;

    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
        QImage image(filePath);
        if (!image.isNull()) {
            chatArea->append(user + ":");
            chatArea->document()->addResource(QTextDocument::ImageResource, QUrl(filePath), QVariant(image));
            chatArea->append(QString("<img src='%1'>").arg(filePath));
            return;
        }
    } else {
        // 普通文件显示为下载链接
        msg_final = QString("<b>%1:</b> <a href='file:///%2'>%3</a>").arg(user, filePath, filename);
    }

    chatArea->append(msg_final);
}
