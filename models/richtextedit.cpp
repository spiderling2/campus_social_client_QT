#include"richtextedit.h"

QString RichTextEdit::toHtmlWithImages() {
    QTextDocument* doc = this->document();
    QString html = doc->toHtml();

    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    while (!cursor.atEnd()) {
        QTextBlock block = cursor.block();
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;

            QTextCharFormat fmt = frag.charFormat();
            if (fmt.isImageFormat()) {
                QTextImageFormat imgFmt = fmt.toImageFormat();
                QString imgName = imgFmt.name();
                QVariant var = doc->resource(QTextDocument::ImageResource, QUrl(imgName));
                if (var.canConvert<QImage>()) {
                    QImage img = var.value<QImage>();
                    QByteArray ba;
                    QBuffer buffer(&ba);
                    buffer.open(QIODevice::WriteOnly);
                    img.save(&buffer, "PNG");
                    QString imgBase64 = QString("data:image/png;base64,%1").arg(QString(ba.toBase64()));
                    // 替换 HTML 中图片引用
                    html.replace(imgName, imgBase64);
                }
            }
        }
        cursor.movePosition(QTextCursor::NextBlock);
    }

    return html;
}


void RichTextEdit::insertFromMimeData(const QMimeData* source) {
    ensureTempDirExists();
    QTextCursor cursor = this->textCursor();

    if (source->hasText() && !source->hasImage() && !source->hasUrls()) {
        cursor.insertText(source->text());
    }
    else if (source->hasImage()) {
        QImage img = qvariant_cast<QImage>(source->imageData());
        if (!img.isNull()) {
            QString tmpPath = tempDir + QString("/pasted_%1.png")
            .arg(QDateTime::currentMSecsSinceEpoch());
            img.save(tmpPath, "PNG");

            QUrl imgUrl = QUrl::fromLocalFile(tmpPath);
            this->document()->addResource(QTextDocument::ImageResource, imgUrl, img);
            cursor.insertImage(imgUrl.toString());

            addAttachedFile(tmpPath);
        }
    }
    else if (source->hasUrls()) {
        for (const QUrl &url : source->urls()) {
            QString filePath = QString::fromUtf8(url.toLocalFile().toUtf8());
            if (filePath.isEmpty()) continue;

            addAttachedFile(filePath);

            if (filePath.endsWith(".mp4") || filePath.endsWith(".avi") || filePath.endsWith(".mov"))
                cursor.insertText(QString("[视频: %1]").arg(filePath));
            else if (filePath.endsWith(".mp3") || filePath.endsWith(".wav"))
                cursor.insertText(QString("[音频: %1]").arg(filePath));
            else
                cursor.insertText(QString("[文件: %1]").arg(filePath));
        }
    }
    else {
        QTextEdit::insertFromMimeData(source);
    }

    this->setTextCursor(cursor);
}
