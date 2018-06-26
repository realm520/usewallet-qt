#ifndef MARKETPAGE_H
#define MARKETPAGE_H

#include <QWidget>
#include <QMap>

namespace Ui {
class MarketPage;
}

struct OrderInfo
{
    QString type;
    QString ratio;
    int     quoteAssetId;
    int     baseAssetId;
    double  orderBalance;
    QString owner;
    QString orderId;
};
typedef QVector<OrderInfo>  OrdersInfoVector;
struct OrderHistory
{
    QString bidRatio;
    QString askRatio;
    int     quoteAssetId;
    int     baseAssetId;
    QString bidOwner;
    QString askOwner;
    double  bidPaidAmount;
    double  askPaidAmount;
    QString timeStamp;
};
typedef QVector<OrderHistory>   OrdersHistoryVector;


class MarketPage : public QWidget
{
    Q_OBJECT

public:
    explicit MarketPage( QString name, QWidget *parent = 0);
    ~MarketPage();

    void getAssets();

    void updateOrders();

    void updateMyOrders();

    void updateRecentOrdersHistory();

    void refresh();

private slots:
    void on_symbolComboBox1_currentIndexChanged(const QString &arg1);

    void on_symbolComboBox2_currentIndexChanged(const QString &arg1);

    void jsonDataUpdated(QString id);

    void on_marketBtn_clicked();

    void on_myOrdersBtn_clicked();

    void on_myHistoryBtn_clicked();

    void on_myOrdersTableWidget_cellClicked(int row, int column);

    void on_accountComboBox_currentIndexChanged(const QString &arg1);

    void on_bidBtn_clicked();

    void on_askBtn_clicked();

    void on_priceLineEdit_textChanged(const QString &arg1);

    void on_numLineEdit_textChanged(const QString &arg1);

    void on_amountLineEdit_textChanged(const QString &arg1);

    void on_bidOrderTableWidget_cellClicked(int row, int column);

    void on_askOrderTableWidget_cellClicked(int row, int column);


    void on_priceLineEdit2_textChanged(const QString &arg1);

    void on_numLineEdit2_textChanged(const QString &arg1);

    void on_amountLineEdit2_textChanged(const QString &arg1);

signals:
    void back();

private:
    Ui::MarketPage *ui;

    bool inited;
    bool editing;

    QString getCurrentQuoteOrBaseAsset(bool isQuote = true);    // true: 返回quoteAsset   false: 返回BaseAsset

    QMap<QString,OrdersInfoVector>      bidOrdersMap;
    QMap<QString,OrdersInfoVector>      askOrdersMap;
    void parseBidOrders(QString result);
    void parseAskOrders(QString result);
    void showBidOrderList();
    void showAskOrderList();

    QMap<QString,OrdersInfoVector>      myOrdersMap;
    void parseMyOrders(QString result);
    void showMyOrders();

    void setPrecision();

    QMap<QString,OrdersHistoryVector>          recentOrdersMap;
    void parseOrderHistory(QString result);
    void showRecentOrders();

    void showBalance();
};

#endif // MARKETPAGE_H
