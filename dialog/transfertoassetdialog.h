#ifndef TRANSFERTOASSETDIALOG_H
#define TRANSFERTOASSETDIALOG_H

#include <QDialog>

namespace Ui {
class TransferToAssetDialog;
}

class TransferToAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferToAssetDialog(int assetId, QWidget *parent = 0);
    ~TransferToAssetDialog();

    int id;

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_accountComboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::TransferToAssetDialog *ui;

    void init();
};

#endif // TRANSFERTOASSETDIALOG_H
