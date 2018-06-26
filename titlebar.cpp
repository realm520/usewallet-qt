#include <QDebug>

#include "titlebar.h"
#include "ui_titlebar.h"
#include "debug_log.h"
#include <QPainter>
#include "setdialog.h"
#include "consoledialog.h"
#include "blockchain.h"
#include "rpcthread.h"
#include "commondialog.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);    

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(35,27,55));
    setPalette(palette);


    ui->minBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/minimize2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                              "QToolButton:hover{background-image:url(:/pic/pic2/minimize_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/close2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                "QToolButton:hover{background-image:url(:/pic/pic2/close_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

    ui->menuBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/set.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->lockBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/lockBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    timer->setInterval(10000);
    timer->start();

    ui->versionLabel->setText( QString(ASSET_NAME"  v") + WALLET_VERSION);

    onTimeOut();
	DLOG_QT_WALLET_FUNCTION_END;
}

TitleBar::~TitleBar()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    delete ui;

	DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_minBtn_clicked()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( Blockchain::getInstance()->minimizeToTray)
    {
        emit tray();
    }
    else
    {  
        emit minimum();
    }

	DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_closeBtn_clicked()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( Blockchain::getInstance()->closeToMinimize)
    {
        emit tray();
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkAndCancel);
        commonDialog.setText( tr( "Sure to close the Wallet?"));
        bool choice = commonDialog.pop();

        if( choice)
        {
            emit closeWallet();
        }
        else
        {
            return;
        }

    }

	DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_menuBtn_clicked()
{
    SetDialog setDialog;
    connect(&setDialog,SIGNAL(settingSaved()),this,SLOT(saved()));
    setDialog.pop();

}

void TitleBar::saved()
{
    emit settingSaved();
}

void TitleBar::retranslator()
{
    ui->retranslateUi(this);
}

void TitleBar::jsonDataUpdated(QString id)
{
    if( id == "id_blockchain_list_pending_transactions")
    {
        QString pendingTransactions = Blockchain::getInstance()->jsonDataValue(id);
        // 查询一遍 config中记录的交易ID
        mutexForConfigFile.lock();
        Blockchain::getInstance()->configFile->beginGroup("/recordId");
        QStringList keys = Blockchain::getInstance()->configFile->childKeys();
        Blockchain::getInstance()->configFile->endGroup();

        int numOfNews = 0;
        foreach (QString key, keys)
        {
            if( Blockchain::getInstance()->configFile->value("/recordId/" + key).toInt() == 2)
            {
                // 失效的交易
                numOfNews++;
                continue;
            }

            if( !pendingTransactions.contains(key))  // 如果不在pending区, 看看是否在链上
            {
                Blockchain::getInstance()->postRPC( toJsonFormat( "id_blockchain_get_transaction_" + key, "blockchain_get_transaction", QStringList() << key  ));
            }

            if( Blockchain::getInstance()->configFile->value("/recordId/" + key).toInt() == 1)
            {
                numOfNews++;
            }
        }
        mutexForConfigFile.unlock();


        return;
    }

    if( id.startsWith("id_blockchain_get_transaction"))
    {

        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result.mid(0,22).contains("exception") || result.mid(0,22).contains("error"))
        {
            // 若不在pending区也不在链上  则为失效交易  recordId置为2
            mutexForConfigFile.lock();

            Blockchain::getInstance()->configFile->setValue("/recordId/" + id.right(40), 2);

            mutexForConfigFile.unlock();

            return;
        }
        else   //  如果已经被打包进区块，则将config中记录置为1
        {
            mutexForConfigFile.lock();

            Blockchain::getInstance()->configFile->setValue("/recordId/" + id.right(40), 1);

            mutexForConfigFile.unlock();
        }

        return;
    }

}

void TitleBar::onTimeOut()
{
//    RpcThread* rpcThread = new RpcThread;
//    connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//    rpcThread->setWriteData( toJsonFormat( "id_blockchain_list_pending_transactions", "blockchain_list_pending_transactions", QStringList() << "" ));
//    rpcThread->start();
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_blockchain_list_pending_transactions", "blockchain_list_pending_transactions", QStringList() << "" ));

    mutexForConfigFile.lock();
    Blockchain::getInstance()->configFile->beginGroup("/applyingForDelegateAccount");
    QStringList keys = Blockchain::getInstance()->configFile->childKeys();
    Blockchain::getInstance()->configFile->endGroup();
    foreach (QString key, keys)
    {
        // 如果申请代理的recordId 被删除了 或者被确认了（=1）或者失效了（=2） 则 删除applyingForDelegateAccount的记录
        if( !Blockchain::getInstance()->configFile->contains("/recordId/" + Blockchain::getInstance()->configFile->value("/applyingForDelegateAccount/" + key).toString())
            ||  Blockchain::getInstance()->configFile->value("/recordId/" + Blockchain::getInstance()->configFile->value("/applyingForDelegateAccount/" + key).toString()).toInt() != 0 )
        {
            Blockchain::getInstance()->configFile->remove("/applyingForDelegateAccount/" + key);
        }
    }

    Blockchain::getInstance()->configFile->beginGroup("/registeringAccount");
    keys = Blockchain::getInstance()->configFile->childKeys();
    Blockchain::getInstance()->configFile->endGroup();
    foreach (QString key, keys)
    {
        // 如果注册升级的recordId 被删除了 或者被确认了（=1）或者失效了（=2） 则 删除registeringAccount的记录
        if( !Blockchain::getInstance()->configFile->contains("/recordId/" + Blockchain::getInstance()->configFile->value("/registeringAccount/" + key).toString())
            ||  Blockchain::getInstance()->configFile->value("/recordId/" + Blockchain::getInstance()->configFile->value("/registeringAccount/" + key).toString()).toInt() != 0 )
        {
            Blockchain::getInstance()->configFile->remove("/registeringAccount/" + key);
        }
    }

    mutexForConfigFile.unlock();
}



void TitleBar::on_consoleBtn_clicked()
{
    ConsoleDialog consoleDialog;
    consoleDialog.pop();

}

void TitleBar::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    painter.setBrush(QColor(0,228,255));
//    painter.setPen(QColor(0,228,255));
//    painter.drawRect(QRect(0,48,960,5));
}

void TitleBar::on_lockBtn_clicked()
{
    qDebug() << "TitleBar::on_lockBtn_clicked ";
    emit lock();
}
