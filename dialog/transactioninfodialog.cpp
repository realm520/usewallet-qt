#include "transactioninfodialog.h"
#include "ui_transactioninfodialog.h"

#include <QDateTime>

TransactionInfoDialog::TransactionInfoDialog(TransactionInfo info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionInfoDialog)
{
    ui->setupUi(this);

    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");


    ui->entriesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->entriesTableWidget->setColumnWidth(0,200);
    ui->entriesTableWidget->setColumnWidth(1,160);
    ui->entriesTableWidget->setColumnWidth(2,120);
    ui->entriesTableWidget->setColumnWidth(3,120);
    ui->entriesTableWidget->setShowGrid(true);
    ui->entriesTableWidget->setFrameShape(QFrame::NoFrame);
    ui->entriesTableWidget->setMouseTracking(true);


    ui->trxIdLabel->setText(info.trxId);
    ui->feeLabel->setText(AmountToQString(info.fee , 100000) + " " + ASSET_NAME);
    ui->blockHeightLabel->setText(QString::number(info.blockNum));

    if(info.isMarket)
    {
        ui->isMarketLabel->setText(tr("Yes"));
    }
    else
    {
        ui->isMarketLabel->setText(tr("No"));
    }

    if(info.isMarketCancel)
    {
        ui->isMarketCancelLabel->setText(tr("Yes"));
    }
    else
    {
        ui->isMarketCancelLabel->setText(tr("No"));
    }

    QString date = info.timeStamp;
    date.replace(QString("T"),QString(" "));
    QDateTime time = QDateTime::fromString(date, "yyyy-MM-dd hh:mm:ss");
    time = time.addSecs(28800);       // 时间加8小时
    ui->trxTimeLabel->setText(time.toString("yyyy-MM-dd hh:mm:ss"));

    date = info.expirationTimeStamp;
    if(date == "1970-01-01T00:00:00")
    {
        ui->trxExpirationLabel->setText("--");
    }
    else
    {
        date.replace(QString("T"),QString(" "));
        time = QDateTime::fromString(date, "yyyy-MM-dd hh:mm:ss");
        time = time.addSecs(28800);       // 时间加8小时
        ui->trxExpirationLabel->setText(time.toString("yyyy-MM-dd hh:mm:ss"));
    }


    foreach (Entry entry, info.entries)
    {

    }

    int size = info.entries.size();
    ui->entriesTableWidget->setRowCount(size);
    for( int i = 0; i < size; i++)
    {
        Entry entry = info.entries.at(i);

        ui->entriesTableWidget->setItem(i,0, new QTableWidgetItem(entry.fromAccount));
        ui->entriesTableWidget->setItem(i,1, new QTableWidgetItem(entry.toAccount));

        AssetInfo assetInfo = Blockchain::getInstance()->assetInfoMap.value(entry.amount.assetId);
        QString amountStr =  AmountToQString(entry.amount.amount , assetInfo.precision) + " " + assetInfo.symbol;
        ui->entriesTableWidget->setItem(i,2, new QTableWidgetItem(amountStr));

        ui->entriesTableWidget->setItem(i,3, new QTableWidgetItem(entry.memo));

        for(int j = 0; j < 4; j++)
        {
            ui->entriesTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
        }

    }
}

TransactionInfoDialog::~TransactionInfoDialog()
{
    delete ui;
}

void TransactionInfoDialog::pop()
{
    move(0,0);
    exec();
}

void TransactionInfoDialog::on_okBtn_clicked()
{
    close();
}
