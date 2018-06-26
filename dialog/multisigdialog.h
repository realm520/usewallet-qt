#ifndef MULTISIGDIALOG_H
#define MULTISIGDIALOG_H

#include <QDialog>

namespace Ui {
class MultiSigDialog;
}

class MultiSigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiSigDialog(QWidget *parent = 0);
    ~MultiSigDialog();

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_chooseFileBtn_clicked();

    void on_pathLineEdit_textChanged(const QString &arg1);

private:
    Ui::MultiSigDialog *ui;

    void init();
    void parseBuilder();
};

#endif // MULTISIGDIALOG_H
