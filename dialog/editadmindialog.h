#ifndef EDITADMINDIALOG_H
#define EDITADMINDIALOG_H

#include <QDialog>

namespace Ui {
class EditAdminDialog;
}

class EditAdminDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditAdminDialog( int assetId, QWidget *parent = 0);
    ~EditAdminDialog();

    int id;

    void  pop();

private slots:
    void jsonDataUpdated( QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_addBtn_clicked();

    void on_undoBtn_clicked();

    void on_addressTableWidget_cellPressed(int row, int column);

private:
    Ui::EditAdminDialog *ui;
    QStringList tmpOwners;

    void init();
    void showAdmins(QStringList owners);
};

#endif // EDITADMINDIALOG_H
