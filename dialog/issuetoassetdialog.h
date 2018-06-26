#ifndef ISSUETOASSETDIALOG_H
#define ISSUETOASSETDIALOG_H

#include <QDialog>

namespace Ui {
class IssueToAssetDialog;
}

class IssueToAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IssueToAssetDialog( int assetId, QWidget *parent = 0);
    ~IssueToAssetDialog();

    int id;

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::IssueToAssetDialog *ui;

    void init();
};

#endif // ISSUETOASSETDIALOG_H
