#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested(const QString& username, const QString& password);
    void logoutRequested(const QString&username);

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginBtn;
    QPushButton* registerBtn;
    QPushButton* logoutBtn;

    void setupLayout();
    void setupConnections();
};
