#include "gui/mainwindow.h"
#include <QFile>
#include <QApplication>
#include  "service/userservice.h"
#include <windows.h>
int main(int argc, char *argv[])
{
    // 设置控制台为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    QApplication app(argc, argv);

    // 加载 QSS 样式表
    // 加载 QSS 文件
    QFile styleFile(":/styles/styles.qss"); // 或者 "styles/style.qss" 如果是本地文件
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
    } else {
        qDebug() << "QSS 文件加载失败";
    }

    MainWindow w;
    w.show();
    return app.exec();
}
