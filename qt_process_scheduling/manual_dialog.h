#ifndef MANUAL_DIALOG_H
#define MANUAL_DIALOG_H

#include <QDialog>

namespace Ui {
class ManualDialog;
}

class ManualDialog : public QDialog {
    Q_OBJECT
public:
    explicit ManualDialog(QWidget* parent = nullptr);
    ~ManualDialog();

private:
    Ui::ManualDialog* ui;
};

#endif // MANUAL_DIALOG_H
