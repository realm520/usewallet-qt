﻿#include <QDebug>
#include <QPainter>
#include <QTextCodec>
#include <QDir>
#ifdef WIN32
#include <windows.h>
#endif

#include "transferpage.h"
#include "ui_transferpage.h"
#include "blockchain.h"
#include "debug_log.h"
#include "contactdialog.h"
#include "remarkdialog.h"
#include "commondialog.h"
#include "transferconfirmdialog.h"
#include "rpcthread.h"


TransferPage::TransferPage(QString name,QWidget *parent) :
    QWidget(parent),
    accountName(name),
    inited(false),
    assetUpdating(false),
    ui(new Ui::TransferPage)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(244,244,242));
    setPalette(palette);

    QString language = Blockchain::getInstance()->language;
    if( language.isEmpty() || language == "Simplified Chinese")
    {
        ui->addContactBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/contactBtn2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                         "QToolButton:hover{background-image:url(:/pic/pic2/contactBtn2_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

    }
    else
    {
        ui->addContactBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/contactBtn2_En.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                         "QToolButton:hover{background-image:url(:/pic/pic2/contactBtn2_hover_En.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
//        ui->tipLabel2->setPixmap(QPixmap("pic2/feeTip_En.png"));
//        ui->tipLabel2->setGeometry(425,260,229,96);
    }
//        delegateLabelString = "pic2/delegateLabel_En.png";
//        registeredLabelString = "pic2/registeredLabel_En.png";
//    }


//    if( name.isEmpty())
//    {
//        Fry::getInstance()->configFile->beginGroup("/accountInfo");
//        accountName = Fry::getInstance()->configFile->value( Fry::getInstance()->configFile->childKeys().at(0)).toString();
//        Fry::getInstance()->configFile->endGroup();
//    }

//    Fry::getInstance()->configFile->beginGroup("/accountInfo");
//    QStringList keys = Fry::getInstance()->configFile->childKeys();

//    for( int i = 0; i < keys.size(); i++)
//    {
//        ui->accountComboBox->addItem( Fry::getInstance()->configFile->value( keys.at(i)).toString());
//    }
//    Fry::getInstance()->configFile->endGroup();
//    ui->accountComboBox->setCurrentText(name);

    if( accountName.isEmpty())  // 如果是点击账单跳转
    {
        if( Blockchain::getInstance()->addressMap.size() > 0)
        {
            accountName = Blockchain::getInstance()->addressMap.keys().at(0);
        }
        else  // 如果还没有账户
        {
            emit back();    // 跳转在functionbar  这里并没有用
            return;
        }
    }

    // 账户下拉框按字母顺序排序
    QStringList keys = Blockchain::getInstance()->addressMap.keys();
    ui->accountComboBox->addItems( keys);
    if( accountName.isEmpty() )
    {
        ui->accountComboBox->setCurrentIndex(0);
    }
    else
    {
        ui->accountComboBox->setCurrentText( accountName);
    }

    QString showName = Blockchain::getInstance()->addressMapValue(accountName).ownerAddress;
    ui->addressLabel->setText(showName);

    ui->amountLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->amountLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx1("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{1,5})?$|(^\\t?$)");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->sendtoLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->sendtoLineEdit->setTextMargins(8,0,0,0);
    QRegExp regx("[a-zA-Z0-9\-\.\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->sendtoLineEdit->setValidator( validator );
    ui->sendtoLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->feeLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->feeLineEdit->setTextMargins(8,0,0,0);
    ui->feeLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->feeLineEdit->setReadOnly(true);
    updateTransactionFee();

//    QRegExp rx("^([0]|[1-9][0-9]{0,5})(?:\\.\\d{1,4})?$|(^\\t?$)");
//    QRegExpValidator *pReg = new QRegExpValidator(rx, this);
//    ui->feeLineEdit->setValidator(pReg);

    ui->messageLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->messageLineEdit->setTextMargins(8,0,0,0);

    ui->tipLabel3->hide();
    ui->tipLabel4->hide();
    ui->tipLabel5->hide();
    ui->tipLabel6->hide();

    ui->tipLabel5->setPixmap(QPixmap(":/pic/pic2/wrong.png"));

    on_amountLineEdit_textChanged(ui->amountLineEdit->text());

#ifdef WIN32
    ui->accountComboBox->setStyleSheet("QComboBox {border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                                           "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                                                                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                                                                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                                           "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                                           );

    ui->assetComboBox->setStyleSheet("QComboBox{border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                  "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                  "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                  );
#endif

    getContactsList();
    getAssets();
    ui->assetComboBox->setCurrentText(Blockchain::getInstance()->currentAsset);

    inited = true;

	DLOG_QT_WALLET_FUNCTION_END;
}

TransferPage::~TransferPage()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;

	DLOG_QT_WALLET_FUNCTION_END;
}



void TransferPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( !inited)  return;


    getBalance();

    accountName = arg1;
    Blockchain::getInstance()->currentAccount = accountName;
    QString showName = Blockchain::getInstance()->addressMapValue(accountName).ownerAddress;
    ui->addressLabel->setText(showName);

    on_amountLineEdit_textChanged(ui->amountLineEdit->text());

	DLOG_QT_WALLET_FUNCTION_END;
}


void TransferPage::on_sendBtn_clicked()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;    

    if(ui->amountLineEdit->text().size() == 0 || ui->sendtoLineEdit->text().size() == 0)
    {      
        CommonDialog tipDialog(CommonDialog::OkOnly);
//        tipDialog.setText( QString::fromLocal8Bit("请输入转账金额和地址"));
        tipDialog.setText( tr("Please enter the amount and address."));
        tipDialog.pop();
        return;
    }

    if( ui->amountLineEdit->text().toDouble()  <= 0)
    {    
        CommonDialog tipDialog(CommonDialog::OkOnly);
//        tipDialog.setText( QString::fromLocal8Bit("金额不能为0"));
        tipDialog.setText( tr("The amount can not be 0"));
        tipDialog.pop();
        return;
    }

    if( ui->feeLineEdit->text().toDouble() <= 0)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("The fee can not be 0"));
        tipDialog.pop();
        return;
    }


    QString remark = ui->messageLineEdit->text();
//    remark.remove(' ');
    if( remark.size() == 0)    // 转地址如果没有备注 会自动添加 TO GOPDD...   所以添加空格
    {
        remark = " ";
    }

    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(remark);
    if( ba.size() > 40)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("Message length more than 40 bytes!"));
        tipDialog.pop();
        return;
    }

    TransferConfirmDialog transferConfirmDialog( ui->sendtoLineEdit->text(), ui->amountLineEdit->text(), ui->feeLineEdit->text(), remark, ui->assetComboBox->currentText());
//    transferConfirmDialog.move( this->mapToGlobal(QPoint( -28, 35)));
//    emit showShadowWidget();
    bool yOrN = transferConfirmDialog.pop();
//    emit hideShadowWidget();
    if( yOrN)
    {

//        QString str = "wallet_set_transaction_fee " + ui->feeLineEdit->text() + '\n';
//        Hcash::getInstance()->write(str);
//        QString result = Hcash::getInstance()->read();

//        if( ui->sendtoLineEdit->text().mid(0,3) == ASSET_NAME)
        if( !ui->sendtoLineEdit->text().isEmpty())
        {

            int assetIndex = ui->assetComboBox->currentIndex();
            if( assetIndex < 0) assetIndex = 0;
            AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(assetIndex);

            Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_transfer_to_address_" + accountName, "wallet_transfer_to_address",
                                                          QStringList() << ui->amountLineEdit->text() << info.symbol << accountName
                                                          << ui->sendtoLineEdit->text() << remark ));
//qDebug() << toJsonFormat( "id_wallet_transfer_to_address_" + accountName, "wallet_transfer_to_address",
//                          QStringList() << ui->amountLineEdit->text() << ASSET_NAME << accountName
//                          << ui->sendtoLineEdit->text() << remark );
        }
//        else
//        {

//            Fry::getInstance()->postRPC( toJsonFormat( "id_wallet_transfer_to_public_account_" + accountName, "wallet_transfer_to_public_account",
//                                                          QStringList() << ui->amountLineEdit->text() << ASSET_NAME << accountName
//                                                          << ui->sendtoLineEdit->text() << remark ));

//            qDebug() << "id_wallet_transfer_to_public_account_" + accountName;

//        }



    }

	DLOG_QT_WALLET_FUNCTION_END;
}



void TransferPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(228,228,228),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(247,246,242),Qt::SolidPattern));
    painter.drawRect(-1,-1,858,68);

}

void TransferPage::on_amountLineEdit_textChanged(const QString &arg1)
{
    double amount = ui->amountLineEdit->text().toDouble();
    double fee = ui->feeLineEdit->text().toDouble();
    QString strBalanceTemp = Blockchain::getInstance()->balanceMapValue(accountName).remove(",");
    strBalanceTemp = strBalanceTemp.remove(" " + QString(ASSET_NAME));
    double dBalance = strBalanceTemp.remove(",").toDouble();

        if( amount + fee > dBalance)
    {
        ui->tipLabel3->show();
        ui->tipLabel5->show();

        ui->sendBtn->setEnabled(false);
    }
    else
    {
        ui->tipLabel3->hide();
        ui->tipLabel5->hide();

        ui->sendBtn->setEnabled(true);
    }
}

void TransferPage::on_sendtoLineEdit_textChanged(const QString &arg1)
{
    if( ui->sendtoLineEdit->text().contains(" ") || ui->sendtoLineEdit->text().contains("\n"))   // 不判断就remove的话 右键菜单撤销看起来等于不能用
    {
        ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().simplified().remove(" "));
    }

//    ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().remove("\n"));
    if( ui->sendtoLineEdit->text().isEmpty() || checkAddress(ui->sendtoLineEdit->text()) )
    {
        ui->tipLabel4->hide();
        return;
    }

//    if( ui->sendtoLineEdit->text().toInt() == 0)   // 不能是纯数字
//    {
//        FBTC::getInstance()->postRPC( toJsonFormat( "id_blockchain_get_account_" + ui->sendtoLineEdit->text(), "blockchain_get_account", QStringList() << ui->sendtoLineEdit->text() ));
//    }
//    else
//    {
        ui->tipLabel4->setText(tr("Invalid add."));
        ui->tipLabel4->show();
//    }

//    QString str;
//    if( ui->sendtoLineEdit->text().toInt() == 0)   // 防止输入数字
//    {
//        str = "blockchain_get_account " + ui->sendtoLineEdit->text() + '\n';
//    }
//    else
//    {
//        str = "blockchain_get_account WRONG \n";
//    }
//    Fry::getInstance()->write(str);
//    QString result = Fry::getInstance()->read();
//    DLOG_QT_WALLET(": %s", result.toStdString().c_str());
//    result = result.left(BLOCKCHAIN_GET_ACCOUNT_RES_LENGTH);
//    DLOG_QT_WALLET(": %s", result.toStdString().c_str());
//    if( result != RES_CODE_UNKNOW_BLOCK_CHAIN_ACCOUNT)
//    {
//        ui->tipLabel4->setText(tr("<body><font color=green>Valid add.</font></body>"));
//        ui->tipLabel4->show();
//    }
//    else
//    {
//        ui->tipLabel4->setText(tr("Invalid add."));
//        ui->tipLabel4->show();
//    }
}

void TransferPage::refresh()
{
    getBalance();
}

void TransferPage::on_addContactBtn_clicked()
{
    ContactDialog contactDialog(this);
    connect(&contactDialog,SIGNAL(contactSelected(QString,QString)), this, SLOT(contactSelected(QString,QString)));
    contactDialog.move( ui->addContactBtn->mapToGlobal( QPoint(0,24)));
    contactDialog.exec();
}

void TransferPage::contactSelected(QString remark, QString contact)
{
    ui->contactLabel->setText(remark);
    ui->sendtoLineEdit->setText(contact);
}


void TransferPage::getContactsList()
{
    if( !Blockchain::getInstance()->contactsFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "contact.dat not exist";
        contactsList.clear();
        return;
    }
    QString str = QByteArray::fromBase64( Blockchain::getInstance()->contactsFile->readAll());
    QStringList strList = str.split(";");
    strList.removeLast();
    int size = strList.size();

    for( int i = 0; i < size; i++)
    {
        QString str2 = strList.at(i);
        contactsList += str2.left( str2.indexOf("="));
    }


    Blockchain::getInstance()->contactsFile->close();
}

QString TransferPage::getCurrentAccount()
{
    return accountName;
}

void TransferPage::setAddress(QString address)
{
    ui->sendtoLineEdit->setText(address);
}

void TransferPage::setContact(QString contactRemark)
{
    ui->contactLabel->setText(contactRemark);
}
void TransferPage::getAssets()
{
    assetUpdating = true;

    int index = ui->assetComboBox->currentIndex();
    ui->assetComboBox->clear();
    foreach (int key, Blockchain::getInstance()->assetInfoMap.keys())
    {
        ui->assetComboBox->addItem( Blockchain::getInstance()->assetInfoMap.value(key).symbol);
    }

    if( index < 0 )   index = 0;
    ui->assetComboBox->setCurrentIndex(index);

    assetUpdating = false;
}

void TransferPage::getBalance()
{
    int assetIndex = ui->assetComboBox->currentIndex();
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(assetIndex);
    if( assetIndex < 0) assetIndex = 0;

    AssetBalanceMap map = Blockchain::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText());
    double balance;
    if( map.contains(assetIndex))
    {
        balance = map.value(assetIndex);
    }
    else
    {
        balance = 0;
    }
    balance = balance / info.precision;

    ui->balanceLabel->setText( "<body><font style=\"font-size:18px\" color=#000000>" + QString::number(balance,'g',15) + "</font><font style=\"font-size:12px\" color=#000000> " + info.symbol + "</font></body>" );
    ui->balanceLabel->adjustSize();
    ui->balanceLabel->move( 783 - ui->balanceLabel->width(),33);
    ui->balanceLabel2->move( 710 - ui->balanceLabel->width(),33);

    ui->amountAssetLabel->setText(info.symbol);

}

void TransferPage::updateTransactionFee()
{
    ui->feeLineEdit->setText( getBigNumberString(Blockchain::getInstance()->transactionFee,100000));
}

void TransferPage::jsonDataUpdated(QString id)
{
    if( id == "id_wallet_transfer_to_address_" + accountName
        || id == "id_wallet_transfer_to_public_account_" + accountName)
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
qDebug() << id << result;
        if( result.mid(0,18) == "\"result\":{\"index\":")             // 成功
        {
            QString recordId = result.mid( result.indexOf("\"record_id\"") + 13, 40);

            mutexForPendingFile.lock();

            mutexForConfigFile.lock();
            Blockchain::getInstance()->configFile->setValue("/recordId/" + recordId , 0);
            mutexForConfigFile.unlock();

            if( !Blockchain::getInstance()->pendingFile->open(QIODevice::ReadWrite))
            {
                qDebug() << "pending.dat open fail";
                return;
            }

            QByteArray ba = QByteArray::fromBase64( Blockchain::getInstance()->pendingFile->readAll());
            ba += QString( recordId + "," + accountName + "," + ui->sendtoLineEdit->text() + "," + ui->amountLineEdit->text() + "," + ui->feeLineEdit->text() + ";").toUtf8();
            ba = ba.toBase64();
            Blockchain::getInstance()->pendingFile->resize(0);
            QTextStream ts(Blockchain::getInstance()->pendingFile);
            ts << ba;

            Blockchain::getInstance()->pendingFile->close();

            mutexForPendingFile.unlock();

            CommonDialog tipDialog(CommonDialog::OkOnly);
            tipDialog.setText( tr("Transaction has been sent,please wait for confirmation"));
            tipDialog.pop();

            if( !contactsList.contains( ui->sendtoLineEdit->text()))
            {
                CommonDialog addContactDialog(CommonDialog::OkAndCancel);
                addContactDialog.setText(tr("Add this address to contacts?"));
                if( addContactDialog.pop())
                {
                    RemarkDialog remarkDialog( ui->sendtoLineEdit->text());
                    remarkDialog.pop();
                    getContactsList();
                }
            }

            emit showAccountPage(accountName);


        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);
            qDebug() << "errorMessage : " << errorMessage;

            if( errorMessage == "Assert Exception")
            {
                if( result.contains("\"format\":\"my->is_receive_account( from_account_name ): Invalid account name\","))
                {
                    CommonDialog tipDialog(CommonDialog::OkOnly);
                    tipDialog.setText( tr("This name has been registered, please rename this account!"));
                    tipDialog.pop();
                }
                else
                {
                    CommonDialog tipDialog(CommonDialog::OkOnly);
                    tipDialog.setText( tr("Wrong address!"));
                    tipDialog.pop();
                }


            }
            else if( errorMessage == "imessage size bigger than soft_max_lenth")
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Message too long!"));
                tipDialog.pop();

            }
            else if( errorMessage == "invalid transaction expiration")
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Failed: You need to wait for synchronization to complete"));
                tipDialog.pop();
            }
            else if( errorMessage == "insufficient funds")
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Not enough balance!"));
                tipDialog.pop();
            }
            else if( errorMessage == "Out of Range")
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Wrong address!"));
                tipDialog.pop();
            }
            else if( errorMessage == "Parse Error")
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Wrong address!"));
                tipDialog.pop();
            }
            else
            {
                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr("Transaction sent failed"));
                tipDialog.pop();
            }

        }
        return;
    }

//    if( id.mid(0,26) == "id_blockchain_get_account_")
//    {
        // 如果跟当前输入框中的内容不一样，则是过时的rpc返回，不用处理
//        if( id.mid(26) != ui->sendtoLineEdit->text())  return;
//        QString result = FBTC::getInstance()->jsonDataValue(id);

//        if( result != "\"result\":null")
//        {
//            ui->tipLabel4->setText(tr("<body><font color=green>Valid add.</font></body>"));
//            ui->tipLabel4->show();
//        }
//        else
//        {
//            ui->tipLabel4->setText(tr("Invalid add."));
//            ui->tipLabel4->show();
//        }

//        return;
//    }
}

void TransferPage::on_feeLineEdit_textChanged(const QString &arg1)
{
    on_amountLineEdit_textChanged("");
}

void TransferPage::on_messageLineEdit_textChanged(const QString &arg1)
{
    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(arg1);
    if( ba.size() > 40)
    {
        ui->tipLabel6->show();
    }
    else
    {
        ui->tipLabel6->hide();
    }
}

void TransferPage::on_assetComboBox_currentIndexChanged(int index)
{
    if( assetUpdating)  return;
    getBalance();
    Blockchain::getInstance()->currentAsset = ui->assetComboBox->currentText();
}