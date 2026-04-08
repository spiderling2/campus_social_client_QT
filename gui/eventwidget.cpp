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
        emit createEventRequested(eventEdit->text());
    });
    connect(joinBtn, &QPushButton::clicked, [this]() {
        emit joinEventRequested(eventEdit->text());
    });
}
