#include "importdialog.h"
#include "ui_importdialog.h"
#include "rpcthread.h"
#include "blockchain.h"
#include "commondialog.h"
#include "namedialog.h"
#include "control/shadowwidget.h"

#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QFile>

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

//    Fry::getInstance()->appendCurrentDialogVector(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");

    ui->containerWidget->setObjectName("containerWidget");
    ui->containerWidget->setStyleSheet("#containerWidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->importBtn->setEnabled(false);

    ui->privateKeyLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->privateKeyLineEdit->setTextMargins(8,0,0,0);
    ui->privateKeyLineEdit->setFocus();

    shadowWidget = new ShadowWidget(this);
    shadowWidget->init(this->size());
    shadowWidget->hide();
}

ImportDialog::~ImportDialog()
{
    delete ui;
//    Fry::getInstance()->removeCurrentDialogVector(this);
}

void ImportDialog::pop()
{
//    QEventLoop loop;
//    show();
//    ui->privateKeyLineEdit->grabKeyboard();
//    connect(this,SIGNAL(accepted()),&loop,SLOT(quit()));
//    loop.exec();  //进入事件 循环处理，阻塞

    move(0,0);
    exec();
}

void ImportDialog::on_cancelBtn_clicked()
{
    close();
//    qDebug() << "on_cancelBtn_clicked";
//    emit accepted();
}

void ImportDialog::on_pathBtn_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Choose your private key file."),"","(*.key)");
    file.replace("/","\\");
    ui->privateKeyLineEdit->setText(file);
}

void ImportDialog::on_importBtn_clicked()
{
    // 如果输入框中是 私钥文件的地址
    if( ui->privateKeyLineEdit->text().contains(".key") )
    {
        QFile file( ui->privateKeyLineEdit->text());
        if( file.open(QIODevice::ReadOnly))
        {
            QByteArray ba = QByteArray::fromBase64( file.readAll());
            QString str(ba);

            Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_import_private_key", "wallet_import_private_key", QStringList() << str));

            ui->importBtn->setEnabled(false);
            shadowWidget->show();

            file.close();
            return;
        }
        else
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Wrong file path."));
            commonDialog.pop();
            return;
        }

    }
    else // 如果输入框中是 私钥
    {
        Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_import_private_key", "wallet_import_private_key", QStringList() << ui->privateKeyLineEdit->text() ));

         ui->importBtn->setEnabled(false);
         shadowWidget->show();

    }

}

QString toThousandFigure( int);

void ImportDialog::jsonDataUpdated(QString id)
{
    if( id == "id_wallet_import_private_key")
    {
        shadowWidget->hide();

        QString result = Blockchain::getInstance()->jsonDataValue(id);
        if( result.mid(0,9) == "\"result\":")
        {
            ui->importBtn->setEnabled(true);

            Blockchain::getInstance()->needToScan = true;  // 导入成功，需要scan

            QString name = result.mid(10, result.indexOf("\"", 10) - 10);
            if( accountNameAlreadyExisted(name))        //  如果已存在该账号
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText( name + tr(" already existed") );
                commonDialog.pop();

            }
            else
            {
                mutexForAddressMap.lock();
                int size = Blockchain::getInstance()->addressMap.size();
                mutexForAddressMap.unlock();

                mutexForConfigFile.lock();
                Blockchain::getInstance()->configFile->setValue( QString("/accountInfo/") + QString::fromLocal8Bit("账户") + toThousandFigure(size+1),name);
                mutexForConfigFile.unlock();
                Blockchain::getInstance()->balanceMapInsert( name, "0.00000 " + QString(ASSET_NAME));
                Blockchain::getInstance()->registerMapInsert( name, "UNKNOWN");
                Blockchain::getInstance()->addressMapInsert( name, Blockchain::getInstance()->getAddress(name));

                emit accountImported();

                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText( name + tr( " has been imported!"));
                commonDialog.pop();

//                RpcThread* rpcThread = new RpcThread;
//                connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//                rpcThread->setLogin("a","b");
//                rpcThread->setWriteData( toJsonFormat( "id_scan", "scan", QStringList() << "0" ));
//                rpcThread->setWriteData( toJsonFormat( "id_wallet_scan_transaction", "wallet_scan_transaction", QStringList() << "ALL" << "true" ));
//                rpcThread->start();

                Blockchain::getInstance()->postRPC( toJsonFormat( "id_scan", "scan", QStringList() << "0" ));
                Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_scan_transaction", "wallet_scan_transaction", QStringList() << "ALL" << "true" ));


                close();
//                emit accepted();
            }

        }
        else if( result.mid(0,8) == "\"error\":")
        {
            qDebug() << "import error: " << result;

            if( !result.contains("Unknown key! You must specify an account name!") )
            {
                // 如果不是未注册账户 是错误的私钥格式
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText( tr("Illegal private key!"));
                commonDialog.pop();

                ui->importBtn->setEnabled(true);
                return;
            }

            NameDialog nameDialog;
            QString name = nameDialog.pop();

            if( name.isEmpty())    // 如果取消 命名
            {
                ui->importBtn->setEnabled(true);
                return;
            }

            if( ui->privateKeyLineEdit->text().contains(".key") )  // 如果是路径
            {
                QFile file( ui->privateKeyLineEdit->text());
                if( file.open(QIODevice::ReadOnly))
                {
                    QByteArray ba = QByteArray::fromBase64( file.readAll());
                    QString str(ba);

                    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_import_private_key_" + name, "wallet_import_private_key", QStringList() << str << name << "true"));
                    shadowWidget->show();

                    file.close();
                    return;
                }
                else
                {
                    CommonDialog commonDialog(CommonDialog::OkOnly);
                    commonDialog.setText( tr("Wrong file path."));
                    commonDialog.pop();

                    ui->importBtn->setEnabled(true);

                    return;
                }

            }
            else // 如果输入框中是 私钥
            {
                Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_import_private_key_" + name, "wallet_import_private_key", QStringList() << ui->privateKeyLineEdit->text() << name << "true" ));
                shadowWidget->show();
            }
        }

        return;
    }

    if( id.mid(0,29) == "id_wallet_import_private_key_")
    {
        ui->importBtn->setEnabled(true);
        shadowWidget->hide();

        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result.mid(0,9) == "\"result\":")
        {
            Blockchain::getInstance()->needToScan = true;  // 导入成功，需要scan

            QString name = result.mid(10, result.indexOf("\"", 10) - 10);

            mutexForAddressMap.lock();
            int size = Blockchain::getInstance()->addressMap.size();
            mutexForAddressMap.unlock();

            mutexForConfigFile.lock();
            Blockchain::getInstance()->configFile->setValue( QString("/accountInfo/") + QString::fromLocal8Bit("账户") + toThousandFigure(size+1),name);
            mutexForConfigFile.unlock();
            Blockchain::getInstance()->balanceMapInsert( name, "0.00000 " + QString(ASSET_NAME));
            Blockchain::getInstance()->registerMapInsert( name, "NO");
            Blockchain::getInstance()->addressMapInsert( name, Blockchain::getInstance()->getAddress(name));

            emit accountImported();

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( name + tr(" has been imported!") );
            commonDialog.pop();

            Blockchain::getInstance()->postRPC( toJsonFormat( "id_scan", "scan", QStringList() << "0" ));
            Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_scan_transaction", "wallet_scan_transaction", QStringList() << "ALL" << "true" ));


            close();
//            emit accepted();
        }
        else if( result.mid(0,8) == "\"error\":")
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Illegal private key!"));
            commonDialog.pop();
        }

        return;
    }

}

bool ImportDialog::accountNameAlreadyExisted(QString name)
{
    mutexForConfigFile.lock();
    Blockchain::getInstance()->configFile->beginGroup("/accountInfo");

    QStringList keys = Blockchain::getInstance()->configFile->childKeys();
    QString str;
    bool existed = false;
    foreach (str, keys)
    {
        if( Blockchain::getInstance()->configFile->value(str) == name)
        {
            existed = true;
            break;
        }
    }
    Blockchain::getInstance()->configFile->endGroup();
    mutexForConfigFile.unlock();
    return existed;

}

void ImportDialog::on_privateKeyLineEdit_textChanged(const QString &arg1)
{
    QString str = arg1.simplified();
    if( str.isEmpty())
    {
        ui->importBtn->setEnabled(false);
    }
    else
    {
        ui->importBtn->setEnabled(true);
    }
}

void ImportDialog::on_privateKeyLineEdit_returnPressed()
{
    if( ui->importBtn->isEnabled())
    {
        on_importBtn_clicked();
    }
}
