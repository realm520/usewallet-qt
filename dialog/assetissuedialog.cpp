#include "assetissuedialog.h"
#include "ui_assetissuedialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"


AssetIssueDialog::AssetIssueDialog( int assetId, QWidget *parent) :
    QDialog(parent),
    id(assetId),
    ui(new Ui::AssetIssueDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");



    ui->addressLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->addressLineEdit->setTextMargins(8,0,0,0);
    QRegExp regx("[a-zA-Z0-9\-\.\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->addressLineEdit->setValidator( validator );
    ui->addressLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->remarkLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->remarkLineEdit->setTextMargins(8,0,0,0);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));



    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    ui->symbolLabel->setText( info.symbol);
    ui->leftAmountLabel->setText( QString::number( (info.maxSupply - info.currentSupply)/ info.precision,'g',15));

    int precisonNum   = QString::number( info.precision, 'g', 15).count() - 1;

    ui->amountLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->amountLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,16})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(precisonNum));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
}

AssetIssueDialog::~AssetIssueDialog()
{
    delete ui;
}

void AssetIssueDialog::pop()
{
    move(0,0);
    exec();
}

void AssetIssueDialog::on_okBtn_clicked()
{
    if( ui->addressLineEdit->text().simplified().isEmpty())     return;
    if( ui->amountLineEdit->text().simplified().isEmpty())      return;

    QString builderPath = Blockchain::getInstance()->getBuilderFilePath("UIAIssue_" + ui->symbolLabel->text());
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_uia_issue-" + builderPath, "wallet_uia_issue", QStringList() << ui->amountLineEdit->text()
                                               << ui->symbolLabel->text() << ui->addressLineEdit->text() << ui->remarkLineEdit->text()
                                               << builderPath << "true"));

}

void AssetIssueDialog::on_cancelBtn_clicked()
{
    close();
}

void AssetIssueDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_uia_issue-"))
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        QString buildPath = id.mid( QString("id_wallet_uia_issue-").size());

        if( result.startsWith("\"result\":"))
        {
            QString builderStr = result.mid(9);
            if( checkSignatureEnough(builderStr))
            {
                Blockchain::getInstance()->scanLater();

                int pos = result.indexOf("\"record_id\":") + 13;
                QString txId = result.mid( pos, result.indexOf( "\"", pos ) - pos );
                Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_scan_transaction", "wallet_scan_transaction", QStringList() << txId << "true" ));

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
            commonDialog.setText( tr("Fail: ") , errorMessage);
            commonDialog.pop();
        }


        return;
    }

    if( id == "id_wallet_scan_transaction")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        qDebug() << result;

        return;
    }

}

void AssetIssueDialog::on_amountLineEdit_textChanged(const QString &arg1)
{

}
