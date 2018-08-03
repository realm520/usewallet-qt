#include "blockchain.h"
#include "debug_log.h"
#include "workerthreadmanager.h"
#include "rpcthread.h"
#include <QTextCodec>
#include <QDebug>
#include <QObject>
#ifdef WIN32
#include <Windows.h>
#endif
#include <QTimer>
#include <QThread>
#include <QApplication>
#include <QFrame>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Blockchain* Blockchain::goo = 0;

Blockchain::Blockchain()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    proc = new QProcess;

    workerManager = NULL;

    getSystemEnvironmentPath();
    changeToWalletConfigPath();

    threadForWorkerManager = NULL;
    currentDialog = NULL;
    needToScan = false;
    currentPort = RPC_PORT;
    currentAccount = "";
    transactionFee = 0;

    configFile = new QSettings(walletConfigPath + "/config.ini", QSettings::IniFormat);
    if( configFile->value("/settings/lockMinutes").toInt() == 0)   // 如果第一次，没有config.ini
    {
        configFile->setValue("/settings/lockMinutes",5);
        lockMinutes     = 5;
        configFile->setValue("/settings/notAutoLock",false);
        notProduce      =  true;
        configFile->setValue("/settings/language","Simplified Chinese");
        language = "Simplified Chinese";
        minimizeToTray  = false;
        configFile->setValue("/settings/minimizeToTray",false);
        closeToMinimize = false;
        configFile->setValue("/settings/closeToMinimize",false);
        resyncNextTime = false;
        configFile->setValue("settings/resyncNextTime",false);

    }
    else
    {
        lockMinutes     = configFile->value("/settings/lockMinutes").toInt();
        notProduce      = !configFile->value("/settings/notAutoLock").toBool();
        minimizeToTray  = configFile->value("/settings/minimizeToTray").toBool();
        closeToMinimize = configFile->value("/settings/closeToMinimize").toBool();
        language        = configFile->value("/settings/language").toString();
        resyncNextTime  = configFile->value("/settings/resyncNextTime",false).toBool();

    }

    QFile file( walletConfigPath + "/log.txt");       // 每次启动清空 log.txt文件
    file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    file.close();

//    contactsFile = new QFile( "contacts.dat");
    contactsFile = NULL;

    pendingFile  = new QFile( walletConfigPath + "/pending.dat");

    consoleWidget = NULL;
    currentAsset = "UL";

//    delegateAccountList << "init0"  << "init1"  << "init2"  << "init3"  << "init4"  << "init5"  << "init6"  << "init7"  << "init8"  << "init9"
//                        << "init10" << "init11" << "init12" << "init13" << "init14" << "init15" << "init16" << "init17" << "init18" << "init19"
//                        << "init20" << "init21" << "init22" << "init23" << "init24";
    DLOG_QT_WALLET_FUNCTION_END;
}

Blockchain::~Blockchain()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

	if (configFile)
	{
		delete configFile;
		configFile = NULL;
	}

    if( contactsFile)
    {
        delete contactsFile;
        contactsFile = NULL;
    }

    if( threadForWorkerManager)
    {
        delete threadForWorkerManager;
        threadForWorkerManager = NULL;
    }

    if( workerManager)
    {
        delete workerManager;
        workerManager = NULL;
    }

    DLOG_QT_WALLET_FUNCTION_END;
}

Blockchain*   Blockchain::getInstance()
{
    if( goo == NULL)
    {
        goo = new Blockchain;
    }
    return goo;
}


qint64 Blockchain::write(QString cmd)
{
    QTextCodec* gbkCodec = QTextCodec::codecForName("GBK");
    QByteArray cmdBa = gbkCodec->fromUnicode(cmd);  // 转为gbk的bytearray
    proc->readAll();
    return proc->write(cmdBa.data());
}

QString Blockchain::read()
{
    QTextCodec* gbkCodec = QTextCodec::codecForName("GBK");
    QString result;
    QString str;
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    while ( !result.contains(">>>"))        // 确保读取全部输出
    {
        proc->waitForReadyRead(50);
        str = gbkCodec->toUnicode(proc->readAll());
        result += str;
        if( str.right(2) == ": " )  break;

        //  手动调用处理未处理的事件，防止界面阻塞
//        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    QApplication::restoreOverrideCursor();
    return result;

}

void Blockchain::deleteAccountInConfigFile(QString accountName)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    mutexForConfigFile.lock();
    configFile->beginGroup("/accountInfo");
    QStringList keys = configFile->childKeys();

    int i = 0;
    for( ; i < keys.size(); i++)
    {
        if( configFile->value( keys.at(i)) == accountName)
        {
            break;
        }
    }

    for( ; i < keys.size() - 1; i++)
    {
        configFile->setValue( keys.at(i) , configFile->value( keys.at(i + 1)));
    }

    configFile->remove( keys.at(i));

    configFile->endGroup();
    mutexForConfigFile.unlock();

    DLOG_QT_WALLET_FUNCTION_END;
}


TwoAddresses Blockchain::getAddress(QString name)
{
    TwoAddresses twoAddresses;

    if( name.isEmpty())
    {
        return twoAddresses;
    }

    QString result = jsonDataValue("id_wallet_list_my_addresses");

    int pos = result.indexOf( "\"name\":\"" + name + "\",") ;
    if( pos != -1)  // 如果 wallet_list_my_addresses 中存在
    {

        int pos2 = result.indexOf( "\"owner_address\":", pos) + 17;
        twoAddresses.ownerAddress = result.mid( pos2, result.indexOf( "\"", pos2) - pos2);

        pos2 = result.indexOf( "\"active_address\":", pos) + 18;
        twoAddresses.activeAddress = result.mid( pos2, result.indexOf( "\"", pos2) - pos2);
    }

    return twoAddresses;
}

bool Blockchain::isMyAddress(QString address)
{
    if( address.isEmpty())
    {
        return false;
    }

    bool result = false;
    foreach (QString key, addressMap.keys())
    {
         if( addressMap.value(key).ownerAddress == address || addressMap.value(key).activeAddress == address)
         {
             result = true;
             break;
         }
    }

    return result;
}

QStringList Blockchain::getMyAddresses(QStringList addresses)
{
    QStringList result;
    foreach (QString address, addresses)
    {
        if(isMyAddress(address))   result << address;
    }

    return result;
}

QString Blockchain::addressToName(QString address)
{
    foreach (QString key, addressMap.keys())
    {
        if( addressMap.value(key).ownerAddress == address)
        {
            return key;
        }
    }

    return address;
}

QString Blockchain::getBalance(QString name)
{
    if( name.isEmpty())
    {
        return NULL;
    }


    QString result = jsonDataValue("id_balance");

    int p = result.indexOf( "[\"" + name + "\",");
    if( p != -1)  // 如果balance中存在
    {
        int pos = p + 8 + name.size();
        QString str = result.mid( pos, result.indexOf( "]", pos) - pos);
        str.remove("\"");

        double amount = str.toDouble() / 100000;

        return doubleTo5Decimals( amount) + " " + QString(ASSET_NAME);
    }

    // balance中不存在
    return "0.00000 " + QString(ASSET_NAME);
}

QString Blockchain::getRegisterTime(QString name)
{
    if( name.isEmpty())
    {
        return NULL;
    }

    QString result = jsonDataValue("id_wallet_list_my_addresses");
    int pos = result.indexOf( "\"name\":\"" + name + "\",") ;
    if( pos != -1)  // 如果 wallet_list_my_addresses 中存在
    {
        int pos2 = result.indexOf( "\"registration_date\":", pos) + 21;
        QString registerTime = result.mid( pos2, result.indexOf( "\"", pos2) - pos2);

        if( registerTime != "1970-01-01T00:00:00")
        {
            return registerTime;
        }
        else
        {
            return "NO";
        }
    }

    return "WRONGACCOUNTNAME";
}

void Blockchain::getSystemEnvironmentPath()
{
    QStringList environment = QProcess::systemEnvironment();
    QString str;

#ifdef WIN32
    foreach(str,environment)
    {
        if (str.startsWith("APPDATA="))
        {
            walletConfigPath = str.mid(8) + "\\" ASSET_NAME "Wallet";
            appDataPath = walletConfigPath + "\\chaindata";
            qDebug() << "appDataPath:" << appDataPath;
            break;
        }
    }
#elif defined(TARGET_OS_MAC)
    foreach(str,environment)
    {
        if (str.startsWith("HOME="))
        {
            walletConfigPath = str.mid(5) + "/Library/Application Support/"ASSET_NAME"Wallet";
            appDataPath = walletConfigPath + "/chaindata";
            qDebug() << "appDataPath:" << appDataPath;
            break;
        }
    }
#else
    foreach(str,environment)
    {
        if (str.startsWith("HOME="))
        {
            walletConfigPath = str.mid(5) + "/"ASSET_NAME"Wallet";
            appDataPath = walletConfigPath + "/chaindata";
            qDebug() << "appDataPath:" << appDataPath;
            break;
        }
    }
#endif
}

void Blockchain::changeToWalletConfigPath()
{
    QFile file("config.ini");
    if( !file.exists())     return;
    QFile file2(walletConfigPath + "/config.ini");
    qDebug() << file2.exists() << walletConfigPath + "/config.ini";
    if( file2.exists())
    {
        qDebug() << "remove config.ini : " << file.remove();
        return;
    }

    qDebug() << "copy config.ini : " << file.copy(walletConfigPath + "/config.ini");
    qDebug() << "remove old config.ini : " << file.remove();
}

void Blockchain::getContactsFile()
{
    QString path;
    if( configFile->contains("/settings/chainPath"))
    {
        path = configFile->value("/settings/chainPath").toString();
    }
    else
    {
        path = appDataPath;
    }

    contactsFile = new QFile(path + "/contacts.dat");
    if( contactsFile->exists())
    {
        // 如果数据路径下 有contacts.dat 则使用数据路径下的
        return;
    }
    else
    {
        QFile file2("contacts.dat");
        if( file2.exists())
        {
            // 如果数据路径下没有 钱包路径下有 将钱包路径下的剪切到数据路径下
       qDebug() << "contacts.dat copy" << file2.copy(path + "/contacts.dat");
       qDebug() << "contacts.dat copy" << file2.remove();
            return;
        }
        else
        {
            // 如果都没有 则使用钱包路径下的
        }
    }
}

QString Blockchain::getBuilderFilePath(QString prefix)
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd_hh.mm.ss");
    QString configPath = Blockchain::getInstance()->walletConfigPath;
    QString builderPath = configPath.replace("\\","/") + "/transaction_builder/" + prefix
            + "_" + current_date + ".builder" ;

    return builderPath;
}

void Blockchain::parseBalance()
{
    accountBalanceMap.clear();

    QString jsonResult = jsonDataValue("id_balance");
    jsonResult.prepend("{");
    jsonResult.append("}");

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(jsonResult.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if( parse_doucment.isObject())
        {
            QJsonObject jsonObject = parse_doucment.object();
            if( jsonObject.contains("result"))
            {
                QJsonValue resultValue = jsonObject.take("result");
                if( resultValue.isArray())
                {
                    QJsonArray resultArray = resultValue.toArray();
                    for( int i = 0; i < resultArray.size(); i++)
                    {
                        QJsonValue resultArrayValue = resultArray.at(i);
                        if( resultArrayValue.isArray())
                        {
                            QJsonArray array = resultArrayValue.toArray();
                            QJsonValue value = array.at(0);
                            QString account = value.toString();
                            AssetBalanceMap assetBalanceMap;
							qDebug() << array;
                            QJsonArray array2 = array.at(1).toArray();

                            for( int i = 0; i < array2.size(); i++)
                            {
                                QJsonArray array3 = array2.at(i).toArray();
                                int asset = array3.at(0).toInt();

                                share_type balance;
                                if( array3.at(1).isString())
                                {
                                    balance = array3.at(1).toString().toULongLong();
                                }
                                else
                                {
									balance = array3.at(1).toInt();
                                }

                                assetBalanceMap.insert(asset, balance);

                            }
                            accountBalanceMap.insert(account,assetBalanceMap);
                        }
                    }
                }

            }
        }
    }


}


void Blockchain::parseAssetInfo()
{
    assetInfoMap.clear();

    QString jsonResult = jsonDataValue("id_blockchain_list_assets");
    jsonResult.prepend("{");
    jsonResult.append("}");


    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(jsonResult);

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(ba, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if( parse_doucment.isObject())
        {
            QJsonObject jsonObject = parse_doucment.object();
            if( jsonObject.contains("result"))
            {
                QJsonValue resultValue = jsonObject.take("result");
                if( resultValue.isArray())
                {
                    QJsonArray resultArray = resultValue.toArray();
                    for( int i = 0; i < resultArray.size(); i++)
                    {
                        AssetInfo assetInfo;

                        QJsonObject object = resultArray.at(i).toObject();
                        assetInfo.id = object.take("id").toInt();
                        assetInfo.symbol = object.take("symbol").toString();
                        assetInfo.activeFlags = object.take("active_flags").toInt();

                        QJsonObject object2 = object.take("authority").toObject();
                        assetInfo.requiredNum = object2.take("required").toInt();
                        QJsonArray array = object2.take("owners").toArray();
                        for( int j = 0; j < array.size(); j++)
                        {
                            assetInfo.owners << array.at(j).toString();
                        }

                        assetInfo.name = object.take("name").toString();
                        QJsonValue desValue = object.take("description");
                        assetInfo.description = desValue.toString();

                        QJsonValue value = object.take("precision");
                        if( value.isString())
                        {
                            assetInfo.precision = value.toString().toUInt();
                        }
                        else
                        {
                            assetInfo.precision = value.toInt();
                        }

                        QJsonValue value2 = object.take("max_supply");
                        if( value2.isString())
                        {
                            assetInfo.maxSupply = value2.toString().toDouble();
                        }
                        else
                        {
                            assetInfo.maxSupply = value2.toDouble();
                        }

                        QJsonValue value3 = object.take("current_supply");
                        if( value3.isString())
                        {
                            assetInfo.currentSupply = value3.toString().toDouble();
                        }
                        else
                        {
                            assetInfo.currentSupply = value3.toDouble();
                        }

                        assetInfo.registrationDate = object.take("registration_date").toString();

                        assetInfoMap.insert(assetInfo.id,assetInfo);

                    }
                }
            }
        }
    }

}

int Blockchain::getAssetId(QString symbol)
{
    int id = -1;
    foreach (int key, Blockchain::getInstance()->assetInfoMap.keys())
    {
        AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(key);
        if( info.symbol == symbol)
        {
            id = key;
            continue;
        }
    }

    return id;
}

void Blockchain::parseTransactions(QString result, QString _key)
{
    transactionsMap.remove(_key);

    result.prepend("{");
    result.append("}");

    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(result);

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(ba, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if( parse_doucment.isObject())
        {
            QJsonObject jsonObject = parse_doucment.object();
            if( jsonObject.contains("result"))
            {
                QJsonValue resultValue = jsonObject.take("result");
                if( resultValue.isArray())
                {
                    TransactionsInfoVector transactionsInfoVector;
                    QJsonArray resultArray = resultValue.toArray();
                    for( int i = 0; i < resultArray.size(); i++)
                    {
                        TransactionInfo transactionInfo;
                        QJsonObject object          = resultArray.at(i).toObject();
                        transactionInfo.isConfirmed = object.take("is_confirmed").toBool();
                        if ( !transactionInfo.isConfirmed)
                        {
                            // 包含error  则为失效交易
                            if( object.contains("error"))   continue;
                        }

                        transactionInfo.trxId       = object.take("trx_id").toString();
                        transactionInfo.isMarket    = object.take("is_market").toBool();
                        transactionInfo.isMarketCancel = object.take("is_market_cancel").toBool();
                        transactionInfo.blockNum    = object.take("block_num").toInt();
                        transactionInfo.timeStamp   = object.take("timestamp").toString();
                        transactionInfo.expirationTimeStamp = object.take("expiration_timestamp").toString();

                        QJsonArray entriesArray       = object.take("ledger_entries").toArray();
                        for( int j = 0; j < entriesArray.size(); j++)
                        {
                            QJsonObject entryObject = entriesArray.at(j).toObject();
                            Entry   entry;
                            entry.fromAccount       = entryObject.take("from_account").toString();
                            entry.toAccount         = entryObject.take("to_account").toString();
                            QJsonValue v = entryObject.take("memo");
                            entry.memo              = v.toString();
                            QJsonObject amountObject = entryObject.take("amount").toObject();
                            entry.amount.assetId     = amountObject.take("asset_id").toInt();
                            QJsonValue amountValue   = amountObject.take("amount");
                            if( amountValue.isString())
                            {
                                entry.amount.amount  = amountValue.toString().toULongLong();
                            }
                            else
                            {
                                entry.amount.amount  = amountValue.toInt();
                            }

                            QJsonArray runningBalanceArray  = entryObject.take("running_balances").toArray().at(0).toArray().at(1).toArray();
                            for( int k = 0; k < runningBalanceArray.size(); k++)
                            {
                                QJsonObject amountObject2    = runningBalanceArray.at(k).toArray().at(1).toObject();
                                AssetAmount assetAmount;
                                assetAmount.assetId = amountObject2.take("asset_id").toInt();
                                QJsonValue amountValue2   = amountObject2.take("amount");
                                if( amountValue2.isString())
                                {
                                    assetAmount.amount  = amountValue2.toString().toULongLong();
                                }
                                else
                                {
                                    assetAmount.amount  = amountValue2.toInt();
                                }
                                entry.runningBalances.append(assetAmount);
                            }

                            transactionInfo.entries.append(entry);
                        }

                        QJsonObject object5         = object.take("fee").toObject();
                        QJsonValue amountValue3     = object5.take("amount");
                        if( amountValue3.isString())
                        {
                            transactionInfo.fee     = amountValue3.toString().toULongLong();
                        }
                        else
                        {
                            transactionInfo.fee     = amountValue3.toInt();
                        }
                        transactionInfo.feeId       = object5.take("asset_id").toInt();


                        QString accountName = _key.left(_key.indexOf("_"));
                        QString assetName = _key.mid(_key.indexOf("_") + 1);

                        TransactionDetail detail = getDetail(transactionInfo, accountName, assetName);
                        if(!detail.isAssetRelated)
                        {
                            continue;
                        }

                        transactionsInfoVector.append(transactionInfo);
                    }

                    transactionsMap.insert(_key,transactionsInfoVector);
                }
            }
        }
    }
}

void Blockchain::parseTransactionOperations(QString result)
{

    result.prepend("{");
    result.append("}");

    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(result);

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(ba, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if( parse_doucment.isObject())
        {
            QJsonObject jsonObject = parse_doucment.object();
            if( jsonObject.contains("result"))
            {
                QJsonValue resultValue = jsonObject.take("result");
                if( resultValue.isArray())
                {
                    TransactionOperation transaction;
                    QString trxId = resultValue.toArray().at(0).toString();
                    transaction.trxId = trxId;

                    QJsonObject trxObject = resultValue.toArray().at(1).toObject().take("trx").toObject();
                    QJsonArray operationsArray = trxObject.take("operations").toArray();

                    QStringList opTypes;
                    int updatePermissionType = -1;      // 1: 编辑管理员   2:暂停市场  3:恢复市场
                    for( int i = 0; i < operationsArray.size(); i++)
                    {
                        Operation op;

                        QJsonObject opObject          = operationsArray.at(i).toObject();
                        op.type = opObject.take("type").toString();

                        opTypes += op.type;
                        transaction.operationVector.append(op);

                        if(op.type == "asset_update_permissions_op_type")
                        {
                            QJsonObject dataObject = opObject.take("data").toObject();
                            if(dataObject.contains("authority"))
                            {
                                updatePermissionType = 1;
                            }
                            else if(dataObject.contains("active_flags"))
                            {
                                int flag = dataObject.take("active_flags").toInt();
                                if(flag & 0x04)
                                {
                                    updatePermissionType = 2;
                                }
                                else
                                {
                                    updatePermissionType = 3;
                                }
                            }
                        }
                    }

                    if( opTypes.contains("asset_update_permissions_op_type"))
                    {
                        transaction.type = 1;

                        switch (updatePermissionType)
                        {
                        case 1:
                            transaction.type = 11;
                            break;
                        case 2:
                            transaction.type = 12;
                            break;
                        case 3:
                            transaction.type = 13;
                            break;
                        default:
                            break;
                        }
                    }
                    else if( opTypes.contains("create_asset_op_type"))
                    {
                        transaction.type = 2;
                    }
                    else if( opTypes.contains("deposit_to_asset_op_type") )
                    {
                        if( opTypes.contains("issue_asset_op_type"))
                        {
                            transaction.type = 3;
                        }
                        else
                        {
                            transaction.type = 4;
                        }
                    }
                    else if( opTypes.contains("issue_asset_op_type"))
                    {
                        transaction.type = 7;
                    }
                    else if( opTypes.contains("register_account_op_type"))
                    {
                        transaction.type = 6;
                    }
                    else if( opTypes.contains("bid_op_type"))
                    {
                        if( opTypes.contains("deposit_op_type"))
                        {
                            transaction.type = 8;
                        }
                        else
                        {
                            transaction.type = 9;
                        }
                    }
                    else if( opTypes.contains("ask_op_type"))
                    {
                        if( opTypes.contains("deposit_op_type"))
                        {
                            transaction.type = 8;
                        }
                        else
                        {
                            transaction.type = 10;
                        }
                    }
                    else if( opTypes.contains("deposit_op_type"))
                    {
                        transaction.type = 5;
                    }
                    else
                    {
                        transaction.type = -1;
                    }

                    transactionOperationsMap.insert(trxId,transaction);
                }
            }
        }
    }
}

void Blockchain::scanLater()
{
    QTimer::singleShot(10000,this,SLOT(scan()));
}

bool Blockchain::isDelegateAccount(QString name)
{
    return delegateAccountList.contains(name);
}

QStringList Blockchain::getMyDelegateAccounts()
{
    QStringList delegates;
    foreach (QString account, addressMap.keys())
    {
        if( isDelegateAccount(account))     delegates << account;
    }
    return delegates;
}

void Blockchain::scan()
{
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_scan", "scan", QStringList() << "0"));
}



void Blockchain::quit()
{
    if (proc)
    {
        proc->close();
        qDebug() << "proc: close";
        delete proc;
        proc = NULL;
    }
}

void Blockchain::updateJsonDataMap(QString id, QString data)
{
    mutexForJsonData.lock();

    jsonDataMap.insert( id, data);
    emit jsonDataUpdated(id);

    mutexForJsonData.unlock();

}

QString Blockchain::jsonDataValue(QString id)
{

    mutexForJsonData.lock();

    QString value = jsonDataMap.value(id);

    mutexForJsonData.unlock();

    return value;
}

double Blockchain::getPendingAmount(QString name)
{
    mutexForConfigFile.lock();
    if( !pendingFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "pending.dat not exist";
        return 0;
    }
    QString str = QByteArray::fromBase64( pendingFile->readAll());
    pendingFile->close();
    QStringList strList = str.split(";");
    strList.removeLast();

    mutexForConfigFile.unlock();

    double amount = 0;
    foreach (QString ss, strList)
    {
        QStringList sList = ss.split(",");
        if( sList.at(1) == name)
        {
            amount += sList.at(2).toDouble() + sList.at(3).toDouble();
        }
    }

    return amount;
}

QString Blockchain::getPendingInfo(QString id)
{
    mutexForConfigFile.lock();
    if( !pendingFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "pending.dat not exist";
        return 0;
    }
    QString str = QByteArray::fromBase64( pendingFile->readAll());
    pendingFile->close();
    QStringList strList = str.split(";");
    strList.removeLast();

    mutexForConfigFile.unlock();

    QString info;
    foreach (QString ss, strList)
    {
        QStringList sList = ss.split(",");
        if( sList.at(0) == id)
        {
            info = ss;
            break;
        }
    }

    return info;
}


QString doubleTo5Decimals(double number)
{
        QString num = QString::number(number,'f', 5);
        int pos = num.indexOf('.') + 6;
        return num.mid(0,pos);
}

QString Blockchain::registerMapValue(QString key)
{
    mutexForRegisterMap.lock();
    QString value = registerMap.value(key);
    mutexForRegisterMap.unlock();

    return value;
}

void Blockchain::registerMapInsert(QString key, QString value)
{
    mutexForRegisterMap.lock();
    registerMap.insert(key,value);
    mutexForRegisterMap.unlock();
}

int Blockchain::registerMapRemove(QString key)
{
    mutexForRegisterMap.lock();
    int number = registerMap.remove(key);
    mutexForRegisterMap.unlock();
    return number;
}

QString Blockchain::balanceMapValue(QString key)
{
    mutexForBalanceMap.lock();
    QString value = balanceMap.value(key);
    mutexForBalanceMap.unlock();

    return value;
}

void Blockchain::balanceMapInsert(QString key, QString value)
{
    mutexForBalanceMap.lock();
    balanceMap.insert(key,value);
    mutexForBalanceMap.unlock();
}

int Blockchain::balanceMapRemove(QString key)
{
    mutexForBalanceMap.lock();
    int number = balanceMap.remove(key);
    mutexForBalanceMap.unlock();
    return number;
}

TwoAddresses Blockchain::addressMapValue(QString key)
{
    mutexForAddressMap.lock();
    TwoAddresses value = addressMap.value(key);
    mutexForAddressMap.unlock();

    return value;
}

void Blockchain::addressMapInsert(QString key, TwoAddresses value)
{
    mutexForAddressMap.lock();
    addressMap.insert(key,value);
    mutexForAddressMap.unlock();
}

int Blockchain::addressMapRemove(QString key)
{
    mutexForAddressMap.lock();
    int number = addressMap.remove(key);
    mutexForAddressMap.unlock();
    return number;
}

bool Blockchain::rpcReceivedOrNotMapValue(QString key)
{
    mutexForRpcReceiveOrNot.lock();
    bool received = rpcReceivedOrNotMap.value(key);
    mutexForRpcReceiveOrNot.unlock();
    return received;
}

void Blockchain::rpcReceivedOrNotMapSetValue(QString key, bool received)
{
    mutexForRpcReceiveOrNot.lock();
    rpcReceivedOrNotMap.insert(key, received);
    mutexForRpcReceiveOrNot.unlock();
}


void Blockchain::appendCurrentDialogVector( QWidget * w)
{
    currentDialogVector.append(w);
}

void Blockchain::removeCurrentDialogVector( QWidget * w)
{
    currentDialogVector.removeAll(w);
}

void Blockchain::hideCurrentDialog()
{
    foreach (QWidget* w, currentDialogVector)
    {
        w->hide();
    }
}

void Blockchain::showCurrentDialog()
{
    foreach (QWidget* w, currentDialogVector)
    {
        qDebug() << "showCurrentDialog :" << w;
        w->show();
        w->move( mainFrame->pos());  // lock界面可能会移动，重新显示的时候要随之移动
    }
}

void Blockchain::resetPosOfCurrentDialog()
{
    foreach (QWidget* w, currentDialogVector)
    {
        w->move( mainFrame->pos());
    }
}

void Blockchain::initWorkerThreadManager()
{
    qDebug() << "initWorkerThreadManager " << QThread::currentThreadId();
    if( workerManager)
    {
        delete workerManager;
    }
    if( threadForWorkerManager)
    {
        delete threadForWorkerManager;
    }

    threadForWorkerManager = new QThread;
    threadForWorkerManager->start();
    workerManager = new WorkerThreadManager;
    workerManager->moveToThread(threadForWorkerManager);
    connect(this, SIGNAL(rpcPosted(QString)), workerManager, SLOT(processRPCs(QString)));

}

void Blockchain::destroyWorkerThreadManager()
{
    qDebug() << "destroyWorkerThreadManager " << QThread::currentThreadId();
    if( workerManager)
    {
        workerManager->deleteLater();
        workerManager = NULL;

        if( threadForWorkerManager)
        {
            threadForWorkerManager->deleteLater();
            threadForWorkerManager = NULL;
        }
    }

}

void Blockchain::postRPC(QString cmd)
{
    if( rpcReceivedOrNotMap.contains( cmd))
    {
        if( rpcReceivedOrNotMap.value( cmd) == true)
        {
            rpcReceivedOrNotMapSetValue( cmd, false);
            emit rpcPosted(cmd);
        }
        else
        {
            // 如果标识为未返回 则不重复排入事件队列
            return;
        }
    }
    else
    {
        rpcReceivedOrNotMapSetValue( cmd, false);
        emit rpcPosted(cmd);
    }

}

QString getShowName(QString name)
{
    if( name == ASSET_NAME)     return SHOW_NAME;
    return name;
}

QString getAssetName(QString name)
{
    if( name == SHOW_NAME)     return ASSET_NAME;
    return name;
}
QString AmountToQString(share_type decimal,int precision)
{
	if (decimal.value == 0)
		return "0";
	int pre_chk = precision;
	int i = 0;
	while (pre_chk != 1)
	{
		if (pre_chk == 0)
			return "0";
		if (pre_chk % 10 != 0)
			return "0";
		pre_chk /= 10;
		i++;
	}
	QList<char> tmp;
	bool point_added = false;
	bool got_first_not_zero = false;
	while (decimal.value != 0)
	{
		if (i == 0)
		{
			point_added = true;
			if (got_first_not_zero)
				tmp.push_front('.');
			i--;
			got_first_not_zero = true;
			continue;
		}
		char tc = decimal.value % 10 + '0';
		if (got_first_not_zero || tc != '0')
		{
			tmp.push_front(tc);
			got_first_not_zero = true;
		}
		i--;
		decimal.value /= 10;
	}
	while (i > 0)
	{
		tmp.push_front('0');
		i--;
	}
	QString out;
	if (!point_added)
	{
		tmp.push_front('.');
		tmp.push_front('0');
	}
	for (auto it : tmp)
		out.push_back(it);
	return out;
}
double roundDown(double decimal, int precision)
{
    int precisonFigureNum   = QString::number( precision, 'g', 15).count() - 1;

    double result = QString::number(decimal,'f',precisonFigureNum).toDouble();
    if( result > decimal)
    {
        if( precision == 0)     precision = 1;
        result = result - 1.0 / precision;
    }

    return result;
}
bool isExistInWallet(QString strName)
{
    mutexForAddressMap.lock();
    for (QMap<QString,TwoAddresses>::const_iterator i = Blockchain::getInstance()->addressMap.constBegin(); i != Blockchain::getInstance()->addressMap.constEnd(); ++i)
    {
        if(i.key() == strName)
        {
            mutexForAddressMap.unlock();
            return true;
        }
    }
    mutexForAddressMap.unlock();
    return false;
}
QString removeLastZeros(QString number)     // 去掉小数最后面的0
{
    if( !number.contains('.'))  return number;
    while (number.endsWith('0'))
    {
        number.chop(1);
    }
    if( number.endsWith('.'))   number.chop(1);
    return number;
}
QString getBigNumberString(unsigned long long number, unsigned long long precision)
{
    QString str = QString::number(number);
    int size = QString::number(precision).size() - 1;
    if( size < 0)  return "";
    if( size == 0) return str;
    if( str.size() > size)
    {
        str.insert(str.size() - size, '.');
    }
    else
    {
        QString zeros;
        zeros.fill('0',size - str.size() + 1);
        str.insert(0,zeros);
        str.insert(1,'.');
    }
    return removeLastZeros(str);
}

bool checkAddress(QString address)
{
    if( address.size() == (32 + QString(ADDRESS_PREFIX).size()) || address.size() == (33 + QString(ADDRESS_PREFIX).size()) )
    {
        if( address.startsWith(ADDRESS_PREFIX))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool checkSignatureEnough(QString builder)
{
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(builder.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if( parse_doucment.isObject())
        {
            QJsonObject jsonObject = parse_doucment.object();
            if( jsonObject.contains("optional_signatures_enough"))
            {
                bool isEnough = jsonObject.take("optional_signatures_enough").toBool();
                return isEnough;
            }

        }
    }

    return false;
}


TransactionDetail getDetail(TransactionInfo info, QString accountName, QString _assetName)
{
    TransactionDetail result;
//    QVector<Entry>  myEntries = getMyEntries(info.entries,accountName);
    QVector<Entry>  myEntries = info.entries;

    QString trxId = info.trxId;
    int trxType = Blockchain::getInstance()->transactionOperationsMap.value(trxId).type;

    switch (trxType) {
    case -1:
    {
        if(myEntries.size() > 1)
        {
            result.type = QObject::tr("market deal");

            Entry entry = myEntries.at(1);
            AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);

            if(assetInfo.symbol != _assetName)
            {
                result.isAssetRelated = false;
            }

            QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
            result.amountVector.append(amountStr);
        }

    }
        break;
    case 0:
        if( info.blockNum == 0 && info.isConfirmed)
        {
            result.type = QObject::tr("GENESIS");

            Entry entry = myEntries.at(0);
            AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
            QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
            result.amountVector.append(amountStr);
        }
        else
        {
            result.type = QObject::tr("unkown");

            foreach (Entry entry, myEntries)
            {
                if(entry.fromAccount == accountName)
                {
                    AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
                    QString amountStr;
                    if(entry.toAccount == accountName || entry.amount.amount == 0)
                    {
                        amountStr = AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                    }
                    else
                    {
                        amountStr = "-" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                    }
                    result.amountVector.append(amountStr);
                    continue;
                }

                if(entry.toAccount == accountName)
                {
                    AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
                    QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                    result.amountVector.append(amountStr);
                }
            }
        }

        break;
    case 1:
    {
        result.type = QObject::tr("update permissions");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

    }
        break;
    case 2:
    {
        result.type = QObject::tr("create asset");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);
    }
        break;
    case 3:
    {
        result.type = QObject::tr("airdrop");

        share_type inAmount = 0;
        int assetId = -1;
        foreach (Entry entry, myEntries)
        {
            if(entry.toAccount == accountName)
            {
                inAmount += entry.amount.amount;
                assetId = entry.amount.assetId;
            }

        }

        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(assetId);

        if(info.fee > 0)         // 如果是空投发起者  显示手续费
        {
            QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
            result.amountVector.append(amountStr);
        }
        else if(_assetName == ASSET_NAME)
        {
            result.isAssetRelated = false;
        }


        if(assetId > 0)
        {
            QString amountStr = "+" + AmountToQString(inAmount , assetInfo.precision) + " " + assetInfo.symbol;
            result.amountVector.append(amountStr);
        }
    }
        break;
    case 4:
    {
        result.type = QObject::tr("transfer to asset");

        share_type inAmount = 0;
        share_type outAmount = 0;
        int assetId = -1;
        foreach (Entry entry, myEntries)
        {
            if(entry.toAccount == accountName)
            {
                inAmount += entry.amount.amount;
                assetId = entry.amount.assetId;
            }

            if(entry.fromAccount == accountName)
            {
                if(entry.amount.assetId == 0)   continue;
                outAmount += entry.amount.amount;
                assetId = entry.amount.assetId;
            }
        }

        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(assetId);


        if(info.fee > 0)         // 如果是分红发起者  显示手续费
        {
            QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
            result.amountVector.append(amountStr);

            amountStr = "-" + AmountToQString(outAmount , assetInfo.precision) + " " + assetInfo.symbol;
            result.amountVector.append(amountStr);
        }
        else if(_assetName == ASSET_NAME)
        {
            result.isAssetRelated = false;
        }


        if(assetId > 0)
        {
            QString amountStr = "+" + AmountToQString(inAmount , assetInfo.precision) + " " + assetInfo.symbol;
            result.amountVector.append(amountStr);
        }

    }
        break;
    case 5:
        if(myEntries.size() == 1)
        {
            Entry entry = myEntries.at(0);

            AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
            if( entry.toAccount == entry.fromAccount)
            {
                result.type = QObject::tr("transfer to self");

                QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
                result.amountVector.append(amountStr);

                amountStr = AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                result.amountVector.append(amountStr);
            }
            else if( entry.fromAccount == accountName)
            {
                result.type = QObject::tr("transfer-out");

                QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
                result.amountVector.append(amountStr);
                share_type transfer_amou=entry.amount.amount;
                if(assetInfo.symbol==ASSET_NAME)
                    transfer_amou-=info.fee;

                amountStr = "-" + AmountToQString(transfer_amou , assetInfo.precision) + " " + assetInfo.symbol;
                result.amountVector.append(amountStr);
            }
            else
            {
                result.type = QObject::tr("transfer-in");
                QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                result.amountVector.append(amountStr);

                if( assetInfo.symbol != ASSET_NAME && _assetName == ASSET_NAME)
                {
                    result.isAssetRelated = false;
                }
            }
        }

        break;
    case 6:
    {
        result.type = QObject::tr("register account");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);
    }
        break;
    case 7:
        result.type = QObject::tr("issue asset");

        if(myEntries.size() == 2)
        {
            Entry entry = myEntries.at(0);
            if(entry.fromAccount == accountName)
            {
                QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
                result.amountVector.append(amountStr);
            }
            else if(_assetName == ASSET_NAME)
            {
                result.isAssetRelated = false;
            }

            entry = myEntries.at(1);
            AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
            if(entry.toAccount == accountName)
            {
                QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                result.amountVector.append(amountStr);
            }
            else if(_assetName != ASSET_NAME)
            {
                result.isAssetRelated = false;
            }
        }
        else if(myEntries.size() == 1)
        {
            Entry entry = myEntries.at(0);
			if (entry.fromAccount == accountName)
			{
				if (_assetName == ASSET_NAME)
				{
					QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
					result.amountVector.append(amountStr);
				}
			}
			result.isAssetRelated = true;
            

            AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
            if(entry.toAccount == accountName)
            {
                QString amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
                result.amountVector.append(amountStr);
            }
			if(result.amountVector.size()==0)
				result.isAssetRelated = false;

        }

        break;
    case 8:
    {
        result.type = QObject::tr("market withdraw");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

        Entry entry = myEntries.at(0);
        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
        amountStr = "+" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
        result.amountVector.append(amountStr);
    }
        break;
    case 9:
    {
        result.type = QObject::tr("market bid");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

        Entry entry = myEntries.at(0);
        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
        amountStr = "-" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
        result.amountVector.append(amountStr);
    }
        break;
    case 10:
    {
        result.type = QObject::tr("market ask");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

        Entry entry = myEntries.at(0);
        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
        amountStr = "-" + AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
        result.amountVector.append(amountStr);
    }
        break;
    case 11:
    {
        result.type = QObject::tr("edit admins");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

    }
        break;
    case 12:
    {
        result.type = QObject::tr("halt market");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

    }
        break;
    case 13:
    {
        result.type = QObject::tr("recover market");

        QString amountStr = "-" + AmountToQString(info.fee , 100000) + " " + ASSET_NAME;
        result.amountVector.append(amountStr);

    }
        break;
    default:
        break;
    }

    return result;
}

QVector<Entry> getMyEntries(QVector<Entry> entries, QString accountName)
{
    QVector<Entry> result;
    foreach (Entry entry, entries)
    {
        if( entry.fromAccount == accountName)
        {
            result.append(entry);
            continue;
        }

        if( entry.toAccount == accountName)    result.append(entry);
    }

    return result;
}
