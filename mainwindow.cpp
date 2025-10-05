#include "mainwindow.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(650, 550);
    setWindowTitle("📏 Multi-Unit Converter");

    tabWidget = new QTabWidget(this);

    setupTab(UnitCategory::Length, "Length");
    setupTab(UnitCategory::Weight, "Weight");
    setupTab(UnitCategory::Temperature, "Temperature");
    setupTab(UnitCategory::Volume, "Volume");
    setupTab(UnitCategory::Speed, "Speed");

    QLabel *mainHeader = new QLabel("🌟 Multi-Unit Converter", this);
    mainHeader->setStyleSheet("font-size: 24px; font-weight: bold; color: #0078D7;");
    mainHeader->setAlignment(Qt::AlignCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mainHeader);
    mainLayout->addWidget(tabWidget);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupTab(UnitCategory category, const QString &title) {
    TabWidgets tw;
    tw.tab = new QWidget;

    QLabel *tabHeader = new QLabel(title + " Conversion", tw.tab);
    tabHeader->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
    tabHeader->setAlignment(Qt::AlignCenter);

    tw.cmbUnitFrom = new QComboBox;
    tw.cmbUnitTo = new QComboBox;

    tw.lnEdtInput = new QLineEdit;
    tw.lnEdtInput->setPlaceholderText("Enter value to convert");
    tw.lnEdtInput->setFixedSize(220, 35);

    tw.lblOutputResult = new QLabel("Result: ");
    tw.lblOutputResult->setStyleSheet("font-size: 16px; font-weight: bold; color: #0078D7;");
    tw.lblOutputResult->setAlignment(Qt::AlignCenter);

    tw.btnSubmit = new QPushButton("Convert");
    tw.btnReverse = new QPushButton("Reverse");

    Units::getInstance().populateUnits(tw.cmbUnitFrom, category);
    Units::getInstance().populateUnits(tw.cmbUnitTo, category);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabHeader);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel("From:"), 0, 0);
    grid->addWidget(tw.cmbUnitFrom, 0, 1);
    grid->addWidget(new QLabel("To:"), 1, 0);
    grid->addWidget(tw.cmbUnitTo, 1, 1);
    layout->addLayout(grid);

    layout->addWidget(tw.lnEdtInput, 0, Qt::AlignCenter);

    if (category == UnitCategory::Speed) {
        QLabel *etaHeader = new QLabel("📍 ETA Calculator", tw.tab);
        etaHeader->setStyleSheet("font-size: 18px; font-weight: bold; color: #005A9E;");
        etaHeader->setAlignment(Qt::AlignCenter);

        QLabel *etaInfo = new QLabel("Distance in meters and speed in chosen unit", tw.tab);
        etaInfo->setStyleSheet("font-size: 12px; color: #555;");
        etaInfo->setAlignment(Qt::AlignCenter);

        tw.lnEdtDistance = new QLineEdit;
        tw.lnEdtDistance->setPlaceholderText("Enter distance (m)");
        tw.lnEdtDistance->setFixedSize(220, 35);

        tw.lblEta = new QLabel("ETA: ");
        tw.lblEta->setStyleSheet("font-size: 14px; font-weight: bold; color: #D35400;");
        tw.lblEta->setAlignment(Qt::AlignCenter);

        QVBoxLayout *etaLayout = new QVBoxLayout;
        etaLayout->addWidget(etaHeader);
        etaLayout->addWidget(etaInfo);
        etaLayout->addWidget(tw.lnEdtDistance);
        etaLayout->addWidget(tw.lblEta);
        etaLayout->setSpacing(5);

        layout->addLayout(etaLayout);
    }

    layout->addWidget(tw.btnSubmit, 0, Qt::AlignCenter);
    layout->addWidget(tw.btnReverse, 0, Qt::AlignCenter);
    layout->addWidget(tw.lblOutputResult, 0, Qt::AlignCenter);

    tw.tab->setLayout(layout);

    tabWidget->addTab(tw.tab, title);
    tabs[category] = tw;

    connect(tw.btnSubmit, &QPushButton::clicked, this, &MainWindow::convertUnits);
    connect(tw.btnReverse, &QPushButton::clicked, this, &MainWindow::reverseConversion);
}

void MainWindow::convertUnits() {
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    auto &tw = tabs[currentCategory];

    bool ok;
    double value = tw.lnEdtInput->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Enter a valid number");
        return;
    }

    double result = Units::getInstance().convert(tw.cmbUnitFrom->currentText(),
                                                 tw.cmbUnitTo->currentText(),
                                                 value);
    tw.lblOutputResult->setText("Result: " + QString::number(result));

    if (currentCategory == UnitCategory::Speed) {
        calculateETA(tw);
    }
}

void MainWindow::reverseConversion() {
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    auto &tw = tabs[currentCategory];

    QString temp = tw.cmbUnitFrom->currentText();
    tw.cmbUnitFrom->setCurrentText(tw.cmbUnitTo->currentText());
    tw.cmbUnitTo->setCurrentText(temp);

    convertUnits();
}

void MainWindow::updateUnits() {
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    Units::getInstance().populateUnits(tabs[currentCategory].cmbUnitTo, currentCategory);
}

void MainWindow::calculateETA(TabWidgets &tw) {
    bool okDistance;
    double distance = tw.lnEdtDistance->text().toDouble(&okDistance);
    if (!okDistance) {
        tw.lblEta->setText("ETA: ❌ Invalid distance");
        return;
    }

    bool okSpeed;
    double speed = tw.lnEdtInput->text().toDouble(&okSpeed);
    if (!okSpeed) {
        tw.lblEta->setText("ETA: ❌ Invalid speed");
        return;
    }

    if (speed == 0) {
        tw.lblEta->setText("ETA: ∞ (Speed is zero)");
        return;
    }

    double etaSeconds = distance / speed;
    int hours = static_cast<int>(etaSeconds) / 3600;
    int minutes = (static_cast<int>(etaSeconds) % 3600) / 60;
    int seconds = static_cast<int>(etaSeconds) % 60;

    tw.lblEta->setText(
        QString("ETA: %1h %2m %3s")
            .arg(hours)
            .arg(minutes)
            .arg(seconds)
        );
}
