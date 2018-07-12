#-------------------------------------------------
#
# Project created by QtCreator 2015-07-02T14:17:06
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = USEWallet
TEMPLATE = app

QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01

LIBS += -lDbgHelp
LIBS += User32.Lib
LIBS += -L$$PWD -lqrencode
LIBS += -limm32

SOURCES += main.cpp\
        firstlogin.cpp \
    normallogin.cpp \
    frame.cpp \
    mainpage.cpp \
    accountpage.cpp \
    transferpage.cpp \
    bottombar.cpp \
    setdialog.cpp \
    lockpage.cpp \
    consoledialog.cpp \
    titlebar.cpp \
    debug_log.cpp \
    contactdialog.cpp \
    waitingforsync.cpp \
    remarkdialog.cpp \
    functionbar.cpp \
    contactpage.cpp \
    singlecontactwidget.cpp \
    commondialog.cpp \
    showcontentdialog.cpp \
    namedialog.cpp \
    deleteaccountdialog.cpp \
    transferconfirmdialog.cpp \
    commontip.cpp \
    searchoptionwidget.cpp \
    rpcthread.cpp \
    addnodedialog.cpp \
    editremarkdialog.cpp \
    addcontactdialog.cpp \
    exportdialog.cpp \
    importdialog.cpp \
    ipcellwidget.cpp \
    incomecellwidget.cpp \
    control/myprogressbar.cpp \
    AES/aes.cpp \
    workerthread.cpp \
    workerthreadmanager.cpp \
    control/showbottombarwidget.cpp \
    control/rightclickmenudialog.cpp \
    control/chooseaddaccountdialog.cpp \
    dialog/renamedialog.cpp \
    control/remarkcellwidget.cpp \
    control/shadowwidget.cpp \
    assetpage.cpp \
    dialog/assetissuedialog.cpp \
    dialog/createassetdialog.cpp \
    marketpage.cpp \
    dialog/choosetokenoperationdialog.cpp \
    dialog/issuetoassetdialog.cpp \
    dialog/transfertoassetdialog.cpp \
    dialog/haltmarketdialog.cpp \
    dialog/multisigdialog.cpp \
    dialog/editadmindialog.cpp \
    dialog/enteraddressdialog.cpp \
    control/qrcodedialog.cpp \
    control/qrcodewidget.cpp \
    dialog/transactioninfodialog.cpp \
    selectwalletpathwidget.cpp \
    blockchain.cpp \
    ErrorTranslator.cpp

HEADERS  += firstlogin.h \
    normallogin.h \
    frame.h \
    mainpage.h \
    accountpage.h \
    transferpage.h \
    bottombar.h \
    setdialog.h \
    lockpage.h \
    consoledialog.h \
    gop_common_define.h \
    debug_log.h \
    titlebar.h \
    contactdialog.h \
    waitingforsync.h \
    remarkdialog.h \
    functionbar.h \
    contactpage.h \
    singlecontactwidget.h \
    commondialog.h \
    showcontentdialog.h \
    namedialog.h \
    deleteaccountdialog.h \
    transferconfirmdialog.h \
    commontip.h \
    searchoptionwidget.h \
    rpcthread.h \
    addnodedialog.h \
    editremarkdialog.h \
    addcontactdialog.h \
    exportdialog.h \
    importdialog.h \
    ipcellwidget.h \
    incomecellwidget.h \
    control/myprogressbar.h \
    AES/aes.h \
    workerthread.h \
    workerthreadmanager.h \
    control/showbottombarwidget.h \
    control/rightclickmenudialog.h \
    control/chooseaddaccountdialog.h \
    dialog/renamedialog.h \
    control/remarkcellwidget.h \
    control/shadowwidget.h \
    assetpage.h \
    dialog/assetissuedialog.h \
    dialog/createassetdialog.h \
    marketpage.h \
    dialog/choosetokenoperationdialog.h \
    dialog/issuetoassetdialog.h \
    dialog/transfertoassetdialog.h \
    dialog/haltmarketdialog.h \
    dialog/multisigdialog.h \
    dialog/editadmindialog.h \
    dialog/enteraddressdialog.h \
    control/qrcodedialog.h \
    control/qrcodewidget.h \
    dialog/transactioninfodialog.h \
    selectwalletpathwidget.h \
    blockchain.h \
    ErrorTranslator.h

FORMS    += firstlogin.ui \
    normallogin.ui \
    mainpage.ui \
    accountpage.ui \
    transferpage.ui \
    bottombar.ui \
    setdialog.ui \
    lockpage.ui \
    consoledialog.ui \
    titlebar.ui \
    contactdialog.ui \
    waitingforsync.ui \
    remarkdialog.ui \
    functionbar.ui \
    contactpage.ui \
    singlecontactwidget.ui \
    commondialog.ui \
    showcontentdialog.ui \
    namedialog.ui \
    deleteaccountdialog.ui \
    transferconfirmdialog.ui \
    commontip.ui \
    searchoptionwidget.ui \
    addnodedialog.ui \
    editremarkdialog.ui \
    addcontactdialog.ui \
    exportdialog.ui \
    importdialog.ui \
    ipcellwidget.ui \
    incomecellwidget.ui \
    control/accountdetailwidget.ui \
    control/rightclickmenudialog.ui \
    control/chooseaddaccountdialog.ui \
    dialog/renamedialog.ui \
    control/remarkcellwidget.ui \
    control/shadowwidget.ui \
    assetpage.ui \
    dialog/assetissuedialog.ui \
    dialog/createassetdialog.ui \
    marketpage.ui \
    dialog/choosetokenoperationdialog.ui \
    dialog/issuetoassetdialog.ui \
    dialog/transfertoassetdialog.ui \
    dialog/haltmarketdialog.ui \
    dialog/multisigdialog.ui \
    dialog/editadmindialog.ui \
    dialog/enteraddressdialog.ui \
    control/qrcodedialog.ui \
    dialog/transactioninfodialog.ui \
    selectwalletpathwidget.ui

DISTFILES += \
    logo.rc

RC_FILE = logo.rc

RESOURCES += \
    fry.qrc

TRANSLATIONS +=   wallet_simplified_Chinese.ts  wallet_English.ts
