#ifndef TRANSACTIONINFODIALOG_H
#define TRANSACTIONINFODIALOG_H

#include <QDialog>

#include "blockchain.h"

namespace Ui {
class TransactionInfoDialog;
}

class TransactionInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransactionInfoDialog( TransactionInfo info, QWidget *parent = 0);
    ~TransactionInfoDialog();


    void  pop();

private slots:
    void on_okBtn_clicked();

private:
    Ui::TransactionInfoDialog *ui;
};

#endif // TRANSACTIONINFODIALOG_H
