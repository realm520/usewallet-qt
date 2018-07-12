#include <QDebug>
#include <QPainter>
#include <QHelpEvent>
#include <QDateTime>
#include <QTextCodec>
#include <QScrollBar>

#include "accountpage.h"
#include "ui_accountpage.h"
#include "debug_log.h"
#include <QClipboard>
#include "commondialog.h"
#include "showcontentdialog.h"
#include "rpcthread.h"
#include "control/remarkcellwidget.h"
#include "control/qrcodedialog.h"
#include "dialog/transactioninfodialog.h"

AccountPage::AccountPage(QString name, QWidget *parent) :
    QWidget(parent),
    accountName(name),
    transactionType(0),
    address(""),
    inited(false),
    assetUpdating(false),
    ui(new Ui::AccountPage)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    ui->setupUi(this);

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(255,255,255));
    setPalette(palette);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    mutexForAddressMap.lock();
    if( accountName.isEmpty())  // 如果是点击账单跳转
    {
        if( Blockchain::getInstance()->addressMap.size() > 0)
        {
            accountName = Blockchain::getInstance()->addressMap.keys().at(0);
        }
        else  // 如果还没有账户
        {
            mutexForAddressMap.unlock();
            emit back();
            return;
        }
    }

    // 账户下拉框按字母顺序排序
    QStringList keys = Blockchain::getInstance()->addressMap.keys();
    ui->accountComboBox->addItems( keys);
    if( accountName.isEmpty() )
    {
        ui->accountComboBox->setCurrentIndex(0);
    }
    else
    {
        ui->accountComboBox->setCurrentText( accountName);
    }
    mutexForAddressMap.unlock();

    ui->pageLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->pageLineEdit->setText("1");
    QIntValidator *validator = new QIntValidator(1,9999,this);
    ui->pageLineEdit->setValidator( validator );
    ui->pageLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    currentPageIndex = 1;
    ui->pageLineEdit->setText( QString::number(currentPageIndex));

    ui->prePageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");
    ui->nextPageBtn->setStyleSheet("QToolButton:!hover{border:0px;color:#999999;} QToolButton:hover{border:0px;color:#469cfc;}");


    ui->accountComboBox->setStyleSheet("QComboBox{border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                  "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                  "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                  );

    ui->assetComboBox->setStyleSheet("QComboBox{border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 9em;}"
                  "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                  "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                  );


    ui->copyBtn->setStyleSheet("QToolButton{background-image:url(:/pic/cplpic/copy.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->copyBtn->setToolTip(tr("copy to clipboard"));

    ui->qrcodeBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/qrcode.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

    ui->accountTransactionsTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->accountTransactionsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->accountTransactionsTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->accountTransactionsTableWidget->setColumnWidth(0,100);
    ui->accountTransactionsTableWidget->setColumnWidth(1,300);
    ui->accountTransactionsTableWidget->setColumnWidth(2,130);
    ui->accountTransactionsTableWidget->setColumnWidth(3,130);
    ui->accountTransactionsTableWidget->setColumnWidth(4,100);
    ui->accountTransactionsTableWidget->setShowGrid(true);
    ui->accountTransactionsTableWidget->setFrameShape(QFrame::NoFrame);
    ui->accountTransactionsTableWidget->setMouseTracking(true);
    ui->accountTransactionsTableWidget->horizontalHeader()->setVisible(true);


    ui->initLabel->hide();



    getAssets();

    ui->assetComboBox->setCurrentText(Blockchain::getInstance()->currentAsset);
    init();
    updateTransactionsList();

    inited = true;



    DLOG_QT_WALLET_FUNCTION_END;
}

QString discard(const QString &str)
{
    int dotPos = str.indexOf(".");
    if( dotPos != -1)
    {
        return str.left( dotPos + 3);
    }
    else
    {
		DLOG_QT_WALLET(" no dot!");
        return NULL;
    }
}

AccountPage::~AccountPage()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;
    DLOG_QT_WALLET_FUNCTION_END;

}

void AccountPage::init()
{

    QString showName = Blockchain::getInstance()->addressMapValue(accountName).ownerAddress;
    address = showName;
    showName = showName.left(9) + "..." + showName.right(3);
    ui->addressLabel->setText( showName);
    ui->addressLabel->adjustSize();
    ui->addressLabel->setGeometry(ui->addressLabel->x(),ui->addressLabel->y(),ui->addressLabel->width(),24);
    ui->copyBtn->move(ui->addressLabel->x() + ui->addressLabel->width() + 9, 39);
    ui->qrcodeBtn->move(ui->addressLabel->x() + ui->addressLabel->width() + 30, 39);

    ui->accountComboBox->setCurrentText(accountName);

    jsonDataUpdated("id_balance");


}

void AccountPage::updateTransactionsList()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    Blockchain::getInstance()->postRPC( toJsonFormat( "id_history_" + accountName + "_" + ui->assetComboBox->currentText(), "history", QStringList() << accountName << getAssetName( ui->assetComboBox->currentText() ) << "9999999" ));

    DLOG_QT_WALLET_FUNCTION_END;
}

void AccountPage::getAssets()
{
    assetUpdating = true;

    int index = ui->assetComboBox->currentIndex();
    ui->assetComboBox->clear();
    foreach (int key, Blockchain::getInstance()->assetInfoMap.keys())
    {
        ui->assetComboBox->addItem( Blockchain::getInstance()->assetInfoMap.value(key).symbol);
    }

    if( index < 0 )   index = 0;
    ui->assetComboBox->setCurrentIndex(index);

    assetUpdating = false;
}


void AccountPage::refresh()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;

    init();
    updateTransactionsList();

    DLOG_QT_WALLET_FUNCTION_END;
}

void AccountPage::on_copyBtn_clicked()
{
    QClipboard* clipBoard = QApplication::clipboard();
    clipBoard->setText(address);

    CommonDialog commonDialog(CommonDialog::OkOnly);
    commonDialog.setText(tr("Copy to clipboard"));
    commonDialog.pop();
}


void AccountPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(228,228,228),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(247,246,242),Qt::SolidPattern));
    painter.drawRect(-1,-1,858,68);
}

void AccountPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    if( inited)  // 防止accountComboBox当前值改变的时候触发
    {
        accountName = arg1;
        Blockchain::getInstance()->currentAccount = accountName;
        currentPageIndex = 1;
        ui->pageLineEdit->setText( QString::number(currentPageIndex));
        init();
        updateTransactionsList();
    }
}




void AccountPage::jsonDataUpdated(QString id)
{
    if( id == "id_history_" + accountName + "_" + ui->assetComboBox->currentText())
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

        Blockchain::getInstance()->parseTransactions(result,accountName + "_" + ui->assetComboBox->currentText());

        showTransactions();
        return;
    }



    if( id == "id_balance")
    {
        Blockchain::getInstance()->parseBalance();

        if( !Blockchain::getInstance()->accountBalanceMap.contains(accountName))
        {
            ui->balanceLabel->setText( "<body><font style=\"font-size:18px\" color=#000000>0.00</font><font style=\"font-size:12px\" color=#000000> " + ui->assetComboBox->currentText() + "</font></body>" );
            return;
        }

        AssetBalanceMap assetBalanceMap = Blockchain::getInstance()->accountBalanceMap.value(accountName);
        int assetId = Blockchain::getInstance()->getAssetId(ui->assetComboBox->currentText());
        if( assetId < 0)    return;
        double amount = assetBalanceMap.value(assetId);
        AssetInfo info = Blockchain::getInstance()->assetInfoMap.value(assetId);
        ui->balanceLabel->setText( "<body><font style=\"font-size:18px\" color=#000000>" + QString::number(amount / info.precision,'g',15) + "</font><font style=\"font-size:12px\" color=#000000> " + ui->assetComboBox->currentText() + "</font></body>" );
        ui->balanceLabel->adjustSize();

        return;
    }

    if( id.startsWith("id_blockchain_get_transaction+"))
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result.startsWith("\"result\":"))
        {
            Blockchain::getInstance()->parseTransactionOperations(result);
        }
        else if( result.startsWith("\"error\":"))
        {
            QString trxId = id.mid(QString("id_blockchain_get_transaction+").size());
            TransactionOperation transaction;
            transaction.trxId = trxId;
            transaction.type = -1;
            Blockchain::getInstance()->transactionOperationsMap.insert(trxId,transaction);
        }

        return;
    }
}



void AccountPage::showTransactions()
{
    TransactionsInfoVector vector = Blockchain::getInstance()->transactionsMap.value(accountName + "_" + ui->assetComboBox->currentText());

    ui->accountTransactionsTableWidget->setRowCount(0);
    if( vector.size() < 1)
    {
        ui->prePageBtn->hide();
        ui->numberLabel->hide();
        ui->pageLineEdit->hide();
        ui->pageLabel->hide();
        ui->nextPageBtn->hide();
        ui->initLabel->show();
        return;
    }
    ui->prePageBtn->show();
    ui->numberLabel->show();
    ui->pageLineEdit->show();
    ui->pageLabel->show();
    ui->nextPageBtn->show();
    ui->initLabel->hide();

    int size = vector.size();
    ui->numberLabel->setText( tr("total ") + QString::number( size) + tr(" ,"));
    ui->pageLabel->setText( "/" + QString::number( (size - 1)/10 + 1 ) );

    int rowCount = size - (currentPageIndex - 1) * 10;
    if( rowCount > 10 )  rowCount = 10;  // 一页最多显示10行
    ui->accountTransactionsTableWidget->setRowCount(rowCount);

    for(int i = rowCount - 1; i > -1; i--)
    {
        ui->accountTransactionsTableWidget->setRowHeight(i,57);
        TransactionInfo transactionInfo = vector.at(  size - ( i + 1) - (currentPageIndex - 1) * 10 );


        // 区块高度
        ui->accountTransactionsTableWidget->setItem(i,0,new QTableWidgetItem( QString::number(transactionInfo.blockNum) ));

        // 交易ID
        ui->accountTransactionsTableWidget->setItem(i,1,new QTableWidgetItem( transactionInfo.trxId));

        if( !Blockchain::getInstance()->transactionOperationsMap.contains(transactionInfo.trxId) && transactionInfo.blockNum > 0)
        {
            getTransaction(transactionInfo.trxId);
        }

        TransactionDetail detail = getDetail(transactionInfo, accountName, ui->assetComboBox->currentText());
        // 交易类型
        ui->accountTransactionsTableWidget->setItem(i,2,new QTableWidgetItem( detail.type));

        int amountSize = detail.amountVector.size();
        QString amountStr;
        for( int i = 0; i < amountSize; i++)
        {
            amountStr += detail.amountVector.at(i) + "\n";
        }
        if( amountStr.size() > 0)    amountStr.chop(1);
        ui->accountTransactionsTableWidget->setItem(i,3,new QTableWidgetItem(amountStr));

        // 查看详情
        ui->accountTransactionsTableWidget->setItem(i,4,new QTableWidgetItem( tr("more")));
        ui->accountTransactionsTableWidget->item(i,4)->setTextColor(QColor(0,255,0));


        for(int j = 0; j < 5; j++)
        {
            ui->accountTransactionsTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

void AccountPage::getTransaction(QString trxId)
{
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_blockchain_get_transaction+" + trxId, "blockchain_get_transaction",
                                               QStringList() << trxId
                                               ));
}



//void AccountPage::on_accountTransactionsTableWidget_cellClicked(int row, int column)
//{
//    if( column == 1 )
//    {
//        ShowContentDialog showContentDialog( ui->accountTransactionsTableWidget->item(row, column)->text(),this);
//        int scrollBarValue = ui->accountTransactionsTableWidget->verticalScrollBar()->sliderPosition();
//        showContentDialog.move( ui->accountTransactionsTableWidget->mapToGlobal( QPoint(120, 57 * (row - scrollBarValue) + 42)));
//        showContentDialog.exec();

//        return;
//    }

//    if( column == 5)
//    {

//        QString remark = ui->accountTransactionsTableWidget->item(row, column)->text();
//        remark.remove(' ');
//        if( remark.isEmpty() )  return;
//        ShowContentDialog showContentDialog( ui->accountTransactionsTableWidget->item(row, column)->text(),this);
//        int scrollBarValue = ui->accountTransactionsTableWidget->verticalScrollBar()->sliderPosition();
//        showContentDialog.move( ui->accountTransactionsTableWidget->mapToGlobal( QPoint(640, 57 * (row - scrollBarValue) + 42)));
//        showContentDialog.exec();
//        return;
//    }
//}


void AccountPage::on_prePageBtn_clicked()
{
    ui->accountTransactionsTableWidget->scrollToTop();
    if( currentPageIndex == 1) return;
    currentPageIndex--;
    showTransactions();
    ui->pageLineEdit->setText( QString::number(currentPageIndex));

}

void AccountPage::on_nextPageBtn_clicked()
{
//    if( currentPageIndex >=  ((searchList.size() - 1)/10 + 1))  return;
    int totalPageNum = ui->pageLabel->text().remove("/").toInt();
    if(  currentPageIndex >= totalPageNum )  return;

    currentPageIndex++;
    showTransactions();
    ui->pageLineEdit->setText( QString::number(currentPageIndex));

    ui->accountTransactionsTableWidget->scrollToTop();
}

void AccountPage::on_pageLineEdit_editingFinished()
{
    currentPageIndex = ui->pageLineEdit->text().toInt();
    showTransactions();
}

void AccountPage::on_pageLineEdit_textEdited(const QString &arg1)
{
    if( arg1.at(0) == '0')
    {
        ui->pageLineEdit->setText( arg1.mid(1));
        return;
    }
    int totalPageNum = ui->pageLabel->text().remove("/").toInt();

    if( arg1.toInt() > totalPageNum)
    {
        ui->pageLineEdit->setText( QString::number( totalPageNum));
        return;
    }
}

void AccountPage::on_assetComboBox_currentIndexChanged(int index)
{
    if( assetUpdating)  return;

    currentPageIndex = 1;
    ui->pageLineEdit->setText( QString::number(currentPageIndex));
    init();
    updateTransactionsList();
    Blockchain::getInstance()->currentAsset = ui->assetComboBox->currentText();
}
void AccountPage::on_qrcodeBtn_clicked()
{
    QRCodeDialog qrcodeDialog(Blockchain::getInstance()->addressMapValue(accountName).ownerAddress);
    qrcodeDialog.move(ui->qrcodeBtn->mapToGlobal( QPoint(20,0)));
    qrcodeDialog.exec();
}

void AccountPage::on_accountTransactionsTableWidget_cellPressed(int row, int column)
{
    if( column == 4 )
    {
        TransactionsInfoVector vector = Blockchain::getInstance()->transactionsMap.value(accountName + "_" + ui->assetComboBox->currentText());
        int size = vector.size();
        TransactionInfo transactionInfo = vector.at(  size - ( row + 1) - (currentPageIndex - 1) * 10 );


        TransactionInfoDialog transactionInfoDialog(transactionInfo);
        transactionInfoDialog.pop();

        return;
    }

    if( column == 1)
    {
        ShowContentDialog showContentDialog( ui->accountTransactionsTableWidget->item(row, column)->text(),this);

        int x = ui->accountTransactionsTableWidget->columnViewportPosition(column) + ui->accountTransactionsTableWidget->columnWidth(column) / 2
                - showContentDialog.width() / 2;
        int y = ui->accountTransactionsTableWidget->rowViewportPosition(row) - 10 + ui->accountTransactionsTableWidget->horizontalHeader()->height();

        showContentDialog.move( ui->accountTransactionsTableWidget->mapToGlobal( QPoint(x, y)));
        showContentDialog.exec();

        return;
    }
}
