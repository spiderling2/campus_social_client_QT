#pragma once
#include <QWidget>
#include <QPushButton>
#include <QProgressBar>

class FileWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

signals:
    void sendFileRequested(const QString& eventName, const QString& filepath);

public slots:
    void updateProgress(int percent);

private:
    QPushButton* selectBtn;
    QProgressBar* progress;

    void setupLayout();
    void setupConnections();
};
