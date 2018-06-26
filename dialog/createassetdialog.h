#ifndef CREATEASSETDIALOG_H
#define CREATEASSETDIALOG_H

#include <QDialog>

namespace Ui {
class CreateAssetDialog;
}

class CreateAssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateAssetDialog(QWidget *parent = 0);
    ~CreateAssetDialog();

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::CreateAssetDialog *ui;
};

#endif // CREATEASSETDIALOG_H
