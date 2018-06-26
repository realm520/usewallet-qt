#include "contactpage.h"
#include "ui_contactpage.h"
#include "blockchain.h"
#include "debug_log.h"
#include "gop_common_define.h"
#include "editremarkdialog.h"
#include "addcontactdialog.h"
#include "rpcthread.h"

#include <QDebug>
#include <QPainter>

ContactPage::ContactPage(QWidget *parent) :
    QWidget(parent),
    oldRemark(""),
    ui(new Ui::ContactPage)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    ui->setupUi(this);

    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(244,244,242));
    setPalette(palette);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));


    ui->addContactBtn->setStyleSheet("QToolButton{background-image:url(:/pic/cplpic/addContactBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");


    updateContactsList();


    //    ui->scrollArea->setStyleSheet("QScrollArea{border:none;}"
    //                                  "QScrollBar:vertical"
    //                                  "{"
    //                                  "width:8px;"
    //                                  "background:rgb(255,255,255);"
    //                                  "margin:0px,0px,0px,0px;"
    //                                  "padding-top:2px;"
    //                                  "padding-bottom:3px;"
    //                                  "}");


    //    ui->scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
    //    ui->scrollAreaWidgetContents->setStyleSheet("#scrollAreaWidgetContents{background-color:rgb(244,244,242);}");

    DLOG_QT_WALLET_FUNCTION_END;
}

ContactPage::~ContactPage()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    delete ui;
    DLOG_QT_WALLET_FUNCTION_END;
}

void ContactPage::updateContactsList()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    if( !Blockchain::getInstance()->contactsFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "contact.dat not exist";
    }
    QByteArray rd = Blockchain::getInstance()->contactsFile->readAll();
    QString str =  QByteArray::fromBase64( rd);

    QStringList strList = str.split(";");
    strList.removeLast();
    int size = strList.size();

    if( size > 4)
    {
        ui->scrollAreaWidgetContents->setMinimumHeight(418 + (size - 4) * 71);
//        ui->scrollAreaWidgetContents->setGeometry(0,0,826, 397 + (size - 4) * 71);
    }
    else
    {
        ui->scrollAreaWidgetContents->setMinimumHeight(418);
    }

    if( size == 0)
    {
        ui->initLabel->show();
    }
    else
    {
        ui->initLabel->hide();
    }

    releaseVector();
    contactVector.clear();
    for( int i = 0; i < size; i++)
    {
        QString str2 = strList.at(i);
        SingleContactWidget* widget = new SingleContactWidget(i,str2.left( str2.indexOf("=")),str2.mid( str2.indexOf("=") + 1),ui->scrollAreaWidgetContents);
        connect(widget,SIGNAL(deleteContact()),this,SLOT(updateContactsList()));
        connect(widget,SIGNAL(showShadowWidget()),this,SLOT(shadowWidgetShow()));
        connect(widget,SIGNAL(hideShadowWidget()),this,SLOT(shadowWidgetHide()));
        connect(widget,SIGNAL(editContact(QString,QString)),this,SLOT(editContact(QString,QString)));
        connect(widget,SIGNAL(gotoTransferPage(QString,QString)),this,SIGNAL(gotoTransferPage(QString,QString)));
        contactVector.insert(i,widget);
        contactVector.at(i)->show();
        contactVector.at(i)->move(20, 32 + 71 * i);
    }


    Blockchain::getInstance()->contactsFile->close();

    DLOG_QT_WALLET_FUNCTION_END;
}

void ContactPage::on_addContactBtn_clicked()
{

    AddContactDialog addContactDialog;
    addContactDialog.pop();

    updateContactsList();

}

void ContactPage::releaseVector()
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    foreach (SingleContactWidget* widget, contactVector) {
        widget->close();
        delete widget;
    }
    DLOG_QT_WALLET_FUNCTION_END;
}

void ContactPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(228,228,228),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(247,246,242),Qt::SolidPattern));
    painter.drawRect(-1,-1,858,68);
}


void ContactPage::shadowWidgetShow()
{
    emit showShadowWidget();
}

void ContactPage::shadowWidgetHide()
{
    emit hideShadowWidget();
}



void ContactPage::editContact(QString address, QString remark)
{
    DLOG_QT_WALLET_FUNCTION_BEGIN;
    oldRemark = remark;

    EditRemarkDialog editRemarkDialog( remark);
//    emit showShadowWidget();
    QString remarkName = editRemarkDialog.pop();
//    emit hideShadowWidget();

    if( oldRemark == remarkName)
    {
        return;
    }

    if( !Blockchain::getInstance()->contactsFile->open(QIODevice::ReadWrite))
    {
        qDebug() << "contact.dat not exist";
        return;
    }

    QByteArray ba = QByteArray::fromBase64( Blockchain::getInstance()->contactsFile->readAll());
    QString str(ba);
    QStringList strList = str.split(";");
    strList.removeLast();

    for(QStringList::iterator& ss = strList.begin();  ss != strList.end(); ss++)
    {
        if(  (*ss).mid(0, (*ss).indexOf('=')) == address && (*ss).mid( (*ss).indexOf('=') + 1) ==  remarkName )
        {
            Blockchain::getInstance()->contactsFile->close();
            return;
        }
        if( ((*ss).mid(0, (*ss).indexOf('=')) == address) && (*ss).mid( (*ss).indexOf('=') + 1) ==  oldRemark )
        {
            (*ss) = address + '=' + remarkName;
            ba = "";
            foreach (QString ss2, strList)
            {
                ba += ss2 + ';';
            }
            ba = ba.toBase64();
            Blockchain::getInstance()->contactsFile->resize(0);
            QTextStream ts(Blockchain::getInstance()->contactsFile);
            ts << ba;
            Blockchain::getInstance()->contactsFile->close();

            updateContactsList();
            return;
        }
    }

    ba += address.toUtf8() + '=' + remarkName.toUtf8() + QString(";").toUtf8();
    ba = ba.toBase64();
    Blockchain::getInstance()->contactsFile->resize(0);
    QTextStream ts(Blockchain::getInstance()->contactsFile);
    ts << ba;

    Blockchain::getInstance()->contactsFile->close();

    DLOG_QT_WALLET_FUNCTION_END;
}


void ContactPage::retranslator(QString language)
{
    ui->retranslateUi(this);
}

void ContactPage::jsonDataUpdated(QString id)
{
    if( id == "id_blockchain_get_account")
    {
        QString  result = Blockchain::getInstance()->jsonDataValue(id);
        return;
    }

}
