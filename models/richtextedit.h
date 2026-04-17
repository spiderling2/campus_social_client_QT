#pragma once
#include <QTextEdit>
#include <QMimeData>
#include <QImage>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QBuffer>
#include <QByteArray>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
class RichTextEdit : public QTextEdit {
    Q_OBJECT
public:
    using QTextEdit::QTextEdit;

    // 将文字和图片转换成 HTML（图片转 Base64）
    QString toHtmlWithImages();

    // 附件管理
    void addAttachedFile(const QString& path) { attachedFiles.append(path); }
    QList<QString> getAttachedFiles() const { return attachedFiles; }
    void clearAttachedFiles() { attachedFiles.clear(); }
protected:
    void insertFromMimeData(const QMimeData* source) override;

    QString tempDir;  // 临时文件夹路径
    QList<QString> attachedFiles;

    void ensureTempDirExists() {
        if (tempDir.isEmpty()) {
            tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + "/CampusSocialTemp";
            QDir dir(tempDir);
            if (!dir.exists())
                dir.mkpath(tempDir);
        }
    }
};
