#ifndef ENTERADDRESSDIALOG_H
#define ENTERADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
class EnterAddressDialog;
}

class EnterAddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterAddressDialog(QWidget *parent = 0);
    ~EnterAddressDialog();

    bool  pop();

    QString address;

private slots:
    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_addressLineEdit_returnPressed();

    void on_addressLineEdit_textChanged(const QString &arg1);

private:
    Ui::EnterAddressDialog *ui;
    bool yesOrNO;
};

#endif // ENTERADDRESSDIALOG_H
