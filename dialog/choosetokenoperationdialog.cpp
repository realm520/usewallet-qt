#include "choosetokenoperationdialog.h"
#include "ui_choosetokenoperationdialog.h"

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"

#include <QDebug>

ChooseTokenOperationDialog::ChooseTokenOperationDialog(int assetId, QWidget *parent) :
    QDialog(parent),
    choice(0),
    id(assetId),
    ui(new Ui::ChooseTokenOperationDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    if( info.activeFlags & 4 )
    {
        ui->haltBtn->setText(tr("Recover market"));
    }
    else
    {
        ui->haltBtn->setText(tr("Halt market"));
    }
}

ChooseTokenOperationDialog::~ChooseTokenOperationDialog()
{
    delete ui;
}

int ChooseTokenOperationDialog::pop()
{
    move(0,0);
    exec();
    return choice;
}

void ChooseTokenOperationDialog::jsonDataUpdated(QString id)
{

}

void ChooseTokenOperationDialog::on_cancelBtn_clicked()
{
    choice = 0;
    close();
}

void ChooseTokenOperationDialog::on_issueBtn_clicked()
{
    choice = 1;
    close();
}

void ChooseTokenOperationDialog::on_issueToAssetBtn_clicked()
{
    choice = 2;
    close();
}

void ChooseTokenOperationDialog::on_dividendBtn_clicked()
{
    choice = 3;
    close();
}

void ChooseTokenOperationDialog::on_haltBtn_clicked()
{
    choice = 4;
    close();
}

void ChooseTokenOperationDialog::on_editAdminBtn_clicked()
{
    choice = 5;
    close();
}
