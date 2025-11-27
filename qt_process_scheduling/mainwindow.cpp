#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "manual_dialog.h"
#include "about_dialog.h"
#include "storage_manager.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QHeaderView>
#include <QRegularExpression>

#include <fstream>
#include <sstream>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupConnections();
    setupTables();

    ui->algorithmComboBox->setCurrentIndex(0);
    onAlgorithmChanged(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

// ---------------------------------------------------------
// Conexões e tabelas
// ---------------------------------------------------------

void MainWindow::setupConnections() {
    connect(ui->addProcessPushButton, &QPushButton::clicked, this, &MainWindow::onAddProcess);
    connect(ui->removeProcessPushButton, &QPushButton::clicked, this, &MainWindow::onRemoveSelected);
    connect(ui->startPushButton, &QPushButton::clicked, this, &MainWindow::onStartScheduling);
    connect(ui->limparPushButton, &QPushButton::clicked, this, &MainWindow::onClearAll);
    connect(ui->algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);

    connect(ui->actionImportar, &QAction::triggered, this, &MainWindow::onLoadCSV);
    connect(ui->actionExportar, &QAction::triggered, this, &MainWindow::onSaveCSV);
    connect(ui->actionManual, &QAction::triggered, this, &MainWindow::showManual);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);

    connect(ui->actionPortugues, &QAction::triggered, this, [=](){loadLanguage("pt");});
    connect(ui->actionIngles, &QAction::triggered, this, [=](){loadLanguage("en");});

}

void MainWindow::setupTables() {
    ui->processesTable->setColumnCount(4);
    ui->processesTable->setHorizontalHeaderLabels({tr("Nome"), tr("Chegada"), tr("Execução"), tr("Prioridade")});
    ui->processesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->processesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->processesTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->resultsTable->setColumnCount(5);
    ui->resultsTable->setHorizontalHeaderLabels({tr("PID"), tr("Início"), tr("Término"), tr("Espera"), tr("Turnaround")});
    ui->resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->diagramTable->setRowCount(2);
    ui->diagramTable->setVerticalHeaderLabels({tr("Tempo"), tr("Processo")});
    ui->diagramTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->diagramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

// ---------------------------------------------------------
// Mudar algoritmo (Priority / RR)
// ---------------------------------------------------------

void MainWindow::onAlgorithmChanged(int index) {
    Q_UNUSED(index);
    showAlgorithmSpecificFields();
}

void MainWindow::showAlgorithmSpecificFields() {
    QString alg = ui->algorithmComboBox->currentText().trimmed();

    bool isPriority = (alg == "Priority");
    bool isRR = (alg == "Round Robin");

    ui->priorityLabel->setVisible(false);
    ui->prioritySpinBox->setVisible(false);
    ui->prioritySpinBox->setEnabled(false);

    ui->quantumLabel->setVisible(false);
    ui->quantumSpinBox->setVisible(false);
    ui->quantumSpinBox->setEnabled(false);

    if (ui->algorithmStackedWidget) {
        ui->algorithmStackedWidget->setVisible(false);
    }

    ui->processesTable->setColumnHidden(3, true);

    if (isPriority) {
        ui->priorityLabel->setVisible(true);
        ui->prioritySpinBox->setVisible(true);
        ui->prioritySpinBox->setEnabled(true);

        if (ui->algorithmStackedWidget) {
            ui->algorithmStackedWidget->setCurrentIndex(0);
            ui->algorithmStackedWidget->setVisible(true);
        }

        ui->processesTable->setColumnHidden(3, false);
    }
    else if (isRR) {
        ui->quantumLabel->setVisible(true);
        ui->quantumSpinBox->setVisible(true);
        ui->quantumSpinBox->setEnabled(true);

        if (ui->algorithmStackedWidget) {
            ui->algorithmStackedWidget->setCurrentIndex(1);
            ui->algorithmStackedWidget->setVisible(true);
        }

        ui->processesTable->setColumnHidden(3, true);
    }
    else {
        ui->processesTable->setColumnHidden(3, true);
    }

    ui->diagramTable->setVisible(true);
}


// ---------------------------------------------------------
// Ler processos da interface
// ---------------------------------------------------------

std::vector<Process> MainWindow::readProcessesFromUI() const {
    std::vector<Process> list;
    int rows = ui->processesTable->rowCount();

    for (int i = 0; i < rows; ++i) {
        auto *nameItem = ui->processesTable->item(i, 0);
        auto *arrItem = ui->processesTable->item(i, 1);
        auto *burstItem = ui->processesTable->item(i, 2);
        auto *prioItem = ui->processesTable->item(i, 3);

        if (!nameItem || !arrItem || !burstItem) continue;

        list.emplace_back(
            nameItem->text().toStdString(),
            arrItem->text().toInt(),
            burstItem->text().toInt(),
            prioItem ? prioItem->text().toInt() : 0
        );
    }

    return list;
}

// ---------------------------------------------------------
// Atualizar tabela
// ---------------------------------------------------------

void MainWindow::refreshProcessTable() {
    const auto &procs = controller.processes();

    ui->processesTable->blockSignals(true);
    ui->processesTable->setRowCount(procs.size());

    for (int i = 0; i < static_cast<int>(procs.size()); ++i) {
        const auto &p = procs[i];

        ui->processesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p.name())));
        ui->processesTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.arrival())));
        ui->processesTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.burst())));
        ui->processesTable->setItem(i, 3, new QTableWidgetItem(QString::number(p.priority())));
    }

    ui->processesTable->blockSignals(false);
}

// ---------------------------------------------------------
// Botões (Add / Remove / Start / Clear)
// ---------------------------------------------------------

void MainWindow::onAddProcess() {
    QString name = ui->processNameInput->text();
    int arrival = ui->arrivalTimeInput->value();
    int burst = ui->executionTimeInput->value();
    int prio = ui->prioritySpinBox->value();

    if (name.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Erro"), tr("O nome do processo não pode ser vazio."));
        return;
    }

    auto procs = controller.processes();
    for (const auto &p : procs) {
        if (p.name() == name.toStdString()) {
            QMessageBox::warning(this, tr("Erro"), tr("Já existe um processo com esse nome."));
            return;
        }
    }

    procs.emplace_back(name.toStdString(), arrival, burst, prio);
    controller.setProcesses(procs);
    refreshProcessTable();

    ui->processNameInput->clear();
    ui->arrivalTimeInput->setValue(0);
    ui->executionTimeInput->setValue(1);
    ui->prioritySpinBox->setValue(1);
}

void MainWindow::onRemoveSelected() {
    auto sel = ui->processesTable->selectionModel()->selectedRows();
    if (sel.empty()) {
        QMessageBox::information(this, tr("Remover"), tr("Selecione um processo para remover."));
        return;
    }

    int row = sel.first().row();
    auto procs = controller.processes();

    if (row >= 0 && row < static_cast<int>(procs.size())) {
        procs.erase(procs.begin() + row);
        controller.setProcesses(procs);
        refreshProcessTable();
    }
}

void MainWindow::onStartScheduling() {
    auto procs = readProcessesFromUI();
    if (procs.empty()) {
        QMessageBox::warning(this, tr("Iniciar"), tr("Adicione pelo menos um processo."));
        return;
    }

    QString algText = ui->algorithmComboBox->currentText().trimmed();
        if (algText.isEmpty()) {
            QMessageBox::warning(this, tr("Erro"), tr("Selecione um algoritmo antes de iniciar."));
            return;
     }

    controller.setProcesses(procs);

    std::string alg = ui->algorithmComboBox->currentText().toStdString();
    int quantum = ui->quantumSpinBox->value();

    ScheduleOutcome o = controller.runAlgorithm(alg, quantum);

    AlgorithmParameters params;
    params.algorithmName = ui->algorithmComboBox->currentText().toStdString();
    params.quantum = ui->quantumSpinBox->value();

    fillResultsTable(o);
    buildTimeline(o);
}

void MainWindow::onClearAll() {
    controller.setProcesses({});
    ui->processesTable->setRowCount(0);
    ui->diagramTable->setColumnCount(0);
    ui->diagramTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultsTable->setRowCount(0);
}

// ---------------------------------------------------------
// Resultados
// ---------------------------------------------------------

void MainWindow::fillResultsTable(const ScheduleOutcome &o) {
    ui->resultsTable->blockSignals(true);
    ui->resultsTable->setRowCount(o.results.size());

    for (int i = 0; i < static_cast<int>(o.results.size()); ++i) {
        const auto &r = o.results[i];

        ui->resultsTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.name)));
        ui->resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(r.startTime)));
        ui->resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(r.finishTime)));
        ui->resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(r.waitingTime)));
        ui->resultsTable->setItem(i, 4, new QTableWidgetItem(QString::number(r.turnaroundTime)));

    }

    ui->resultsTable->blockSignals(false);
}

// ---------------------------------------------------------
// Timeline
// ---------------------------------------------------------

void MainWindow::buildTimeline(const ScheduleOutcome &o) {
    ui->diagramTable->blockSignals(true);
    ui->diagramTable->clearContents();

    int n = o.timeline.size();
    ui->diagramTable->setRowCount(2);
    ui->diagramTable->setColumnCount(n);

    ui->diagramTable->verticalHeader()->setVisible(true);
    ui->diagramTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->diagramTable->verticalHeader()->setDefaultSectionSize(40);

    for (int i = 0; i < n; ++i) {
        ui->diagramTable->setItem(0, i, new QTableWidgetItem(QString::number(o.timeline[i].time)));
        ui->diagramTable->setItem(1, i, new QTableWidgetItem(QString::fromStdString(o.timeline[i].running)));
        ui->diagramTable->setColumnWidth(i, 40);
    }

    ui->diagramTable->horizontalHeader()->setVisible(false);
    ui->diagramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->diagramTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->diagramTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    int totalHeight = ui->diagramTable->horizontalHeader()->height();
    for (int r = 0; r < 2; ++r)
        totalHeight += ui->diagramTable->rowHeight(r);

    totalHeight += (ui->diagramTable->horizontalScrollBar()->maximum() > 0)
                   ? ui->diagramTable->horizontalScrollBar()->height() : 0;

    ui->diagramTable->setMinimumHeight(totalHeight);
    ui->diagramTable->setMaximumHeight(totalHeight);
    ui->diagramTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui->diagramTable->setVisible(true);
    ui->diagramTable->blockSignals(false);
}

// ---------------------------------------------------------
// Salvar CSV
// ---------------------------------------------------------

void MainWindow::onSaveCSV() {
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Salvar Arquivo"),
        "",
        tr("Text Files (*.txt *.csv)")
    );

    if (path.isEmpty()) return;

    StorageManager manager;
    AlgorithmParameters params;
    params.algorithmName = ui->algorithmComboBox->currentText().toStdString();
    params.quantum = ui->quantumSpinBox->value();

    std::vector<Process> processes = readProcessesFromUI();

    std::vector<ProcessResult> results;
    int resRows = ui->resultsTable->rowCount();
    for (int i = 0; i < resRows; ++i) {
        ProcessResult r;
        r.name = ui->resultsTable->item(i, 0)->text().toStdString();
        r.startTime = ui->resultsTable->item(i, 1)->text().toInt();
        r.finishTime = ui->resultsTable->item(i, 2)->text().toInt();
        r.waitingTime = ui->resultsTable->item(i, 3)->text().toInt();
        r.turnaroundTime = ui->resultsTable->item(i, 4)->text().toInt();
        results.push_back(r);
    }

    if (!manager.saveProject(processes, params, results, path.toStdString())) {
        QMessageBox::warning(this, tr("Erro"), tr("Falha ao salvar o arquivo."));
    }
}

// ---------------------------------------------------------
// Carregar CSV
// ---------------------------------------------------------

void MainWindow::onLoadCSV() {
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Carregar Arquivo"),
        "",
        tr("Text Files (*.txt *.csv)")
    );

    if (path.isEmpty()) return;

    StorageManager manager;
    AlgorithmParameters params;
    std::vector<Process> processes;
    std::vector<ProcessResult> results;

    if (!manager.loadProject(processes, params, results, path.toStdString())) {
        QMessageBox::warning(this, tr("Erro"), tr("Falha ao carregar o arquivo."));
        return;
    }

    // Atualiza a interface
    int algIndex = ui->algorithmComboBox->findText(QString::fromStdString(params.algorithmName));
    if (algIndex >= 0) ui->algorithmComboBox->setCurrentIndex(algIndex);
    ui->quantumSpinBox->setValue(params.quantum);

    ui->processesTable->setRowCount(processes.size());
    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        ui->processesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(processes[i].name())));
        ui->processesTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].arrival())));
        ui->processesTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].burst())));
        ui->processesTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].priority())));
    }

    ui->resultsTable->setRowCount(results.size());
    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        ui->resultsTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(results[i].name)));
        ui->resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(results[i].startTime)));
        ui->resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(results[i].finishTime)));
        ui->resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(results[i].waitingTime)));
        ui->resultsTable->setItem(i, 4, new QTableWidgetItem(QString::number(results[i].turnaroundTime)));
    }
}

// ---------------------------------------------------------
// Menus Ajuda e Manual do Usuário
// ---------------------------------------------------------

void MainWindow::showManual()
{
    ManualDialog dialog(this);
    dialog.exec();
}

void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ---------------------------------------------------------
// Tradução
// ---------------------------------------------------------

void MainWindow::loadLanguage(const QString &langCode)
{
    qApp->removeTranslator(&translator);

    if (langCode == "en") {
        if (translator.load(":/translations/app_en.qm")) {
            qApp->installTranslator(&translator);
        }
    }
    else if (langCode == "pt") {
        if (translator.load(":/translations/app_pt.qm")) {
            qApp->installTranslator(&translator);
        }
    }

    ui->processesTable->setHorizontalHeaderLabels(
        { tr("Nome"), tr("Chegada"), tr("Execução"), tr("Prioridade") }
    );

    ui->resultsTable->setHorizontalHeaderLabels(
        { tr("PID"), tr("Início"), tr("Término"), tr("Espera"), tr("Turnaround") }
    );

    ui->diagramTable->setVerticalHeaderLabels(
        { tr("Tempo"), tr("Processo") }
    );

    ui->retranslateUi(this);
}
