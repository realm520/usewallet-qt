﻿#include <QPainter>
#include <QMouseEvent>
#include <QUrl>
#include <QDesktopServices>

#include "setdialog.h"
#include "ui_setdialog.h"
#include "blockchain.h"
#include "debug_log.h"
#include "rpcthread.h"
#include "commondialog.h"


#define SETDIALOG_GENERALBTN_SELECTED_STYLE     "QToolButton{background-color:rgb(0,138,254);color:white;border:1px solid rgb(0,138,254);border-top-left-radius: 3px;border-bottom-left-radius: 3px;}"
#define SETDIALOG_GENERALBTN_UNSELECTED_STYLE   "QToolButton{background-color:white;color:rgb(0,138,254);border:1px solid rgb(0,138,254);border-top-left-radius: 3px;border-bottom-left-radius: 3px;}"
#define SETDIALOG_SAVEBTN_SELECTED_STYLE        "QToolButton{background-color:rgb(0,138,254);color:white;border:1px solid rgb(0,138,254);border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
#define SETDIALOG_SAVEBTN_UNSELECTED_STYLE      "QToolButton{background-color:white;color:rgb(0,138,254);border:1px solid rgb(0,138,254);border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"


SetDialog::SetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetDialog)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);

//    Fry::getInstance()->appendCurrentDialogVector(this);
    setParent(Blockchain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    ui->containerWidget->installEventFilter(this);

    connect( Blockchain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    on_generalBtn_clicked();

    ui->lockTimeSpinBox->setValue(QString::number(Blockchain::getInstance()->lockMinutes).toInt());

    if(!Blockchain::getInstance()->notProduce)
    {
        ui->nolockCheckBox->setChecked(true);
        ui->lockTimeSpinBox->setEnabled(false);
    }
    else
    {
        ui->nolockCheckBox->setChecked(false);
        ui->lockTimeSpinBox->setEnabled(true);
    }


    QString language = Blockchain::getInstance()->language;
    if( language == "Simplified Chinese")
    {
        ui->languageComboBox->setCurrentIndex(0);
    }
    else if( language == "English")
    {
        ui->languageComboBox->setCurrentIndex(1);
    }
    else
    {
        ui->languageComboBox->setCurrentIndex(0);
    }

    ui->minimizeCheckBox->setChecked( Blockchain::getInstance()->minimizeToTray);
    ui->closeCheckBox->setChecked( Blockchain::getInstance()->closeToMinimize);
    ui->resyncCheckBox->setChecked( Blockchain::getInstance()->resyncNextTime);
    ui->feeLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->feeLineEdit->setTextMargins(8,0,0,0);
    ui->feeLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->feeLineEdit->setText( getBigNumberString(Blockchain::getInstance()->transactionFee,100000));

    QRegExp rx("^([0])(?:\\.\\d{0,3})?$|(^\\t?$)");
    QRegExpValidator *pReg = new QRegExpValidator(rx, this);
    ui->feeLineEdit->setValidator(pReg);
    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/close4.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

#ifdef WIN32
    ui->languageComboBox->setStyleSheet("QComboBox {border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;min-width: 5em;}"
                                    "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                                                           "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                                                           "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                                    "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                                    );
#endif

    ui->confirmBtn->setEnabled(false);

    ui->oldPwdLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->oldPwdLineEdit->setTextMargins(8,0,0,0);
    ui->oldPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->newPwdLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->newPwdLineEdit->setTextMargins(8,0,0,0);
    ui->newPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->confirmPwdLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->confirmPwdLineEdit->setTextMargins(8,0,0,0);
    ui->confirmPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    ui->oldPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->newPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->confirmPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->lockTimeSpinBox->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->minimizeCheckBox->setStyleSheet("QCheckBox::indicator{ image:url(:/pic/pic2/checkBox_unchecked.png); }"
                                    "QCheckBox::indicator:checked{ image:url(:/pic/cplpic/checkBox_checked.png); }");
    ui->closeCheckBox->setStyleSheet("QCheckBox::indicator{ image:url(:/pic/pic2/checkBox_unchecked.png); }"
                                    "QCheckBox::indicator:checked{ image:url(:/pic/cplpic/checkBox_checked.png); }");
    ui->nolockCheckBox->setStyleSheet("QCheckBox::indicator{ image:url(:/pic/pic2/checkBox_unchecked.png); }"
                                    "QCheckBox::indicator:checked{ image:url(:/pic/cplpic/checkBox_checked.png); }");
    ui->resyncCheckBox->setStyleSheet("QCheckBox::indicator{ image:url(:/pic/pic2/checkBox_unchecked.png); }"
                                    "QCheckBox::indicator:checked{ image:url(:/pic/cplpic/checkBox_checked.png); }");


    DLOG_QT_WALLET_FUNCTION_END;
}

SetDialog::~SetDialog()
{
    qDebug() << "setdialog delete";
//    Fry::getInstance()->currentDialog = NULL;
    delete ui;
//    Fry::getInstance()->removeCurrentDialogVector(this);
}

void SetDialog::pop()
{
//    QEventLoop loop;
//    show();
//    connect(this,SIGNAL(accepted()),&loop,SLOT(quit()));
//    loop.exec();  //进入事件 循环处理，阻塞

    move(0,0);
    exec();
}


bool SetDialog::eventFilter(QObject *watched, QEvent *e)
{
    if( watched == ui->containerWidget)
    {
        if( e->type() == QEvent::Paint)
        {
            QPainter painter(ui->containerWidget);
            painter.setPen(QPen(QColor(35,27,55),Qt::SolidLine));
            painter.setBrush(QBrush(QColor(35,27,55),Qt::SolidPattern));
            painter.drawRect(0,0,628,50);

            return true;
        }
    }

    return QWidget::eventFilter(watched,e);
}

void SetDialog::on_closeBtn_clicked()
{
    close();
//    emit accepted();
}

void SetDialog::on_saveBtn_clicked()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    if( ui->feeLineEdit->text().toDouble() == 0)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText( tr("The fee should not be 0!"));
        commonDialog.pop();
        return;
    }
    mutexForConfigFile.lock();
    Blockchain::getInstance()->lockMinutes = ui->lockTimeSpinBox->value();
    Blockchain::getInstance()->configFile->setValue("/settings/lockMinutes", Blockchain::getInstance()->lockMinutes);
    if( ui->nolockCheckBox->isChecked())
    {
        Blockchain::getInstance()->notProduce = false;
    }
    else
    {
        Blockchain::getInstance()->notProduce = true;
    }
    Blockchain::getInstance()->configFile->setValue("/settings/notAutoLock", !Blockchain::getInstance()->notProduce);

    if( ui->languageComboBox->currentIndex() == 0)      // config 中保存语言设置
    {
        Blockchain::getInstance()->configFile->setValue("/settings/language", "Simplified Chinese");
        Blockchain::getInstance()->language = "Simplified Chinese";
    }
    else if( ui->languageComboBox->currentIndex() == 1)
    {
        Blockchain::getInstance()->configFile->setValue("/settings/language", "English");
        Blockchain::getInstance()->language = "English";
    }


    Blockchain::getInstance()->minimizeToTray = ui->minimizeCheckBox->isChecked();
    Blockchain::getInstance()->configFile->setValue("/settings/minimizeToTray", Blockchain::getInstance()->minimizeToTray);

    Blockchain::getInstance()->closeToMinimize = ui->closeCheckBox->isChecked();
    Blockchain::getInstance()->configFile->setValue("/settings/closeToMinimize", Blockchain::getInstance()->closeToMinimize);

    Blockchain::getInstance()->resyncNextTime = ui->resyncCheckBox->isChecked();
    Blockchain::getInstance()->configFile->setValue("/settings/resyncNextTime", Blockchain::getInstance()->resyncNextTime);


    mutexForConfigFile.unlock();
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_set_transaction_fee", "wallet_set_transaction_fee", QStringList() << ui->feeLineEdit->text() ));
    emit settingSaved();

//    close();
//    emit accepted();

	DLOG_QT_WALLET_FUNCTION_END;
}


//void SetDialog::mousePressEvent(QMouseEvent *event)
//{
//    if(event->button() == Qt::LeftButton)
//     {
//          mouse_press = true;
//          //鼠标相对于窗体的位置（或者使用event->globalPos() - this->pos()）
//          move_point = event->pos();;
//     }
//}

//void SetDialog::mouseMoveEvent(QMouseEvent *event)
//{
//    //若鼠标左键被按下
//    if(mouse_press)
//    {
//        //鼠标相对于屏幕的位置
//        QPoint move_pos = event->globalPos();

//        //移动主窗体位置
//        this->move(move_pos - move_point);
//    }
//}

//void SetDialog::mouseReleaseEvent(QMouseEvent *)
//{
//    mouse_press = false;
//}

void SetDialog::on_nolockCheckBox_clicked()
{
    if(ui->nolockCheckBox->isChecked())
    {
        ui->lockTimeSpinBox->setEnabled(false);
    }
    else
    {
        ui->lockTimeSpinBox->setEnabled(true);
    }
}

void SetDialog::on_lockTimeSpinBox_valueChanged(const QString &arg1)
{
    if( arg1.startsWith('0') || arg1.isEmpty())
    {
        ui->lockTimeSpinBox->setValue(1);
    }
}

void SetDialog::on_generalBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->generalBtn->setStyleSheet(SETDIALOG_GENERALBTN_SELECTED_STYLE);
    ui->safeBtn->setStyleSheet(SETDIALOG_SAVEBTN_UNSELECTED_STYLE);
}

void SetDialog::on_safeBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->generalBtn->setStyleSheet(SETDIALOG_GENERALBTN_UNSELECTED_STYLE);
    ui->safeBtn->setStyleSheet(SETDIALOG_SAVEBTN_SELECTED_STYLE);
}

void SetDialog::on_confirmBtn_clicked()
{
    ui->tipLabel3->clear();

//    RpcThread* rpcThread = new RpcThread;
//    connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//    rpcThread->setLogin("a","b");
//    rpcThread->setWriteData( toJsonFormat( "id_wallet_change_passphrase", "wallet_change_passphrase", QStringList() << ui->oldPwdLineEdit->text() <<  ui->newPwdLineEdit->text() ));
//    rpcThread->start();
//    Fry::getInstance()->postRPC( toJsonFormat( "id_wallet_change_passphrase", "wallet_change_passphrase", QStringList() << ui->oldPwdLineEdit->text() <<  ui->newPwdLineEdit->text() ));
    Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_check_passphrase", "wallet_check_passphrase", QStringList() << ui->oldPwdLineEdit->text() ));

}

void SetDialog::on_newPwdLineEdit_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1)
    {
        ui->newPwdLineEdit->setText( ui->newPwdLineEdit->text().remove(' '));
        return;
    }

    if( arg1.isEmpty())
    {
        ui->tipLabel->setText("");
        ui->confirmBtn->setEnabled(false);

        return;
    }

    if( ui->newPwdLineEdit->text().length() < 8)
    {
//        ui->tipLabel->setText(QString::fromLocal8Bit("密码最少为8位"));
        ui->tipLabel->setText(tr("at least 8 letters"));
        ui->confirmBtn->setEnabled(false);

        return;
    }
    else
    {
        ui->tipLabel->clear();
    }

    if( ui->confirmPwdLineEdit->text().isEmpty())
    {
        ui->tipLabel2->clear();
        ui->confirmBtn->setEnabled(false);

        return;
    }

    if( ui->newPwdLineEdit->text() == ui->confirmPwdLineEdit->text())
    {
        ui->tipLabel2->clear();
        ui->confirmBtn->setEnabled(true);

    }
    else
    {
//        ui->tipLabel2->setText(QString::fromLocal8Bit("两次密码不一致"));
        ui->tipLabel2->setText(tr("not consistent"));
        ui->confirmBtn->setEnabled(false);

    }
}

void SetDialog::on_confirmPwdLineEdit_textChanged(const QString &arg1)
{
    if( arg1.indexOf(' ') > -1)
    {
        ui->confirmPwdLineEdit->setText( ui->confirmPwdLineEdit->text().remove(' '));
        return;
    }

    if( arg1.isEmpty())
    {
        ui->tipLabel2->setText("");
        ui->confirmBtn->setEnabled(false);

        return;
    }

    if( ui->confirmPwdLineEdit->text().length() < 8)
    {
//        ui->tipLabel2->setText(QString::fromLocal8Bit("两次密码不一致"));
        ui->tipLabel2->setText(tr("not consistent"));
        ui->tipLabel2->setText(tr("at least 8 letters"));
        ui->confirmBtn->setEnabled(false);

        return;
    }
    else
    {
        ui->tipLabel2->clear();
    }


    if( ui->newPwdLineEdit->text() == ui->confirmPwdLineEdit->text())
    {
        ui->confirmBtn->setEnabled(true);

    }
    else
    {
//        ui->tipLabel2->setText(QString::fromLocal8Bit("两次密码不一致"));
        ui->tipLabel2->setText(tr("not consistent"));
        ui->confirmBtn->setEnabled(false);

    }
}


void SetDialog::on_oldPwdLineEdit_textChanged(const QString &arg1)
{
    ui->tipLabel3->clear();

    if( arg1.indexOf(' ') > -1)
    {
        ui->oldPwdLineEdit->setText( ui->oldPwdLineEdit->text().remove( ' '));
    }
}

void SetDialog::jsonDataUpdated(QString id)
{
    if( id == "id_wallet_check_passphrase")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result == "\"result\":true")
        {
            Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_change_passphrase", "wallet_change_passphrase", QStringList() << ui->newPwdLineEdit->text() <<  ui->newPwdLineEdit->text() ));
        }
        else
        {
            ui->tipLabel3->setText( tr("wrong password"));
        }

        return;
    }

    if( id == "id_wallet_change_passphrase")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);

        if( result == "\"result\":null")
        {
            close();
        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( "Failed: " , errorMessage);
            commonDialog.pop();
        }

        return;
    }
    if( id == "id_wallet_set_transaction_fee")
    {
        QString result = Blockchain::getInstance()->jsonDataValue(id);
        if( result.startsWith("\"result\":"))
        {
            Blockchain::getInstance()->postRPC( toJsonFormat( "id_wallet_get_transaction_fee", "wallet_get_transaction_fee", QStringList() << "" ));
            close();
        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);
            if( errorMessage == "invalid fee")
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText( tr("The fee should not be 0!"));
                commonDialog.pop();
            }
            else
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText( "Failed: " , errorMessage);
                commonDialog.pop();
            }
        }
        return;
    }
}


//void SetDialog::setVisible(bool visiable)
//{
//    // 这样调用的是qwidget的setvisible 而不是qdialog的setvisible
//    // 即使调用hide 也不会结束exec(delete)
////    qDebug() << "setvisible " << visiable;
//    QWidget::setVisible(visiable);
//}
