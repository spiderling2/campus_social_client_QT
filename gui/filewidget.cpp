#include "filewidget.h"
#include <QHBoxLayout>
#include <QFileDialog>

FileWidget::FileWidget(QWidget* parent) : QWidget(parent) {
    selectBtn = new QPushButton("Send File", this);
    progress = new QProgressBar(this);

    setupLayout();
    setupConnections();
}

void FileWidget::setupLayout() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(selectBtn);
    layout->addWidget(progress);
}

void FileWidget::setupConnections() {
    connect(selectBtn, &QPushButton::clicked, [this]() {
        QString filepath = QFileDialog::getOpenFileName(this, "Select File");
        if (!filepath.isEmpty()) {
            emit sendFileRequested("default_event", filepath);
        }
    });
}

void FileWidget::updateProgress(int percent) {
    progress->setValue(percent);
}
