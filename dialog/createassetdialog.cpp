#include "createassetdialog.h"
#include "ui_createassetdialog.h"

#include <QDebug>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

CreateAssetDialog::CreateAssetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAssetDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    ui->accountComboBox->setStyleSheet("QComboBox {border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                                           "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                                                                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                                                                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                                           "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                                           );

    ui->symbolLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->symbolLineEdit->setTextMargins(8,0,0,0);
    QRegExp regx("^[A-Z]{3,12}$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->symbolLineEdit->setValidator( validator );
    ui->symbolLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->assetNameLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->assetNameLineEdit->setTextMargins(8,0,0,0);
//    QRegExp regx2("[a-zA-Z]+$");
//    QValidator *validator2 = new QRegExpValidator(regx2, this);
//    ui->assetNameLineEdit->setValidator( validator2 );
    ui->assetNameLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->descriptionLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->descriptionLineEdit->setTextMargins(8,0,0,0);

    ui->maxSupplyLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->maxSupplyLineEdit->setTextMargins(8,0,0,0);
    QRegExp regx3("[0-9\.]+$");
    QValidator *validator3 = new QRegExpValidator(regx3, this);
    ui->maxSupplyLineEdit->setValidator( validator3 );
    ui->maxSupplyLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));


    QStringList delegates = Blockchain::getInstance()->getMyDelegateAccounts();
    ui->accountComboBox->addItems( delegates);



}

CreateAssetDialog::~CreateAssetDialog()
{
    delete ui;
}

void CreateAssetDialog::pop()
{
    move(0,0);
    exec();
}

void CreateAssetDialog::on_okBtn_clicked()
{
    if( ui->symbolLineEdit->text().simplified().isEmpty() || ui->symbolLineEdit->text().size()<3) {
        return;
    }
    if( ui->assetNameLineEdit->text().simplified().isEmpty())     return;
    if( ui->descriptionLineEdit->text().simplified().isEmpty())     return;
    if( ui->maxSupplyLineEdit->text().simplified().isEmpty())     return;

    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_uia_create", "wallet_uia_create", QStringList() << ui->accountComboBox->currentText()
                                               << ui->symbolLineEdit->text() << ui->assetNameLineEdit->text() << ui->descriptionLineEdit->text()
                                               << ui->maxSupplyLineEdit->text()));

}

void CreateAssetDialog::on_cancelBtn_clicked()
{
    close();
}


void CreateAssetDialog::jsonDataUpdated(QString id)
{
    if( id == "id_wallet_uia_create")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result.startsWith("\"result\":"))
        {
            Blockchain::getInstance()->scanLater();

            close();

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Succeed to create!"));
            commonDialog.pop();

        }
        else if( result.startsWith("\"error\":"))
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            if( errorMessage == "unknown account id")
            {
                CommonDialog commonDialog(CommonDialog::YesOrNo);
                commonDialog.setText(tr("This account is unregistered.Will you register it?\n(You will pay the fee)"));
                if( commonDialog.pop())
                {
                    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_account_register", "wallet_account_register",
                                                               QStringList() << ui->accountComboBox->currentText() << ui->accountComboBox->currentText() ));
                }

                return;

            }


            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Fail to create: ") + errorMessage);
            commonDialog.pop();
        }


        return;
    }

    if( id == "id_wallet_account_register")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        if( result.startsWith("\"result\":"))
        {
            Blockchain::getInstance()->scanLater();

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Succeed to register."));
            commonDialog.pop();
        }
        else if( result.startsWith("\"error\":"))
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Fail to register: ") + errorMessage);
            commonDialog.pop();
        }

        return;
    }
}

void CreateAssetDialog::on_symbolLineEdit_textChanged(const QString &arg1)
{
    if (ui->symbolLineEdit->text().size()<3) {
        ui->symbolLineEdit->setStyleSheet("color:black;border:1px solid red;border-radius:3px;");
    } else {
        ui->symbolLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    }

}
