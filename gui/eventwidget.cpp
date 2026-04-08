#include "eventwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

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
            emit createEventRequested(eventName);
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
        emit joinEventRequested(eventName);
    });

    connect(joinedList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            emit eventSelected(item->text());
        }
    });
}
