#include "issuetoassetdialog.h"
#include "ui_issuetoassetdialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

IssueToAssetDialog::IssueToAssetDialog(int assetId, QWidget *parent) :
    QDialog(parent),
    id(assetId),
    ui(new Ui::IssueToAssetDialog)
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

IssueToAssetDialog::~IssueToAssetDialog()
{
    delete ui;
}

void IssueToAssetDialog::pop()
{
    move(0,0);
    exec();
}

void IssueToAssetDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_uia_issue_to_asset-") )
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        QString buildPath = id.mid( QString("id_wallet_uia_issue_to_asset-").size());

qDebug() << id << result;
        if( result.startsWith("\"result\":"))
        {

            QString builderStr = result.mid(9);
            if( checkSignatureEnough(builderStr))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText(tr("Transaction has been sent out."));
                commonDialog.pop();
            }
            else
            {
                CommonDialog commonDialog(CommonDialog::YesOrNo);
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
            commonDialog.setText( tr("Fail: ") + errorMessage);
            commonDialog.pop();
        }

        return;
    }
}

void IssueToAssetDialog::on_okBtn_clicked()
{
    QString builderPath = Blockchain::getInstance()->getBuilderFilePath("IssueToAsset_" + ui->symbolLabel->text() + "_to_" + ui->targetAssetLineEdit->text());
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_uia_issue_to_asset-" + builderPath, "wallet_uia_issue_to_asset", QStringList() << ui->accountComboBox->currentText()
                                               << ui->symbolLabel->text() << ui->targetAssetLineEdit->text() << ui->amountLineEdit->text()
                                               << builderPath << "true"));

}

void IssueToAssetDialog::on_cancelBtn_clicked()
{
    close();
}

void IssueToAssetDialog::init()
{
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    QStringList myAddresses = Blockchain::getInstance()->getMyAddresses(info.owners);
    foreach (QString address, myAddresses)
    {
        ui->accountComboBox->addItem(Blockchain::getInstance()->addressToName(address));
    }

    ui->symbolLabel->setText(info.symbol);

    ui->targetAssetLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->targetAssetLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx1("[A-Z]+$");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->targetAssetLineEdit->setValidator(pReg1);
    ui->targetAssetLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    int precisonNum   = QString::number( info.precision, 'g', 15).count() - 1;

    ui->amountLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->amountLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx2(QString("^([0]|[1-9][0-9]{0,16})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(precisonNum));
    QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
    ui->amountLineEdit->setValidator(pReg2);
    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
}
