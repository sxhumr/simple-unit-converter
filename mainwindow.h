#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <unordered_map>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QProgressBar>
#include <QToolBar>
#include <QDateTime>

#include "units.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void convertUnits();
    void reverseConversion();
    void updateUnits();

    // Network
    void onRatesReplyFinished(QNetworkReply *reply);
    void onRatesFetchTimeout();

private:
    Ui::MainWindow *ui;

    QTabWidget *tabWidget;

    struct TabWidgets {
        QWidget *tab = nullptr;
        QComboBox *cmbUnitFrom = nullptr;
        QComboBox *cmbUnitTo = nullptr;
        QLineEdit *lnEdtInput = nullptr;
        QLabel *lblOutputResult = nullptr;
        QPushButton *btnSubmit = nullptr;
        QPushButton *btnReverse = nullptr;

        // Speed
        QLineEdit *lnEdtDistance = nullptr;
        QLabel *lblEta = nullptr;

        // Currency small UI pieces
        QLabel *lblRatesStatus = nullptr;
        QProgressBar *ratesProgress = nullptr;
    };

    std::unordered_map<UnitCategory, TabWidgets> tabs;

    void setupTab(UnitCategory category, const QString &title);
    void calculateETA(TabWidgets &tw);

    // Currency API
    QNetworkAccessManager *networkManager = nullptr;
    QTimer *refreshTimer = nullptr;
    QTimer *requestTimeoutTimer = nullptr;

    QString ratesApiEndpoint;
    int refreshIntervalSeconds;
    int requestTimeoutMs;
    bool requestInProgress;

    void fetchRates(const QString &baseCurrency = "USD");

    // UI helpers
    void applyGlobalStyle();
    void updateCurrencyStatus(const QString &text, bool busy = false);
    void setCurrencyControlsEnabled(bool enabled);
    QToolBar *mainToolBar = nullptr;
    QLabel *mainStatusLabel = nullptr;
    QDateTime lastRatesUpdate;
};

#endif // MAINWINDOW_H
