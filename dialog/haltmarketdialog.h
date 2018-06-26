#ifndef HALTMARKETDIALOG_H
#define HALTMARKETDIALOG_H

#include <QDialog>

namespace Ui {
class HaltMarketDialog;
}

class HaltMarketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HaltMarketDialog(int assetId, bool _trueOrNot, QWidget *parent = 0);
    ~HaltMarketDialog();

    int id;

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_accountComboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::HaltMarketDialog *ui;
    bool trueOrNot;

    void init();
};

#endif // HALTMARKETDIALOG_H
