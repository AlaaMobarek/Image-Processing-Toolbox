#include "histogramtab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QLabel>

HistogramTab::HistogramTab(QWidget *parent) : QWidget(parent)
{
    isGrayscaleMode = false;
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Toolbar logic...
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QPushButton *btnEqualize = new QPushButton("Equalize Image");
    QPushButton *btnNormalize = new QPushButton("Normalize Image");

    QString styleDarkPurple = "QPushButton { padding: 6px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #b026ff, stop:1 #7a00cc); color: white; border-radius: 5px; font-weight: bold; }";
    btnEqualize->setStyleSheet(styleDarkPurple);
    btnNormalize->setStyleSheet(styleDarkPurple);

    QSlider *modeSwitch = new QSlider(Qt::Horizontal);
    modeSwitch->setRange(0, 1); modeSwitch->setFixedSize(46, 22);
    // Add your slider styling here...

    controlsLayout->addWidget(btnEqualize);
    controlsLayout->addWidget(btnNormalize);
    controlsLayout->addStretch();
    controlsLayout->addWidget(new QLabel("RGB"));
    controlsLayout->addWidget(modeSwitch);
    controlsLayout->addWidget(new QLabel("Gray"));

    connect(btnEqualize, &QPushButton::clicked, this, &HistogramTab::onEqualizeClicked);
    connect(btnNormalize, &QPushButton::clicked, this, &HistogramTab::onNormalizeClicked);
    connect(modeSwitch, &QSlider::valueChanged, this, [this](int value) { onModeToggled(value == 1); });

    mainLayout->addLayout(controlsLayout);

    // UI Setup: Initialize labels before passing them
    rgbInputImgLabel = new ResponsiveLabel(); rgbOutputImgLabel = new ResponsiveLabel();
    histRLabel = new ResponsiveLabel(); histGLabel = new ResponsiveLabel(); histBLabel = new ResponsiveLabel();
    cdfRLabel = new ResponsiveLabel(); cdfGLabel = new ResponsiveLabel(); cdfBLabel = new ResponsiveLabel();
    grayInputImgLabel = new ResponsiveLabel(); grayOutputImgLabel = new ResponsiveLabel();
    grayHistLabel = new ResponsiveLabel(); grayCdfLabel = new ResponsiveLabel();

    stackedWidget = new QStackedWidget();

    // Page 1: RGB Layout
    pageRGB = new QWidget();
    QHBoxLayout *rgbLayout = new QHBoxLayout(pageRGB);

    QVBoxLayout *rgbCol1 = new QVBoxLayout();
    rgbCol1->addWidget(Utils::createBox("Original Input Image", rgbInputImgLabel));
    rgbCol1->addWidget(Utils::createBox("Processed Output Image", rgbOutputImgLabel));

    QVBoxLayout *rgbCol2 = new QVBoxLayout();
    rgbCol2->addWidget(Utils::createBox("Histogram (Red)", histRLabel));
    rgbCol2->addWidget(Utils::createBox("Histogram (Green)", histGLabel));
    rgbCol2->addWidget(Utils::createBox("Histogram (Blue)", histBLabel));

    QVBoxLayout *rgbCol3 = new QVBoxLayout();
    rgbCol3->addWidget(Utils::createBox("Distribution Curve (Red)", cdfRLabel));
    rgbCol3->addWidget(Utils::createBox("Distribution Curve (Green)", cdfGLabel));
    rgbCol3->addWidget(Utils::createBox("Distribution Curve (Blue)", cdfBLabel));

    rgbLayout->addLayout(rgbCol1, 2); rgbLayout->addLayout(rgbCol2, 3); rgbLayout->addLayout(rgbCol3, 3);

    // Page 2: Gray Layout
    pageGray = new QWidget();
    QGridLayout *grayLayout = new QGridLayout(pageGray);
    grayLayout->addWidget(Utils::createBox("Original Input Image", grayInputImgLabel), 0, 0);
    grayLayout->addWidget(Utils::createBox("Processed Output Image", grayOutputImgLabel), 1, 0);
    grayLayout->addWidget(Utils::createBox("Histogram (Grayscale)", grayHistLabel), 0, 1);
    grayLayout->addWidget(Utils::createBox("Distribution Curve (Grayscale)", grayCdfLabel), 1, 1);

    stackedWidget->addWidget(pageRGB); stackedWidget->addWidget(pageGray);
    mainLayout->addWidget(stackedWidget);
}

void HistogramTab::onModeToggled(bool checked) {
    isGrayscaleMode = checked;
    stackedWidget->setCurrentIndex(isGrayscaleMode ? 1 : 0);
    updateViews();
}

void HistogramTab::setSourceImage(const cv::Mat& img) {
    if (img.empty()) return;
    sourceImage = img.clone();
    processedImage = sourceImage.clone();
    updateViews();
}

// Draw chart (Cleaned up the repetitive X-axis generation logic)
cv::Mat HistogramTab::drawChart(const cv::Mat& data, cv::Scalar color, bool isCDF) {
    int width = 500, height = 220;
    cv::Mat plot = cv::Mat::zeros(height, width, CV_8UC3);
    int marginLeft = 65, marginRight = 20, marginTop = 35, marginBottom = 30;
    int plotW = width - marginLeft - marginRight;
    int plotH = height - marginTop - marginBottom;
    int font = cv::FONT_HERSHEY_SIMPLEX;

    // Outer borders
    cv::rectangle(plot, cv::Point(5, 5), cv::Point(width - 5, height - 5), cv::Scalar(60, 60, 60), 1);
    cv::line(plot, cv::Point(marginLeft, marginTop), cv::Point(marginLeft, height - marginBottom), cv::Scalar(200, 200, 200), 1);
    cv::line(plot, cv::Point(marginLeft, height - marginBottom), cv::Point(width - marginRight, height - marginBottom), cv::Scalar(200, 200, 200), 1);

    // X-Axis logic (Cleaned up DRY)
    for (int i = 0; i <= 5; ++i) {
        char buf[10]; int x;
        if (isCDF) {
            x = marginLeft + (plotW * i) / 5;
            snprintf(buf, sizeof(buf), "%.1f", i * 0.2f);
        } else {
            int val = i * 50;
            x = marginLeft + (plotW * val) / 250;
            snprintf(buf, sizeof(buf), "%d", val);
        }
        cv::putText(plot, buf, cv::Point(x - 12, height - marginBottom + 18), font, 0.35, cv::Scalar(200, 200, 200), 1);
        cv::line(plot, cv::Point(x, height - marginBottom), cv::Point(x, height - marginBottom + 4), cv::Scalar(200, 200, 200), 1);
    }

    double maxVal; cv::minMaxLoc(data, nullptr, &maxVal);
    if (maxVal == 0) maxVal = 1;
    cv::Mat normData;
    cv::normalize(data, normData, 0, plotH, cv::NORM_MINMAX);
    float binW = (float)plotW / 256.0f;

    for (int i = 1; i < 256; i++) {
        int x1 = marginLeft + cvRound(binW * (i - 1));
        int y1 = height - marginBottom - cvRound(normData.at<float>(i - 1));
        int x2 = marginLeft + cvRound(binW * i);
        int y2 = height - marginBottom - cvRound(normData.at<float>(i));

        if (isCDF) cv::line(plot, cv::Point(x1, y1), cv::Point(x2, y2), color, 2, cv::LINE_AA);
        else cv::rectangle(plot, cv::Point(x1, y1), cv::Point(x2, height - marginBottom), color, cv::FILLED);
    }
    return plot;
}

// THE BIG CLEANUP: No more repeating code 3 times for RGB!
void HistogramTab::updateViews() {
    if (sourceImage.empty() || processedImage.empty()) return;

    int histSize = 256; float range[] = { 0, 256 }; const float* histRange[] = { range };

    if (isGrayscaleMode) {
        Utils::displayImage(sourceImage, grayInputImgLabel);
        cv::Mat grayProcessed;
        if (processedImage.channels() == 3) cv::cvtColor(processedImage, grayProcessed, cv::COLOR_BGR2GRAY);
        else grayProcessed = processedImage.clone();

        Utils::displayImage(grayProcessed, grayOutputImgLabel);

        cv::Mat gray_hist, gray_cdf;
        cv::calcHist(&grayProcessed, 1, 0, cv::Mat(), gray_hist, 1, &histSize, histRange);

        gray_cdf = gray_hist.clone();
        for (int i = 1; i < histSize; i++) gray_cdf.at<float>(i) += gray_cdf.at<float>(i - 1);

        Utils::displayImage(drawChart(gray_hist, cv::Scalar(180, 180, 180), false), grayHistLabel);
        Utils::displayImage(drawChart(gray_cdf, cv::Scalar(180, 180, 180), true), grayCdfLabel);
    } else {
        Utils::displayImage(sourceImage, rgbInputImgLabel);
        Utils::displayImage(processedImage, rgbOutputImgLabel);

        cv::Mat colorWorkImg = processedImage.clone();
        if (colorWorkImg.channels() == 1) cv::cvtColor(colorWorkImg, colorWorkImg, cv::COLOR_GRAY2BGR);

        std::vector<cv::Mat> bgr_planes;
        cv::split(colorWorkImg, bgr_planes);

        // CLEAN CODE: Loop through B, G, R to generate Histograms and CDFs dynamically
        cv::Mat hist[3], cdf[3];
        cv::Scalar colors[3] = {cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255)}; // B, G, R
        ResponsiveLabel* histLabels[3] = {histBLabel, histGLabel, histRLabel};
        ResponsiveLabel* cdfLabels[3] = {cdfBLabel, cdfGLabel, cdfRLabel};

        for (int i = 0; i < 3; i++) {
            cv::calcHist(&bgr_planes[i], 1, 0, cv::Mat(), hist[i], 1, &histSize, histRange);
            Utils::displayImage(drawChart(hist[i], colors[i], false), histLabels[i]);

            cdf[i] = hist[i].clone();
            for (int j = 1; j < histSize; j++) cdf[i].at<float>(j) += cdf[i].at<float>(j - 1);
            Utils::displayImage(drawChart(cdf[i], colors[i], true), cdfLabels[i]);
        }
    }
}

void HistogramTab::onEqualizeClicked() {
    if (processedImage.empty()) return;
    if (processedImage.channels() == 1) {
        cv::equalizeHist(processedImage, processedImage);
    } else {
        cv::Mat ycrcb;
        cv::cvtColor(processedImage, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        cv::split(ycrcb, channels);
        cv::equalizeHist(channels[0], channels[0]);
        cv::merge(channels, ycrcb);
        cv::cvtColor(ycrcb, processedImage, cv::COLOR_YCrCb2BGR);
    }
    updateViews();
}

void HistogramTab::onNormalizeClicked() {
    if (processedImage.empty()) return;
    cv::normalize(processedImage, processedImage, 0, 255, cv::NORM_MINMAX);
    updateViews();
}
