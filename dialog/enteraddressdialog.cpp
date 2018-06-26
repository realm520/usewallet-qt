#include "enteraddressdialog.h"
#include "ui_enteraddressdialog.h"

#include <QDebug>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

EnterAddressDialog::EnterAddressDialog(QWidget *parent) :
    QDialog(parent),
    yesOrNO(false),
    ui(new Ui::EnterAddressDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    QRegExp regx("[a-zA-Z0-9\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->addressLineEdit->setValidator( validator );

    ui->addressLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->addressLineEdit->setTextMargins(8,0,0,0);
    ui->addressLineEdit->setPlaceholderText( tr("Please enter an account address."));
    ui->addressLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->okBtn->setEnabled(false);
    ui->addressLineEdit->setFocus();
}

EnterAddressDialog::~EnterAddressDialog()
{
    delete ui;
}

bool EnterAddressDialog::pop()
{
    move(0,0);
    exec();

    return yesOrNO;
}

void EnterAddressDialog::on_okBtn_clicked()
{
    address = ui->addressLineEdit->text();
    yesOrNO = true;
    close();
}

void EnterAddressDialog::on_cancelBtn_clicked()
{
    yesOrNO = false;
    close();
}

void EnterAddressDialog::on_addressLineEdit_returnPressed()
{
    on_okBtn_clicked();
}

void EnterAddressDialog::on_addressLineEdit_textChanged(const QString &arg1)
{
    if( checkAddress(ui->addressLineEdit->text()) )
    {
        ui->okBtn->setEnabled(true);
    }
    else
    {
        ui->okBtn->setEnabled(false);
    }
}
