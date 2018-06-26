#include "multisigdialog.h"
#include "ui_multisigdialog.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

MultiSigDialog::MultiSigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultiSigDialog)
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


    ui->pathLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->pathLineEdit->setTextMargins(8,0,0,0);

    ui->signedTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->signedTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->signedTableWidget->setColumnWidth(0,280);
    ui->signedTableWidget->setColumnWidth(1,80);
    ui->signedTableWidget->setShowGrid(true);
    ui->signedTableWidget->setFrameShape(QFrame::NoFrame);
    ui->signedTableWidget->setMouseTracking(true);


    ui->unsignedTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->unsignedTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->unsignedTableWidget->setColumnWidth(0,280);
    ui->unsignedTableWidget->setColumnWidth(1,80);
    ui->unsignedTableWidget->setShowGrid(true);
    ui->unsignedTableWidget->setFrameShape(QFrame::NoFrame);
    ui->unsignedTableWidget->setMouseTracking(true);

    init();
}

MultiSigDialog::~MultiSigDialog()
{
    delete ui;
}

void MultiSigDialog::pop()
{
    move(0,0);
    exec();
}

void MultiSigDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_builder_file_add_signature-") )
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        QString buildPath = id.mid( QString("id_wallet_builder_file_add_signature-").size());

qDebug() << id << result;
        if( result.startsWith("\"result\":"))
        {

            QString builderStr = result.mid(9);
            if( checkSignatureEnough(builderStr))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
//                commonDialog.setText(QString::fromLocal8Bit("签名成功，交易已发出"));
                commonDialog.setText(tr("Signature(s) already added and the transaction has been sent out."));
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

void MultiSigDialog::on_okBtn_clicked()
{
    QString builderPath = Blockchain::getInstance()->getBuilderFilePath("FileAddSignature");
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_builder_file_add_signature-" + builderPath, "wallet_builder_file_add_signature", QStringList() << ui->pathLineEdit->text()
                                               << "true"
                                               ));

    qDebug() << toJsonFormat( "id_wallet_builder_file_add_signature-" + builderPath, "wallet_builder_file_add_signature", QStringList() << ui->pathLineEdit->text()
                              << "true"
                              );

}

void MultiSigDialog::on_cancelBtn_clicked()
{
    close();
}

void MultiSigDialog::init()
{
    on_pathLineEdit_textChanged("");
}

void MultiSigDialog::parseBuilder()
{
    QString path = ui->pathLineEdit->text();
    if(!path.endsWith(".builder"))      return;
    QFile file(ui->pathLineEdit->text());
    if( file.open(QIODevice::ReadOnly))
    {
        QByteArray ba = file.readAll();


        QJsonParseError json_error;
        QJsonDocument parse_doucment = QJsonDocument::fromJson(ba, &json_error);
        if(json_error.error == QJsonParseError::NoError)
        {
            if( parse_doucment.isObject())
            {
                QJsonObject jsonObject = parse_doucment.object();
                if(jsonObject.contains("optional_signatures"))
                {
                    QJsonArray array = jsonObject.take("optional_signatures").toArray();
                    QStringList allAddresses;
                    for(int i = 0; i < array.size(); i++)
                    {
                        allAddresses << array.at(i).toString();
                    }

                    QJsonArray array2 = jsonObject.take("signatures_unsigned").toArray();
                    QStringList unsignedAddresses;
                    for(int i = 0; i < array2.size(); i++)
                    {
                        unsignedAddresses << array2.at(i).toString();
                    }

                    QStringList signedAddresses = allAddresses;
                    foreach (QString address, signedAddresses)
                    {
                        if( unsignedAddresses.contains(address))
                        {
                            signedAddresses.removeAll(address);
                        }
                    }

                    int requireCount = jsonObject.take("optional_signatures_required").toInt();
                    ui->requireCountLabel->setText( QString::number(requireCount));

                    int signedCount = signedAddresses.size();
                    ui->signedCountLabel->setText(QString::number( signedCount));

                    int unsignedCount = unsignedAddresses.size();
                    ui->unsignedCountLabel->setText(QString::number( unsignedCount));

                    ui->signedTableWidget->setRowCount(signedCount);
                    for( int i = 0; i < signedCount; i++)
                    {
                        QString address = signedAddresses.at(i);
                        ui->signedTableWidget->setItem(i,0, new QTableWidgetItem(address));
                        if( Blockchain::getInstance()->isMyAddress( address) )
                        {
                            ui->signedTableWidget->setItem(i,1, new QTableWidgetItem( Blockchain::getInstance()->addressToName(address)));
                        }
                        else
                        {
//                            ui->signedTableWidget->setItem(i,1, new QTableWidgetItem( QString::fromLocal8Bit("非钱包内账户")));
                            ui->signedTableWidget->setItem(i,1, new QTableWidgetItem( tr("non-local")));
                        }
                    }

                    ui->unsignedTableWidget->setRowCount(unsignedCount);
                    int myAddressCount = 0;
                    for( int i = 0; i < unsignedCount; i++)
                    {
                        QString address = unsignedAddresses.at(i);
                        ui->unsignedTableWidget->setItem(i,0, new QTableWidgetItem(address));
                        if( Blockchain::getInstance()->isMyAddress( address) )
                        {
                            ui->unsignedTableWidget->setItem(i,1, new QTableWidgetItem( Blockchain::getInstance()->addressToName(address)));
                            myAddressCount++;
                        }
                        else
                        {
                            ui->unsignedTableWidget->setItem(i,1, new QTableWidgetItem( tr("non-local")));
                        }
                    }

                    if( signedCount >= requireCount)
                    {
//                        ui->okBtn->setEnabled(false);
//                        ui->tipLabel->setText(QString::fromLocal8Bit("签名本就已经满足，请勿重复广播交易"));
                        ui->tipLabel->setText(tr("Signatures are already enough.You should not rebroadcast this transaction."));
                        ui->tipLabel->setStyleSheet("color:red");
                    }
                    else if( signedCount + myAddressCount >= requireCount )
                    {
//                        ui->okBtn->setEnabled(true);
//                        ui->tipLabel->setText(QString::fromLocal8Bit("签名可满足，签名后将会直接广播交易"));
                        ui->tipLabel->setText(tr("Signatures are enough.This transaction will be broadcast after you sign it."));
                        ui->tipLabel->setStyleSheet("color:green");
                    }
                    else
                    {
//                        ui->okBtn->setEnabled(true);
//                        ui->tipLabel->setText(QString::fromLocal8Bit("签名还未满足，将生产新的builder文件"));
                        ui->tipLabel->setText(tr("Signatures are not enough.A new builder file will be generated."));
                        ui->tipLabel->setStyleSheet("color:green");
                    }
                }
            }

        }
    }
    else
    {
        qDebug() << "builder file open failed: " << path << file.errorString();
    }
}

void MultiSigDialog::on_chooseFileBtn_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select multi-signature file"), Blockchain::getInstance()->walletConfigPath + "/transaction_builder","(*.builder)");
    ui->pathLineEdit->setText(file);
}

void MultiSigDialog::on_pathLineEdit_textChanged(const QString &arg1)
{
    ui->requireCountLabel->clear();
    ui->signedCountLabel->clear();
    ui->unsignedCountLabel->clear();
    ui->signedTableWidget->setRowCount(0);
    ui->unsignedTableWidget->setRowCount(0);
//    ui->okBtn->setEnabled(false);

    parseBuilder();
}
