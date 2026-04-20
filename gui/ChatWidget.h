#pragma once
#include <QWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include "../models/richtextedit.h"


class ChatWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatWidget(QWidget* parent = nullptr);

signals:
    void sendMessageRequested(const QString& eventName, const QVariant& content);

public slots:
    void appendMessage(const QVariantMap &msg);

    // 新增槽：切换当前聊天事件
    void switchToEvent(const QString& eventName);

    void onSendClicked();
    void appendFile(const QString &username, const QString &filename, const QString &filePath);

private:
    QTextBrowser* chatArea;
    //QLineEdit* msgEdit;
    QPushButton* sendBtn;

    QString currentEvent; // 当前事件名

    //富文本
    RichTextEdit* msgEdit;



    void setupLayout();
    void setupConnections();


};

class IconButton : public QPushButton {
    Q_OBJECT
public:
    explicit IconButton(QWidget* parent = nullptr) : QPushButton(parent) {}
protected:
    void resizeEvent(QResizeEvent* event) override {
        QPushButton::resizeEvent(event);
        int size = qMin(width(), height());
        setIconSize(QSize(size, size));
    }
};


