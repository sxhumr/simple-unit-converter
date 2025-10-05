#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <unordered_map>
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

private:
    Ui::MainWindow *ui;

    QTabWidget *tabWidget;

    struct TabWidgets {
        QWidget *tab;
        QComboBox *cmbUnitFrom;
        QComboBox *cmbUnitTo;
        QLineEdit *lnEdtInput;
        QLabel *lblOutputResult;
        QPushButton *btnSubmit;
        QPushButton *btnReverse;

        QLineEdit *lnEdtDistance; // For ETA
        QLabel *lblEta;
    };

    std::unordered_map<UnitCategory, TabWidgets> tabs;

    void setupTab(UnitCategory category, const QString &title);
    void calculateETA(TabWidgets &tw);
};

#endif // MAINWINDOW_H
