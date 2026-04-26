#ifndef UTILS_H
#define UTILS_H

#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QPainter>
#include <opencv2/opencv.hpp>

// ==========================================
// 1. Unified Responsive Label
// Replaces FreqResponsiveLabel and ResponsiveLabel
// ==========================================
class ResponsiveLabel : public QLabel {
    Q_OBJECT
public:
    explicit ResponsiveLabel(QWidget *parent = nullptr) : QLabel(parent) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumSize(100, 100);
    }

    void setOriginalPixmap(const QPixmap &pm) {
        originalPixmap = pm;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        if (originalPixmap.isNull()) {
            QLabel::paintEvent(event);
            return;
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPixmap scaled = originalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    }
private:
    QPixmap originalPixmap;
};

// ==========================================
// 2. Shared Utilities Helper Class
// Contains all repeated logic (DRY Principle)
// ==========================================
class Utils {
public:
    // Helper to wrap widgets in a styled frame with a title
    static QWidget* createBox(const QString& title, QWidget* content) {
        QFrame* frame = new QFrame();
        frame->setStyleSheet("QFrame { background-color: #2b2b2e; border: 1px solid #555; border-radius: 6px; margin: 2px; }"
                             "QLabel { border: none; background: transparent; }");

        QVBoxLayout* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(10, 8, 10, 10);

        QLabel* titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #9370db;");
        layout->addWidget(titleLabel);

        QFrame* hline = new QFrame();
        hline->setFrameShape(QFrame::HLine);
        hline->setStyleSheet("background-color: #555; max-height: 1px; margin-bottom: 5px;");
        layout->addWidget(hline);

        layout->addWidget(content);
        return frame;
    }

    // Unified displayImage for OpenCV Mat to ResponsiveLabel
    static void displayImage(const cv::Mat& img, ResponsiveLabel* label) {
        if (img.empty()) return;
        cv::Mat rgbMat;
        if (img.channels() == 3) cv::cvtColor(img, rgbMat, cv::COLOR_BGR2RGB);
        else cv::cvtColor(img, rgbMat, cv::COLOR_GRAY2RGB);

        QImage qImg(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
        label->setOriginalPixmap(QPixmap::fromImage(qImg));
    }

    // Standard displayImage for regular QLabel (Used in MainWindow)
    static void displayImageStandard(const cv::Mat& img, QLabel* label) {
        if (img.empty()) return;
        cv::Mat rgbMat;
        if (img.channels() == 3) cv::cvtColor(img, rgbMat, cv::COLOR_BGR2RGB);
        else cv::cvtColor(img, rgbMat, cv::COLOR_GRAY2RGB);

        QImage qImg(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
        label->setPixmap(QPixmap::fromImage(qImg).scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
};

#endif // UTILS_H
