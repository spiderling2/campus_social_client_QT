#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>

class EventWidget : public QWidget {
    Q_OBJECT
public:
    explicit EventWidget(QWidget* parent = nullptr);

signals:
    void createEventRequested(const QString& userName,const QString& eventName);
    void joinEventRequested(const QString& userName,const QString& eventName);
    void eventSelected(const QString& userName,const QString& eventName);

private:
    QLineEdit* eventEdit;
    QPushButton* createBtn;
    QPushButton* joinBtn;
    QListWidget* joinedList;

    void setupLayout();
    void setupConnections();
};
