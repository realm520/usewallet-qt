#ifndef ASSETPAGE_H
#define ASSETPAGE_H

#include <QWidget>

namespace Ui {
class AssetPage;
}

class AssetPage : public QWidget
{
    Q_OBJECT

public:
    explicit AssetPage(QWidget *parent = 0);
    ~AssetPage();

    void updateAssetInfo();
    void updateMyAsset();

private slots:
    void jsonDataUpdated(QString id);

    void on_allAssetBtn_clicked();

    void on_myAssetBtn_clicked();

    void on_createAssetBtn_clicked();

    void on_myAssetWidget_cellPressed(int row, int column);

    void on_multiSigBtn_clicked();

    void on_assetInfoTableWidget_cellPressed(int row, int column);

private:
    Ui::AssetPage *ui;

    void paintEvent(QPaintEvent*);

};

#endif // ASSETPAGE_H
