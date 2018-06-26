#ifndef ACCOUNTPAGE_H
#define ACCOUNTPAGE_H

#include <QWidget>
#include "blockchain.h"

namespace Ui {
class AccountPage;
}

#define MODULE_ACCOUNT_PAGE "ACCOUNT_PAGE"

struct Entry;
class AccountPage : public QWidget
{
    Q_OBJECT

public:
    explicit AccountPage(QString name,QWidget *parent = 0);
    ~AccountPage();

    void updateTransactionsList();

    void getAssets();

public slots:
    void refresh();

private slots:

    void on_copyBtn_clicked();

    void on_accountComboBox_currentIndexChanged(const QString &arg1);

    void jsonDataUpdated(QString id);

//    void on_accountTransactionsTableWidget_cellClicked(int row, int column);

    void on_prePageBtn_clicked();

    void on_nextPageBtn_clicked();

    void on_pageLineEdit_editingFinished();

    void on_pageLineEdit_textEdited(const QString &arg1);

    void on_assetComboBox_currentIndexChanged(int index);

    void on_qrcodeBtn_clicked();
    void on_accountTransactionsTableWidget_cellPressed(int row, int column);

signals:
    void back();
    void accountChanged(QString);
    void showShadowWidget();
    void hideShadowWidget();
    void showApplyDelegatePage(QString);

private:
    Ui::AccountPage *ui;
    QString accountName;
    QString address;
    int transactionType;
    bool inited;
    int currentPageIndex;
    QString registeredLabelString;
    bool assetUpdating;

    void paintEvent(QPaintEvent*);
    void init();
    void showTransactions();
    void getTransaction(QString trxId);


};

#endif // ACCOUNTPAGE_H
