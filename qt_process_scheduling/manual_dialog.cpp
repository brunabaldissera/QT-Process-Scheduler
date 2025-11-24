#include "manual_dialog.h"
#include "ui_manual_dialog.h"

ManualDialog::ManualDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::ManualDialog)
{
    ui->setupUi(this);
}

ManualDialog::~ManualDialog()
{
    delete ui;
}
