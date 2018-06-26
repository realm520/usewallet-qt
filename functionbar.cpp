#include "functionbar.h"
#include "ui_functionbar.h"
#include "blockchain.h"
#include "commondialog.h"
#include "debug_log.h"

#include <QPainter>
#include <QDebug>

#define FUNCTIONPAGE_BTN_SELECTED       "background-color: rgb(62,43,90);color: rgb(255,255,255);border:none;"
#define FUNCTIONPAGE_BTN_UNSELECTED     "background-color: rgb(109,91,255);color: rgb(255,255,255);border:none;"

FunctionBar::FunctionBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionBar)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    ui->setupUi(this);

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(FUNCTION_BAR_COLOR));
    setPalette(palette);

    ui->marketBtn->hide();
    ui->contactBtn->move(0, 173);
    choosePage(1);

    DLOG_QT_WALLET_FUNCTION_END;
}

FunctionBar::~FunctionBar()
{
    delete ui;
}

void FunctionBar::on_accountBtn_clicked()
{
    choosePage(1);
    emit showMainPage();
}

void FunctionBar::on_assetBtn_clicked()
{
    choosePage(2);
    emit showAssetPage();
}

void FunctionBar::on_transferBtn_clicked()
{
    mutexForAddressMap.lock();
    int size = Blockchain::getInstance()->addressMap.size();
    mutexForAddressMap.unlock();
    if( size > 0)   // 有至少一个账户
    {
        choosePage(3);
        emit showTransferPage();
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("No account for transfering,\nadd an account first"));
//        commonDialog.move( this->mapToGlobal(QPoint(318,150)));
        commonDialog.pop();
        on_accountBtn_clicked();
    }
}

void FunctionBar::on_marketBtn_clicked()
{
    choosePage(4);
    emit showMarketPage();
}

void FunctionBar::on_contactBtn_clicked()
{
    choosePage(5);
    emit showContactPage();
}

void FunctionBar::choosePage(int pageIndex)
{

    switch (pageIndex) {
    case 1:
        ui->accountBtn->setStyleSheet(FUNCTIONPAGE_BTN_SELECTED);
        ui->assetBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->transferBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->marketBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->contactBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        break;
    case 2:
        ui->accountBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->assetBtn->setStyleSheet(FUNCTIONPAGE_BTN_SELECTED);
        ui->transferBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->marketBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->contactBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        break;
    case 3:
        ui->accountBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->assetBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->transferBtn->setStyleSheet(FUNCTIONPAGE_BTN_SELECTED);
        ui->marketBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->contactBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        break;
    case 4:
        ui->accountBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->assetBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->transferBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->marketBtn->setStyleSheet(FUNCTIONPAGE_BTN_SELECTED);
        ui->contactBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        break;
    case 5:
        ui->accountBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->assetBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->transferBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->marketBtn->setStyleSheet(FUNCTIONPAGE_BTN_UNSELECTED);
        ui->contactBtn->setStyleSheet(FUNCTIONPAGE_BTN_SELECTED);
        break;
    default:
        break;
    }
}

void FunctionBar::retranslator()
{
    ui->retranslateUi(this);
}



