#include "eventwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include"../service/userservice.h"
EventWidget::EventWidget(QWidget* parent) : QWidget(parent) {
    eventEdit = new QLineEdit(this);
    createBtn = new QPushButton("Create Event", this);
    joinBtn = new QPushButton("Join Event", this);
    joinedList = new QListWidget(this);

    setupLayout();
    setupConnections();
}

void EventWidget::setupLayout() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* top = new QHBoxLayout();
    top->addWidget(new QLabel("Event:"));
    top->addWidget(eventEdit);
    top->addWidget(createBtn);
    top->addWidget(joinBtn);

    layout->addLayout(top);
    layout->addWidget(joinedList);
}

void EventWidget::setupConnections() {
    connect(createBtn, &QPushButton::clicked, [this]() {
        const QString eventName = eventEdit->text().trimmed();
        if (!eventName.isEmpty()) {
            QString userName=UserService::instance().get_username();
            emit createEventRequested(userName,eventName);
        }
    });
    connect(joinBtn, &QPushButton::clicked, [this]() {
        const QString eventName = eventEdit->text().trimmed();
        if (eventName.isEmpty()) {
            return;
        }
        bool exists = false;
        for (int i = 0; i < joinedList->count(); ++i) {
            if (joinedList->item(i)->text() == eventName) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            joinedList->addItem(eventName);
        }
        QString userName=UserService::instance().get_username();
        emit joinEventRequested(userName,eventName);
    });

    connect(joinedList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        if (item != nullptr) {
             QString userName=UserService::instance().get_username();
            emit eventSelected(item->text());
        }
    });

}
void EventWidget::getEvents(QVariantList events)
{
    // Clear the current list
    joinedList->clear();

    // Iterate through the events and add them to the list
    for (const QVariant& eventVariant : events) {
        // Assuming each event is a QVariantMap or QString
        if (eventVariant.canConvert<QString>()) {
            QString eventName = eventVariant.toString();
            joinedList->addItem(eventName);
        }
        else if (eventVariant.canConvert<QVariantMap>()) {
            QVariantMap eventMap = eventVariant.toMap();
            QString eventName = eventMap.value("name", eventMap.value("eventName")).toString();
            if (!eventName.isEmpty()) {
                joinedList->addItem(eventName);
            }
        }
    }
}
