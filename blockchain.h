/*
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
         佛祖保佑       永无BUG
*/

#ifndef DYL_H
#define DYL_H
#include <QObject>
#include <QMap>
#include <QSettings>
#include <QFile>
#include <QProcess>
#include <QMutex>
#include <QDialog>

#define ASSET_NAME "UL"
#define WASSET_NAME L"UL"
#define SHOW_NAME   "UL"
#define ADDRESS_PREFIX  "UL"
#define WALLET_VERSION "1.1.0"           // 版本号
#define AUTO_REFRESH_TIME 1000           // 自动刷新时间(ms)
#define SLOW_TIMER_TIME   60 * 1000
#define BACKGROUND_COLOR 190,218,255
#define FUNCTION_BAR_COLOR 109,91,255

//  密码输入错误5次后 锁定一段时间 (秒)
#define PWD_LOCK_TIME  7200
#define RPC_PORT 55090

#define MIN_BUTTON_STYLE "QToolButton{background-image:url(:/pic/pic2/minimize2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"\
                         "QToolButton:hover{background-image:url(:/pic/pic2/minimize_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
#define CLOSE_BUTTON_STYLE "QToolButton{background-image:url(:/pic/pic2/close2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"\
                           "QToolButton:hover{background-image:url(:/pic/pic2/close_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
#define CLICK_BUTTON_STYLE "color:white;border:1px solid rgb(109,91,255);border-radius:21px;background-color: rgb(109,91,255);"
#define MAX_CONTRACT_REMARK_LENGTH 30

class QTimer;
class QFrame;
class WorkerThreadManager;
class ConsoleWidget;

static QMutex mutexForJsonData;
static QMutex mutexForPending;
static QMutex mutexForConfigFile;
static QMutex mutexForMainpage;
static QMutex mutexForPendingFile;
static QMutex mutexForDelegateList;
static QMutex mutexForRegisterMap;
static QMutex mutexForBalanceMap;
static QMutex mutexForAddressMap;
static QMutex mutexForRpcReceiveOrNot;

typedef QMap<int,double>  AssetBalanceMap;
struct AssetInfo
{
    int id;
    QString symbol;
//    QString owner;
    int requiredNum;
    QStringList owners;
    QString name;
    QString description;
    double precision;
    double maxSupply;
    double currentSupply;
    QString registrationDate;
    int activeFlags;
};
struct AssetAmount
{
    int assetId;
    double amount;
};
struct Entry
{
    QString fromAccount;
    QString toAccount;
    AssetAmount amount;
    QString memo;
    QVector<AssetAmount>    runningBalances;
};
struct TransactionInfo
{
    QString trxId;
    bool isConfirmed;
    bool isMarket;
    bool isMarketCancel;
    int blockNum;
    QVector<Entry>  entries;
//    QString fromAaccount;
//    QString toAccount;
//    double amount;      //  交易金额
//    int assetId;        //  交易金额的资产类型
//    QString memo;
//    AssetBalanceMap runningBalances;
    double fee;
    int    feeId;
    QString timeStamp;
    QString expirationTimeStamp;
};
typedef QVector<TransactionInfo>  TransactionsInfoVector;

struct TwoAddresses     // owner_address 和 active_address
{
    QString ownerAddress;
    QString activeAddress;
};

struct OperationData
{
    QString name;
    int dataType;   // 值类型 string:0  int:1
    QString value;
};
struct Operation
{
    QString type;
//    QVector<OperationData> dataVector;
};
struct TransactionOperation
{
    QString trxId;
    int type = 0;       // 1:资产状态更新     2:创建资产  3:空投  4:分红  5:转账    6:注册账户  7:分配资产  8:市场撤单
                        // 9:市场挂买单      10:市场挂卖单  11:编辑管理员  12:暂停市场  13:恢复市场   -1:市场成交  0:未知
    QVector<Operation>  operationVector;
};

class Blockchain : public QObject
{
    Q_OBJECT
public:
    ~Blockchain();
    static Blockchain*   getInstance();
    qint64 write(QString);
    void quit();
    QString read();
    QProcess* proc;
    int lockMinutes;   // 自动锁定时间
    bool notProduce;   // 是否产块/记账
    bool minimizeToTray;  // 是否最小化到托盘
    bool closeToMinimize; // 是否点击关闭最小化
    bool resyncNextTime;    // 下次启动时是否重新同步
    QString language;   // 语言
    bool needToScan;   // 在当前scan完成后是否还需要scan
    QString currentAccount; // 保存当前账户  切换页面的时候默认选择当前账户
    unsigned long long transactionFee;
    ConsoleWidget* consoleWidget;

    QMap<QString,QString> balanceMap;
    QMap<QString,TwoAddresses> addressMap;
    QMap<QString,QString> registerMap;
    QMap<QString,bool> rpcReceivedOrNotMap;  // 标识rpc指令是否已经返回了，如果还未返回则忽略

    TwoAddresses getAddress(QString);
    bool    isMyAddress(QString address);
    QStringList getMyAddresses(QStringList addresses);      // 从一堆地址中找出钱包内的地址
    QString addressToName(QString address);
    QString getBalance(QString);
    QString getRegisterTime(QString);
    void deleteAccountInConfigFile(QString);
    void updateJsonDataMap(QString id, QString data);
    QString jsonDataValue(QString id);
    double getPendingAmount(QString name);
    QString getPendingInfo(QString id);

    QString registerMapValue(QString key);
    void registerMapInsert(QString key, QString value);
    int registerMapRemove(QString key);
    QString balanceMapValue(QString key);
    void balanceMapInsert(QString key, QString value);
    int balanceMapRemove(QString key);
    TwoAddresses addressMapValue(QString key);
    void addressMapInsert(QString key, TwoAddresses value);
    int addressMapRemove(QString key);
    bool rpcReceivedOrNotMapValue(QString key);
    void rpcReceivedOrNotMapSetValue(QString key, bool received);

    void appendCurrentDialogVector(QWidget*);
    void removeCurrentDialogVector(QWidget *);
    void hideCurrentDialog();
    void showCurrentDialog();
    void resetPosOfCurrentDialog();

    void initWorkerThreadManager();
    void destroyWorkerThreadManager();
    void postRPC(QString cmd);

    void getContactsFile();  // contacts.dat 改放到数据路径
    QString getBuilderFilePath(QString prefix);  // 获取生成的transaction_builder路径

    QVector<QWidget*> currentDialogVector;  // 保存不属于frame的dialog
                                            // 为的是自动锁定的时候hide这些dialog

    QSettings *configFile;
//    void loadAccountInfo();

    QString appDataPath;
    QString walletConfigPath;

    QFile* contactsFile;
    QFile* pendingFile;

    QThread* threadForWorkerManager;

    QDialog* currentDialog;  // 如果不为空 则指向当前最前面的不属于frame的dialog
                             // 为的是自动锁定的时候hide该dialog

    QFrame* mainFrame = NULL; // 指向主窗口的指针

    int currentPort;          // 当前rpc 端口

    QString localIP;   // 保存 peerinfo 获得的本机IP和端口

    int currentBlockHeight = 0;     // info 获得的当前区块高度

    QMap<QString,AssetBalanceMap> accountBalanceMap;
    void parseBalance();

    QMap<int,AssetInfo>  assetInfoMap;
    void parseAssetInfo();
    int getAssetId(QString symbol);

    QMap<QString,TransactionsInfoVector> transactionsMap;   // key是 "账户名-资产符号" 形式
    void parseTransactions(QString result, QString _key = "ALL");

    QMap<QString,TransactionOperation>  transactionOperationsMap;
    void parseTransactionOperations(QString result);

    void scanLater();
    QString currentAsset;   // 保存当前资产

    QStringList delegateAccountList;
    bool isDelegateAccount(QString name);
    QStringList getMyDelegateAccounts();      // 钱包是否有

private slots:
    void scan();


signals:
    void started();

    void jsonDataUpdated(QString);

    void rpcPosted(QString cmd);

private:

    Blockchain();
    static Blockchain* goo;
    QMap<QString,QString> jsonDataMap;   //  各指令的id,各指令的返回
    WorkerThreadManager* workerManager;  // 处理rpc worker thread

    void getSystemEnvironmentPath();
    void changeToWalletConfigPath();     // 4.2.2后config pending log 等文件移动到 APPDATA路径

    class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~CGarbo()
        {
            if (Blockchain::goo)
                delete Blockchain::goo;
        }
    };
    static CGarbo Garbo; // 定义一个静态成员，在程序结束时，系统会调用它的析构函数
};

QString doubleTo5Decimals(double number);
QString getShowName(QString name);
QString getAssetName(QString name);
double roundDown(double decimal, int precision = 0);        // 根据精度 向下取"整"
QString removeLastZeros(QString number);        // qstring::number() 对小数的处理有问题  使用std::to_string() 然后把后面的0去掉
QString getBigNumberString(unsigned long long number,unsigned long long precision);
bool   checkAddress(QString address);
bool    checkSignatureEnough(QString builder);
bool isExistInWallet(QString);

struct TransactionDetail
{
//        QString opposite;
//        int type = 0;   // 0:默认/自己转自己  1:转出 2:转入    3:市场挂买单  4:市场挂卖单  5:市场成交   6:市场撤单
//        AssetAmount assetAmount;
//        QString memo;
    QString type;
    QVector<QString> amountVector;
    bool isAssetRelated = true;    // 是否与传入的资产相关
};
TransactionDetail getDetail(TransactionInfo info, QString accountName, QString _assetName);    // 从transactioninfo中获取交易对方   或者市场挂单/市场撤单/市场成交
QVector<Entry> getMyEntries(QVector<Entry> entries, QString accountName);

#endif // DYL_H

