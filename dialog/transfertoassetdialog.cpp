#include "transfertoassetdialog.h"
#include "ui_transfertoassetdialog.h"

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

#include <QDebug>

TransferToAssetDialog::TransferToAssetDialog(int assetId, QWidget *parent) :
    QDialog(parent),
    id(assetId),
    ui(new Ui::TransferToAssetDialog)
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

    ui->targetAssetLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->targetAssetLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx1("[A-Z]+$");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->targetAssetLineEdit->setValidator(pReg1);
    ui->targetAssetLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->amountLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->amountLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx2("^([0]|[1-9][0-9]{0,16})(?:\\.\\d{1,5})?$|(^\\t?$)");
    QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
    ui->amountLineEdit->setValidator(pReg2);
    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    init();
}

TransferToAssetDialog::~TransferToAssetDialog()
{
    delete ui;
}

void TransferToAssetDialog::pop()
{
    move(0,0);
    exec();
}

void TransferToAssetDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_transfer_to_asset") )
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

qDebug() << id << result;
        if( result.startsWith("\"result\":"))
        {
            close();
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("The transaction has been sent out!"));
            commonDialog.pop();

        }
        else if( result.startsWith("\"error\":"))
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);


            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Fail: ") , errorMessage);
            commonDialog.pop();
        }

        return;
    }
}

void TransferToAssetDialog::on_okBtn_clicked()
{
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_transfer_to_asset", "wallet_transfer_to_asset", QStringList() << ui->accountComboBox->currentText()
                                               << ui->amountLineEdit->text() << ui->symbolLabel->text() << ui->targetAssetLineEdit->text()
                                               ));
}

void TransferToAssetDialog::on_cancelBtn_clicked()
{
    close();
}

void TransferToAssetDialog::init()
{
    QStringList keys = Blockchain::getInstance()->addressMap.keys();
    ui->accountComboBox->addItems( keys);

    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);

    ui->symbolLabel->setText(info.symbol);
    ui->balanceLabel->setText(AmountToQString( Blockchain::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText()).value(id) , info.precision) );
}

void TransferToAssetDialog::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    ui->balanceLabel->setText(AmountToQString( Blockchain::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText()).value(id) , info.precision) );
}
