#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QTranslator>
#include <vector>

#include "process.h"
#include "scheduling_controller.h"
#include "storage_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAlgorithmChanged(int index);
    void onAddProcess();
    void onRemoveSelected();
    void onStartScheduling();
    void onClearAll();
    void onSaveCSV();
    void onLoadCSV();
    void showManual();
    void showAbout();

private:
    Ui::MainWindow *ui;
    QTranslator translator;

    SchedulingController controller;
    ScheduleOutcome lastOutcome;

    StorageManager storage;

    void refreshProcessTable();
    std::vector<Process> readProcessesFromUI() const;

    void fillResultsTable(const ScheduleOutcome &outcome);
    void buildTimeline(const ScheduleOutcome &outcome);

    void setupConnections();
    void setupTables();

    void showAlgorithmSpecificFields();
    void loadLanguage(const QString &langCode);
};

#endif // MAINWINDOW_H
