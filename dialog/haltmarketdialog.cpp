#include "haltmarketdialog.h"
#include "ui_haltmarketdialog.h"

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

HaltMarketDialog::HaltMarketDialog(int assetId, bool _trueOrNot, QWidget *parent) :
    QDialog(parent),
    id(assetId),
    trueOrNot(_trueOrNot),
    ui(new Ui::HaltMarketDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->accountComboBox->setStyleSheet("QComboBox {border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                                           "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                                                                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                                                                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                                           "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                                           );

    init();
}

HaltMarketDialog::~HaltMarketDialog()
{
    delete ui;
}

void HaltMarketDialog::pop()
{
    move(0,0);
    exec();
}

void HaltMarketDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_uia_update_active_flags-") )
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        QString buildPath = id.mid( QString("id_wallet_uia_update_active_flags-").size());

qDebug() << id << result;
        if( result.startsWith("\"result\":"))
        {
            QString builderStr = result.mid(9);
            if( checkSignatureEnough(builderStr))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
//                commonDialog.setText(QString::fromLocal8Bit("交易已发出"));
                commonDialog.setText(tr("Transaction has been sent out."));
                commonDialog.pop();
            }
            else
            {
                CommonDialog commonDialog(CommonDialog::YesOrNo);
//                commonDialog.setText(QString::fromLocal8Bit("成功生成交易，是否查看transaction_builder?"));
                commonDialog.setText(tr("Transaction generated.Do you want to view the transaction_builder?"));
                if(commonDialog.pop())
                {
                    QFileInfo fileInfo(buildPath);
                    QString path = fileInfo.absolutePath();
                    QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));
                }

            }

            close();
        }
        else if( result.startsWith("\"error\":"))
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);


            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Failed: ") , errorMessage);
            commonDialog.pop();
        }

        return;
    }
}

void HaltMarketDialog::on_okBtn_clicked()
{
    QString boolStr;
    if(trueOrNot)
    {
        boolStr = "true";
    }
    else
    {
        boolStr = "false";
    }

    QString builderPath = Blockchain::getInstance()->getBuilderFilePath("HaltMarkets_" + ui->symbolLabel->text() + "_" + boolStr);
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_uia_update_active_flags-" + builderPath, "wallet_uia_update_active_flags", QStringList() << ui->accountComboBox->currentText()
                                               << ui->symbolLabel->text() << QString::number(4)  << boolStr << builderPath << "true"
                                               ));
}

void HaltMarketDialog::on_cancelBtn_clicked()
{
    close();
}

void HaltMarketDialog::init()
{
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    QStringList myAddresses = Blockchain::getInstance()->getMyAddresses(info.owners);
    foreach (QString address, myAddresses)
    {
        ui->accountComboBox->addItem(Blockchain::getInstance()->addressToName(address));
    }

    ui->symbolLabel->setText(info.symbol);
    ui->balanceLabel->setText( AmountToQString( Blockchain::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText()).value(0) , 100000) );

    QString str;
    if( trueOrNot)
    {
        str = tr("halt the market");
    }
    else
    {
        str = tr("recover the market");
    }
//    ui->inquryLabel->setText(QString::fromLocal8Bit("是否确定%1?").arg(str));
    ui->inquryLabel->setText(tr("Sure to %1?").arg(str));

}

void HaltMarketDialog::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    ui->balanceLabel->setText(AmountToQString( Blockchain::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText()).value(0) , 100000 ) + " " + ASSET_NAME );
}
