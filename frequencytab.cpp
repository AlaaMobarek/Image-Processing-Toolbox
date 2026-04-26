#include "frequencytab.h"
#include <QVBoxLayout>
#include <QGridLayout>

FrequencyTab::FrequencyTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    img1Label = new ResponsiveLabel();
    img2Label = new ResponsiveLabel();
    hybridLabel = new ResponsiveLabel();

    lowPassSlider = new QSlider(Qt::Horizontal);
    lowPassSlider->setRange(1, 50); lowPassSlider->setValue(15);

    highPassSlider = new QSlider(Qt::Horizontal);
    highPassSlider->setRange(1, 50); highPassSlider->setValue(15);

    connect(lowPassSlider, &QSlider::valueChanged, this, &FrequencyTab::updateHybridImage);
    connect(highPassSlider, &QSlider::valueChanged, this, &FrequencyTab::updateHybridImage);

    QGridLayout *gridLayout = new QGridLayout();

    // Container 1
    QWidget *container1 = new QWidget();
    QVBoxLayout *col1Layout = new QVBoxLayout(container1);
    col1Layout->setContentsMargins(0, 0, 0, 0);
    col1Layout->addWidget(new QLabel("Low-Pass Filter Radius:"));
    col1Layout->addWidget(lowPassSlider);
    col1Layout->addWidget(img1Label, 1);

    // Container 2
    QWidget *container2 = new QWidget();
    QVBoxLayout *col2Layout = new QVBoxLayout(container2);
    col2Layout->setContentsMargins(0, 0, 0, 0);
    col2Layout->addWidget(new QLabel("High-Pass Filter Radius:"));
    col2Layout->addWidget(highPassSlider);
    col2Layout->addWidget(img2Label, 1);

    // Using Shared Utils to create boxes
    gridLayout->addWidget(Utils::createBox("Image 1 (Low Freq)", container1), 0, 0);
    gridLayout->addWidget(Utils::createBox("Image 2 (High Freq)", container2), 1, 0);
    gridLayout->addWidget(Utils::createBox("Hybrid Image Result", hybridLabel), 0, 1, 2, 1);

    gridLayout->setRowStretch(0, 1); gridLayout->setRowStretch(1, 1);
    gridLayout->setColumnStretch(0, 1); gridLayout->setColumnStretch(1, 2);

    mainLayout->addLayout(gridLayout);
}

void FrequencyTab::setImage1(const cv::Mat& img) {
    if (img.empty()) return;
    image1 = img.clone();
    Utils::displayImage(image1, img1Label);
    updateHybridImage();
}

void FrequencyTab::setImage2(const cv::Mat& img) {
    if (img.empty()) return;
    image2 = img.clone();
    Utils::displayImage(image2, img2Label);
    updateHybridImage();
}

void FrequencyTab::updateHybridImage() {
    if (image1.empty() || image2.empty()) return;

    cv::Mat img1_resized, img2_resized;
    cv::resize(image1, img1_resized, cv::Size(500, 500));
    cv::resize(image2, img2_resized, cv::Size(500, 500));

    img1_resized.convertTo(img1_resized, CV_32F, 1.0 / 255.0);
    img2_resized.convertTo(img2_resized, CV_32F, 1.0 / 255.0);

    cv::Mat lowPassImg, blurImg2, highPassImg, hybrid;
    int ksize1 = lowPassSlider->value() * 2 + 1;
    cv::GaussianBlur(img1_resized, lowPassImg, cv::Size(ksize1, ksize1), 0);

    int ksize2 = highPassSlider->value() * 2 + 1;
    cv::GaussianBlur(img2_resized, blurImg2, cv::Size(ksize2, ksize2), 0);

    highPassImg = img2_resized - blurImg2;
    hybrid = lowPassImg + highPassImg;
    cv::Mat lowDisplay ,highDisplay, hybridDisplay;

    lowPassImg.convertTo(lowDisplay, CV_8UC3, 255.0);
    Utils::displayImage(lowDisplay, img1Label);

    cv::normalize(highPassImg, highDisplay, 0, 255.0, cv::NORM_MINMAX);
    highDisplay.convertTo(highDisplay, CV_8UC3);
    Utils::displayImage(highDisplay, img2Label);

    cv::normalize(hybrid, hybridDisplay, 0, 255.0, cv::NORM_MINMAX);
    hybridDisplay.convertTo(hybridDisplay, CV_8UC3);

    Utils::displayImage(hybridDisplay, hybridLabel);
}
