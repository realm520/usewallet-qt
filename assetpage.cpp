#include "assetpage.h"
#include "ui_assetpage.h"

#include <QDebug>
#include <QPainter>

#include "blockchain.h"
#include "dialog/assetissuedialog.h"
#include "dialog/createassetdialog.h"
#include "commondialog.h"
#include "dialog/choosetokenoperationdialog.h"
#include "dialog/issuetoassetdialog.h"
#include "dialog/transfertoassetdialog.h"
#include "dialog/haltmarketdialog.h"
#include "dialog/multisigdialog.h"
#include "dialog/editadmindialog.h"
#include "showcontentdialog.h"

#define ASSETPAGE_ALLASSETBTN_SELECTED_STYLE     "QToolButton{background-color:rgb(0,138,254);color:white;border:1px solid rgb(0,138,254);border-top-left-radius: 3px;border-bottom-left-radius: 3px;}"
#define ASSETPAGE_ALLASSETBTN_UNSELECTED_STYLE   "QToolButton{background-color:white;color:rgb(0,138,254);border:1px solid rgb(0,138,254);border-top-left-radius: 3px;border-bottom-left-radius: 3px;}"
#define ASSETPAGE_MYASSETBTN_SELECTED_STYLE        "QToolButton{background-color:rgb(0,138,254);color:white;border:1px solid rgb(0,138,254);border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
#define ASSETPAGE_MYASSETBTN_UNSELECTED_STYLE      "QToolButton{background-color:white;color:rgb(0,138,254);border:1px solid rgb(0,138,254);border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"


AssetPage::AssetPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetPage)
{
    ui->setupUi(this);

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(244,244,242));
    setPalette(palette);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    updateAssetInfo();
    updateMyAsset();
    on_allAssetBtn_clicked();

    ui->assetInfoTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->assetInfoTableWidget->setColumnWidth(0,100);
    ui->assetInfoTableWidget->setColumnWidth(1,100);
    ui->assetInfoTableWidget->setColumnWidth(2,120);
    ui->assetInfoTableWidget->setColumnWidth(3,120);
    ui->assetInfoTableWidget->setColumnWidth(4,220);
    ui->assetInfoTableWidget->setShowGrid(true);
    ui->assetInfoTableWidget->setFrameShape(QFrame::NoFrame);
    ui->assetInfoTableWidget->setMouseTracking(true);

    ui->myAssetWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->myAssetWidget->setColumnWidth(0,100);
    ui->myAssetWidget->setColumnWidth(1,100);
    ui->myAssetWidget->setColumnWidth(2,120);
    ui->myAssetWidget->setColumnWidth(3,120);
    ui->myAssetWidget->setColumnWidth(4,220);
    ui->myAssetWidget->setColumnWidth(5,100);
    ui->myAssetWidget->setShowGrid(true);
    ui->myAssetWidget->setFrameShape(QFrame::NoFrame);
    ui->myAssetWidget->setMouseTracking(true);

    ui->assetInfoTableWidget->horizontalHeader()->setVisible(true);
    ui->myAssetWidget->horizontalHeader()->setVisible(true);
    ui->multiSigBtn->hide();
}

AssetPage::~AssetPage()
{
    delete ui;
}

void AssetPage::updateAssetInfo()
{
    ui->assetInfoTableWidget->setRowCount(0);
    ui->assetInfoTableWidget->setRowCount(Blockchain::getInstance()->assetInfoMap.size());
    foreach (int id, Blockchain::getInstance()->assetInfoMap.keys())
    {
        ui->assetInfoTableWidget->setRowHeight(id,45);

        AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);

        ui->assetInfoTableWidget->setItem(id,0,new QTableWidgetItem(QString::number(id)));
        ui->assetInfoTableWidget->setItem(id,1,new QTableWidgetItem(info.symbol));

        if( id == 0)
        {
            ui->assetInfoTableWidget->setItem(id,3,new QTableWidgetItem("-"));
        }
        else
        {
            ui->assetInfoTableWidget->setItem(id,3,new QTableWidgetItem(QString::number(info.currentSupply * 100 / info.maxSupply,'g',5) + "%"));
        }

        ui->assetInfoTableWidget->setItem(id,2,new QTableWidgetItem(QString::number(info.maxSupply / info.precision,'g',15)));
        ui->assetInfoTableWidget->setItem(id,4,new QTableWidgetItem(info.description));

        for(int j = 0; j < 5; j++)
        {
            ui->assetInfoTableWidget->item(id,j)->setTextAlignment(Qt::AlignCenter);
        }

    }
}

void AssetPage::updateMyAsset()
{
    ui->myAssetWidget->setRowCount(0);
    ui->myAssetWidget->setRowCount(Blockchain::getInstance()->assetInfoMap.size());

    int myAssetCount = 0;
    foreach (int id, Blockchain::getInstance()->assetInfoMap.keys())
    {
        ui->myAssetWidget->setRowHeight(myAssetCount,45);

        AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);

//        if( !Fry::getInstance()->isMyAddress(info.owner))   continue;
        QStringList myAddresses = Blockchain::getInstance()->getMyAddresses(info.owners);
        if(myAddresses.isEmpty())   continue;

        ui->myAssetWidget->setItem(myAssetCount,0,new QTableWidgetItem(QString::number(id)));
        ui->myAssetWidget->setItem(myAssetCount,1,new QTableWidgetItem(info.symbol));

        if( id == 0)
        {
            ui->myAssetWidget->setItem(id,3,new QTableWidgetItem("-"));
        }
        else
        {
            ui->myAssetWidget->setItem(myAssetCount,3,new QTableWidgetItem(QString::number(info.currentSupply * 100 / info.maxSupply,'g',5) + "%"));
        }

        ui->myAssetWidget->setItem(myAssetCount,2,new QTableWidgetItem(QString::number(info.maxSupply / info.precision,'g',15)));
        ui->myAssetWidget->setItem(myAssetCount,4,new QTableWidgetItem(info.description));
        ui->myAssetWidget->setItem(myAssetCount,5,new QTableWidgetItem(tr("manage")));
        ui->myAssetWidget->item(myAssetCount,5)->setTextColor(QColor(65,205,82));

        for(int j = 0; j < 6; j++)
        {
            ui->myAssetWidget->item(myAssetCount,j)->setTextAlignment(Qt::AlignCenter);
        }

        myAssetCount++;
    }

    ui->myAssetWidget->setRowCount(myAssetCount);
}

void AssetPage::jsonDataUpdated(QString id)
{

}

void AssetPage::on_allAssetBtn_clicked()
{
    ui->assetInfoTableWidget->show();
    ui->myAssetWidget->hide();

    ui->allAssetBtn->setStyleSheet(ASSETPAGE_ALLASSETBTN_SELECTED_STYLE);
    ui->myAssetBtn->setStyleSheet(ASSETPAGE_MYASSETBTN_UNSELECTED_STYLE);
}

void AssetPage::on_myAssetBtn_clicked()
{
    ui->assetInfoTableWidget->hide();
    ui->myAssetWidget->show();

    ui->allAssetBtn->setStyleSheet(ASSETPAGE_ALLASSETBTN_UNSELECTED_STYLE);
    ui->myAssetBtn->setStyleSheet(ASSETPAGE_MYASSETBTN_SELECTED_STYLE);
}

void AssetPage::on_createAssetBtn_clicked()
{
    mutexForAddressMap.lock();
    int size = Blockchain::getInstance()->addressMap.size();
    mutexForAddressMap.unlock();
    if( size == 0)   // 有至少一个账户
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
//        commonDialog.setText(QString::fromLocal8Bit("您还没有创建账户!"));
        commonDialog.setText(tr("You have no account!"));
        commonDialog.pop();

        return;
    }

    if( Blockchain::getInstance()->getMyDelegateAccounts().isEmpty())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Only delegate account can create assets!"));
        commonDialog.pop();

        return;
    }

    CreateAssetDialog createAssetDialog;
    createAssetDialog.pop();
}

void AssetPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(228,228,228),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(247,246,242),Qt::SolidPattern));
    painter.drawRect(-1,-1,858,66);
}

void AssetPage::on_myAssetWidget_cellPressed(int row, int column)
{
    if( column == 5)
    {
//        int id = ui->myAssetWidget->item(row,0)->text().toInt();
//        AssetIssueDialog assetIssueDialog(id);
//        assetIssueDialog.pop();

        int id = ui->myAssetWidget->item(row,0)->text().toInt();
        ChooseTokenOperationDialog chooseTokenOperationDialog(id);
        int choice = chooseTokenOperationDialog.pop();

        switch (choice)
        {
        case 1:
        {
            AssetIssueDialog assetIssueDialog(id);
            assetIssueDialog.pop();
            break;
        }
        case 2:
        {
            IssueToAssetDialog issueToAssetDialog(id);
            issueToAssetDialog.pop();
            break;
        }
        case 3:
        {
            TransferToAssetDialog transferToAssetDialog(id);
            transferToAssetDialog.pop();
            break;
        }
        case 4:
        {
            AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(id);
            QString str;
            if( info.activeFlags & 4)
            {
                HaltMarketDialog haltMarketDialog(id,false);
                haltMarketDialog.pop();
            }
            else
            {
                HaltMarketDialog haltMarketDialog(id,true);
                haltMarketDialog.pop();
            }

            break;
        }
        case 5:
        {
            EditAdminDialog editAdminDialog(id);
            editAdminDialog.pop();
            break;
        }
        default:
            break;
        }
    }

}

void AssetPage::on_multiSigBtn_clicked()
{
    MultiSigDialog multiSigDialog;
    multiSigDialog.pop();
}

void AssetPage::on_assetInfoTableWidget_cellPressed(int row, int column)
{

}
