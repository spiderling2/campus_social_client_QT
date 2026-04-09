#pragma once
#include <QWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>

class ChatWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatWidget(QWidget* parent = nullptr);

signals:
    void sendMessageRequested(const QString& userName,const QString& eventName, const QString& message);

public slots:
    void appendMessage(const QString &msg);

    // 新增槽：切换当前聊天事件
    void switchToEvent(const QString& eventName);

private:
    QTextBrowser* chatArea;
    QLineEdit* msgEdit;
    QPushButton* sendBtn;

    QString currentEvent; // 当前事件名


    void setupLayout();
    void setupConnections();
};
