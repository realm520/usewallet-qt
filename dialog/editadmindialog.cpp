#include "editadmindialog.h"
#include "ui_editadmindialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"
#include "dialog/enteraddressdialog.h"

EditAdminDialog::EditAdminDialog(int assetId, QWidget *parent) :
    QDialog(parent),
    id(assetId),
    ui(new Ui::EditAdminDialog)
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

    ui->numLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->numLineEdit->setTextMargins(8,0,0,0);
    QRegExp rx1("^([1-9][0-9]{0,3})");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->numLineEdit->setValidator(pReg1);
    ui->numLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->addressTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->addressTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->addressTableWidget->setColumnWidth(0,330);
    ui->addressTableWidget->setColumnWidth(1,120);
    ui->addressTableWidget->setColumnWidth(2,80);
    ui->addressTableWidget->setShowGrid(true);
    ui->addressTableWidget->setFrameShape(QFrame::NoFrame);
    ui->addressTableWidget->setMouseTracking(true);

    init();
}

EditAdminDialog::~EditAdminDialog()
{
    delete ui;
}

void EditAdminDialog::pop()
{
    move(0,0);
    exec();
}

void EditAdminDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id_wallet_set_asset_authority-") )
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        QString buildPath = id.mid( QString("id_wallet_set_asset_authority-").size());

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

void EditAdminDialog::on_okBtn_clicked()
{
    if( tmpOwners.size() == 0)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
//        commonDialog.setText(QString::fromLocal8Bit("管理员账户不能为空"));
        commonDialog.setText(tr("At least one admin is needed."));
        commonDialog.pop();
        return;
    }

    int num = ui->numLineEdit->text().toInt();
    if( num < 1)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
//        commonDialog.setText(QString::fromLocal8Bit("确认数量至少为1"));
        commonDialog.setText(tr("Amount of confirmation should be at least 1."));
        commonDialog.pop();
    }
    else if( num > tmpOwners.size())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
//        commonDialog.setText(QString::fromLocal8Bit("确认数量不能超过地址总数"));
        commonDialog.setText(tr("It should not be larger than the number of the addresses."));
        commonDialog.pop();
    }
    else
    {
        // 将地址拼成["address1","address2"] 的格式
        QString str = "[";
        foreach (QString address, tmpOwners)
        {
            str += "\"" + address + "\",";

        }
        str += "]";
        str.replace(",]","]");

        QString builderPath = Blockchain::getInstance()->getBuilderFilePath("SetAssetAuthority_" + ui->symbolLabel->text());
//        Fry::getInstance()->postRPC( toJsonFormat( "id_wallet_set_asset_authority-" + builderPath, "wallet_set_asset_authority", QStringList() << ui->accountComboBox->currentText()
//                                                   << ui->symbolLabel->text() << str << QString::number(num)
//                                                   << builderPath ));
//        qDebug() << toJsonFormat( "id_wallet_set_asset_authority-" + builderPath, "wallet_set_asset_authority", QStringList() << ui->accountComboBox->currentText()
//                                  << ui->symbolLabel->text() << str << QString::number(num)
//                                  << builderPath );


        QString jsonStr = QString("{\"jsonrpc\":\"2.0\",\"id\":\"id_wallet_set_asset_authority-%1\",\"method\":\"wallet_set_asset_authority\","
                                  "\"params\":[\"%2\",\"%3\",%4,%5,\"%6\"]}").arg(builderPath).arg(ui->accountComboBox->currentText()).arg(ui->symbolLabel->text())
                                   .arg(str).arg(QString::number(num)).arg(builderPath);
        Blockchain::getInstance()->postRPC( jsonStr);
    }
}

void EditAdminDialog::on_cancelBtn_clicked()
{
    close();
}

void EditAdminDialog::on_addBtn_clicked()
{
    EnterAddressDialog enterAddressDialog;
    if(enterAddressDialog.pop())
    {
        QString address = enterAddressDialog.address;
        if( tmpOwners.contains(address))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
//            commonDialog.setText(QString::fromLocal8Bit("地址已存在!"));
            commonDialog.setText(tr("This address already exists!"));
            commonDialog.pop();
        }
        else
        {
            tmpOwners << address;
            showAdmins(tmpOwners);
        }
    }
}

void EditAdminDialog::on_undoBtn_clicked()
{
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    showAdmins(info.owners);
    tmpOwners = info.owners;
}

void EditAdminDialog::init()
{
    AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
    QStringList myAddresses = Blockchain::getInstance()->getMyAddresses(info.owners);
    ui->accountComboBox->clear();
    foreach (QString address, myAddresses)
    {
        ui->accountComboBox->addItem(Blockchain::getInstance()->addressToName(address));
    }

    ui->symbolLabel->setText(info.symbol);

    ui->numLineEdit->setText( QString::number(info.requiredNum) );

    showAdmins(info.owners);

    tmpOwners = info.owners;
}

void EditAdminDialog::showAdmins(QStringList owners)
{
    ui->addressTableWidget->setRowCount(0);
    ui->addressTableWidget->setRowCount(owners.size());
    QStringList myAddresses = Blockchain::getInstance()->getMyAddresses(owners);
    int row = 0;
    foreach (QString address, owners)
    {
        ui->addressTableWidget->setItem( row, 0, new QTableWidgetItem(address));

        if( myAddresses.contains(address))
        {
            ui->addressTableWidget->setItem( row, 1, new QTableWidgetItem( Blockchain::getInstance()->addressToName(address) ));
        }
        else
        {
            ui->addressTableWidget->setItem( row, 1, new QTableWidgetItem(tr("non-local")));
        }

        ui->addressTableWidget->setItem( row, 2, new QTableWidgetItem(tr("delete")));

        ui->addressTableWidget->item(row,0)->setTextAlignment(Qt::AlignCenter);
        ui->addressTableWidget->item(row,1)->setTextAlignment(Qt::AlignCenter);
        ui->addressTableWidget->item(row,2)->setTextAlignment(Qt::AlignCenter);
        ui->addressTableWidget->item(row,2)->setTextColor(Qt::red);

        row++;
    }

}

void EditAdminDialog::on_addressTableWidget_cellPressed(int row, int column)
{
    if( column == 2)
    {
        QString address = ui->addressTableWidget->item(row,0)->text();

        tmpOwners.removeAll(address);
        showAdmins(tmpOwners);
    }
}
