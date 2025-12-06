#include "mainwindow.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFont>
#include <QFrame>
#include <QSpacerItem>
#include <QDateTime>
#include <QHBoxLayout>
#include <QApplication>
#include <QStatusBar>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Multi-Unit Converter");
    resize(820, 620);

    applyGlobalStyle();

    // Toolbar
    mainToolBar = new QToolBar("Main", this);
    mainToolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, mainToolBar);
    mainToolBar->setStyleSheet("QToolBar { spacing: 10px; padding: 6px; }");

    QAction *aboutAction = new QAction("About", this);
    mainToolBar->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, [&]() {
        QMessageBox::information(this, "About",
                                 "Multi-Unit Converter\nC++ / Qt • Live currency rates\nBuilt by Sohum");
    });

    // Main header
    QLabel *appTitle = new QLabel("Multi-Unit Converter", this);
    appTitle->setStyleSheet("font-size: 28px; font-weight: 700; color: #0B5FFF;");
    appTitle->setAlignment(Qt::AlignCenter);

    // Tabs container card
    QFrame *card = new QFrame(this);
    card->setFrameShape(QFrame::Box);
    card->setStyleSheet(
        "QFrame { background: #ffffff; border-radius: 10px; padding: 18px; }"
        );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);

    tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::North);

    // Create tabs
    setupTab(UnitCategory::Length, "Length");
    setupTab(UnitCategory::Weight, "Weight");
    setupTab(UnitCategory::Temperature, "Temperature");
    setupTab(UnitCategory::Volume, "Volume");
    setupTab(UnitCategory::Speed, "Speed");
    setupTab(UnitCategory::Currency, "Currency");

    tabWidget->setStyleSheet(
        "QTabBar::tab { min-width: 90px; padding: 8px 12px; }"
        "QTabWidget::pane { border: none; }"
        );

    cardLayout->addWidget(tabWidget);

    // Main layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(central);
    root->addWidget(appTitle);
    root->addSpacing(8);
    root->addWidget(card);

    setCentralWidget(central);

    // Status bar
    mainStatusLabel = new QLabel("Ready", this);
    statusBar()->addPermanentWidget(mainStatusLabel);

    // ---------- Network setup ----------
    networkManager = new QNetworkAccessManager(this);
    requestInProgress = false;

    refreshTimer = new QTimer(this);
    requestTimeoutTimer = new QTimer(this);

    ratesApiEndpoint = "https://api.exchangerate.host/latest";
    refreshIntervalSeconds = 3600;
    requestTimeoutMs = 8000;

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onRatesReplyFinished);

    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        fetchRates("USD");
    });

    connect(requestTimeoutTimer, &QTimer::timeout,
            this, &MainWindow::onRatesFetchTimeout);

    // Start refreshing and fetch initially
    refreshTimer->start(refreshIntervalSeconds * 1000);
    fetchRates("USD");
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* ----------------------- Styling ------------------------ */
void MainWindow::applyGlobalStyle()
{
    // Basic modern look
    QString style = R"(
        QWidget { background: #f3f6fb; font-family: "Segoe UI", Roboto, Arial; }
        QLabel { color: #333333; }
        QPushButton { background: #0B5FFF; color: white; border-radius: 6px; padding: 8px 14px; }
        QPushButton#secondary { background: #e0e6f8; color:#333; }
        QLineEdit { padding: 8px; border: 1px solid #cfd8ef; border-radius: 6px; background: white; }
        QComboBox { padding: 6px 8px; border: 1px solid #cfd8ef; border-radius: 6px; background: white; }
        QLabel#bigResult { font-size: 20px; font-weight: 700; color: #0B5FFF; }
    )";
    qApp->setStyleSheet(style);
}

/* ------------------------ Tabs -------------------------- */
void MainWindow::setupTab(UnitCategory category, const QString &title)
{
    TabWidgets tw;
    tw.tab = new QWidget;

    // Title inside tab
    QLabel *tabHeader = new QLabel(title + " Conversion", tw.tab);
    tabHeader->setStyleSheet("font-size: 20px; font-weight: 600; color: #222;");
    tabHeader->setAlignment(Qt::AlignCenter);

    // From/To grid
    tw.cmbUnitFrom = new QComboBox;
    tw.cmbUnitTo = new QComboBox;

    tw.lnEdtInput = new QLineEdit;
    tw.lnEdtInput->setPlaceholderText("Enter value to convert");

    tw.lblOutputResult = new QLabel("Result: 0");
    tw.lblOutputResult->setObjectName("bigResult");
    tw.lblOutputResult->setAlignment(Qt::AlignCenter);

    tw.btnSubmit = new QPushButton("Convert");
    tw.btnReverse = new QPushButton("Reverse");
    tw.btnReverse->setObjectName("secondary");

    // Populate units (Units singleton)
    Units::getInstance().populateUnits(tw.cmbUnitFrom, category);
    Units::getInstance().populateUnits(tw.cmbUnitTo, category);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabHeader);

    QGridLayout *grid = new QGridLayout;
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(12);

    QLabel *lblFrom = new QLabel("From:");
    QLabel *lblTo = new QLabel("To:");

    grid->addWidget(lblFrom, 0, 0, Qt::AlignRight);
    grid->addWidget(tw.cmbUnitFrom, 0, 1);
    grid->addWidget(lblTo, 1, 0, Qt::AlignRight);
    grid->addWidget(tw.cmbUnitTo, 1, 1);

    layout->addLayout(grid);

    // Input area centered
    QHBoxLayout *inputRow = new QHBoxLayout;
    inputRow->addStretch();
    inputRow->addWidget(tw.lnEdtInput);
    inputRow->addStretch();
    layout->addLayout(inputRow);

    // Special: Speed tab gets ETA inputs
    if (category == UnitCategory::Speed) {
        QLabel *etaHeader = new QLabel("ETA Calculator (distance in meters)", tw.tab);
        etaHeader->setAlignment(Qt::AlignCenter);
        etaHeader->setStyleSheet("color:#666; font-size:13px;");
        layout->addWidget(etaHeader);

        tw.lnEdtDistance = new QLineEdit;
        tw.lnEdtDistance->setPlaceholderText("Enter distance (m)");

        tw.lblEta = new QLabel("ETA: -");
        tw.lblEta->setAlignment(Qt::AlignCenter);
        tw.lblEta->setStyleSheet("color:#D35400; font-weight:600;");

        QHBoxLayout *etaRow = new QHBoxLayout;
        etaRow->addStretch();
        etaRow->addWidget(tw.lnEdtDistance);
        etaRow->addStretch();
        layout->addLayout(etaRow);
        layout->addWidget(tw.lblEta);
    }

    // Currency: add rates status row
    if (category == UnitCategory::Currency) {
        QHBoxLayout *ratesRow = new QHBoxLayout;
        ratesRow->setContentsMargins(0, 10, 0, 0);

        tw.lblRatesStatus = new QLabel("Rates: updating...", tw.tab);
        tw.lblRatesStatus->setStyleSheet("color:#555; font-size:12px;");

        tw.ratesProgress = new QProgressBar(tw.tab);
        tw.ratesProgress->setFixedSize(140, 14);
        tw.ratesProgress->setTextVisible(false);
        tw.ratesProgress->setRange(0, 0); // busy indicator

        ratesRow->addStretch();
        ratesRow->addWidget(tw.lblRatesStatus);
        ratesRow->addSpacing(8);
        ratesRow->addWidget(tw.ratesProgress);
        ratesRow->addStretch();

        layout->addLayout(ratesRow);

        // Initially disable currency convert until rates load
        tw.btnSubmit->setEnabled(false);
    }

    // Buttons and result
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(tw.btnSubmit);
    btnRow->addSpacing(12);
    btnRow->addWidget(tw.btnReverse);
    btnRow->addStretch();

    layout->addLayout(btnRow);
    layout->addSpacing(6);
    layout->addWidget(tw.lblOutputResult);

    tw.tab->setLayout(layout);
    tabWidget->addTab(tw.tab, title);
    tabs[category] = tw;

    // Connect signals
    connect(tw.btnSubmit, &QPushButton::clicked, this, &MainWindow::convertUnits);
    connect(tw.btnReverse, &QPushButton::clicked, this, &MainWindow::reverseConversion);
}

/* --------------------- Conversion ----------------------- */
void MainWindow::convertUnits()
{
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    auto &tw = tabs[currentCategory];

    bool ok;
    double value = tw.lnEdtInput->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Enter a valid number");
        return;
    }

    // Currency special-case
    if (currentCategory == UnitCategory::Currency) {
        double directRate = 0.0;
        if (Units::getInstance().getCurrencyRate(tw.cmbUnitFrom->currentText(),
                                                 tw.cmbUnitTo->currentText(), directRate)) {
            double result = value * directRate;
            tw.lblOutputResult->setText("Result: " + QString::number(result, 'f', 4));
            // update status
            mainStatusLabel->setText(QString("Converted %1 %2 → %3")
                                         .arg(value).arg(tw.cmbUnitFrom->currentText()).arg(tw.cmbUnitTo->currentText()));
            return;
        }

        // If no rate available yet, start fetch and notify user
        if (!requestInProgress) fetchRates("USD");
        QMessageBox::information(this, "Please wait", "Currency rates are updating. Try again in a moment.");
        return;
    }

    // Normal conversions
    double result = Units::getInstance().convert(
        tw.cmbUnitFrom->currentText(),
        tw.cmbUnitTo->currentText(),
        value
        );

    tw.lblOutputResult->setText("Result: " + QString::number(result));
    if (currentCategory == UnitCategory::Speed) calculateETA(tw);
    mainStatusLabel->setText("Converted locally");
}

/* --------------------- Reverse -------------------------- */
void MainWindow::reverseConversion()
{
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    auto &tw = tabs[currentCategory];

    QString temp = tw.cmbUnitFrom->currentText();
    tw.cmbUnitFrom->setCurrentText(tw.cmbUnitTo->currentText());
    tw.cmbUnitTo->setCurrentText(temp);

    convertUnits();
}

/* ------------------- Update Units ----------------------- */
void MainWindow::updateUnits()
{
    UnitCategory currentCategory = static_cast<UnitCategory>(tabWidget->currentIndex());
    Units::getInstance().populateUnits(tabs[currentCategory].cmbUnitTo, currentCategory);
}

/* ---------------------- ETA ----------------------------- */
void MainWindow::calculateETA(TabWidgets &tw)
{
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

    tw.lblEta->setText(QString("ETA: %1h %2m %3s").arg(hours).arg(minutes).arg(seconds));
}

/* --------------------- Networking ----------------------- */
void MainWindow::fetchRates(const QString &baseCurrency)
{
    if (requestInProgress) return;

    requestInProgress = true;
    updateCurrencyStatus("Fetching rates...", true);
    setCurrencyControlsEnabled(false);

    QUrl url(ratesApiEndpoint);
    QUrlQuery q;
    q.addQueryItem("base", baseCurrency);
    url.setQuery(q);

    QNetworkRequest request(url);
    networkManager->get(request);
    requestTimeoutTimer->start(requestTimeoutMs);
}

void MainWindow::onRatesReplyFinished(QNetworkReply *reply)
{
    requestTimeoutTimer->stop();
    requestInProgress = false;

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        updateCurrencyStatus("Failed to update rates", false);
        setCurrencyControlsEnabled(true); // still enable to allow user try cached conversions
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        updateCurrencyStatus("Invalid rates data", false);
        setCurrencyControlsEnabled(true);
        return;
    }

    QJsonObject obj = doc.object();
    QString base = obj.contains("base") ? obj["base"].toString() : "USD";
    QJsonObject ratesObj = obj["rates"].toObject();
    if (ratesObj.isEmpty()) {
        updateCurrencyStatus("Rates empty", false);
        setCurrencyControlsEnabled(true);
        return;
    }

    // Build rates map
    QMap<QString, double> ratesMap;
    ratesMap[base] = 1.0;
    for (auto it = ratesObj.begin(); it != ratesObj.end(); ++it) {
        ratesMap[it.key()] = it.value().toDouble();
    }

    // Populate Units with pairwise rates
    QList<QString> keys = ratesMap.keys();
    for (const QString &a : keys) {
        for (const QString &b : keys) {
            double ra = ratesMap.value(a, 0.0);
            double rb = ratesMap.value(b, 0.0);
            if (ra > 0.0 && rb > 0.0) {
                Units::getInstance().setCurrencyRate(a, b, rb / ra);
            }
        }
    }

    lastRatesUpdate = QDateTime::currentDateTimeUtc();
    updateCurrencyStatus("Rates updated • " + lastRatesUpdate.toLocalTime().toString("hh:mm:ss"), false);
    setCurrencyControlsEnabled(true);
    mainStatusLabel->setText("Rates updated");
}

void MainWindow::onRatesFetchTimeout()
{
    requestInProgress = false;
    updateCurrencyStatus("Fetch timed out", false);
    setCurrencyControlsEnabled(true);
}

/* -------------------- UI helpers ------------------------ */
void MainWindow::updateCurrencyStatus(const QString &text, bool busy)
{
    auto it = tabs.find(UnitCategory::Currency);
    if (it == tabs.end()) return;
    TabWidgets &ctw = it->second;
    if (ctw.lblRatesStatus) ctw.lblRatesStatus->setText(text);
    if (ctw.ratesProgress) {
        if (busy) { ctw.ratesProgress->setRange(0, 0); ctw.ratesProgress->show(); }
        else { ctw.ratesProgress->setRange(0, 1); ctw.ratesProgress->setValue(1); }
    }
}

void MainWindow::setCurrencyControlsEnabled(bool enabled)
{
    auto it = tabs.find(UnitCategory::Currency);
    if (it == tabs.end()) return;
    TabWidgets &ctw = it->second;
    if (ctw.btnSubmit) ctw.btnSubmit->setEnabled(enabled);
}
