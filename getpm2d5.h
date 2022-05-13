#ifndef GETPM2D5_H
#define GETPM2D5_H


#include <QMainWindow>
#include<QFileDialog>
#include<QMessageBox>
#include<QGraphicsView>
#include<gdal_priv.h>
#include<QStandardPaths>
#include<QProcess>
#include "singleprocess.h"
#include "ui_singleprocess.h"
#include"batchprocess.h"
#include "ui_batchprocess.h"
QT_BEGIN_NAMESPACE
namespace Ui { class getPM2D5; }
QT_END_NAMESPACE

class getPM2D5 : public QMainWindow {
    Q_OBJECT

public:
    getPM2D5(QWidget *parent = nullptr);


    ~getPM2D5();


private:
    SingleProcess *singlePwidget;
    BatchProcess  *batchPwidget;

private:
    //    QGraphicsScene *myScene;

    Ui::getPM2D5 *ui;
    QString oImageName;
    QStringList batchFileNames;
    GDALDataset *poDataSet;
    GDALRasterBand *currentBand;

    //    GDALDataset *WSDataset;
    //    GDALDataset *RHDataset;

    int outWidth;
    int outHeight;

    QMap<int,QProcess*> m_processMap;

    double minmax[2];
private:
    //!初始化界面
    void init();
    //!在侧边栏显示打开图形的属性。
    //!适用情况，
    //! 1单处理模式显示打开图像的信息
    //! 2单处理模式显示结果图像的信息
    //! 3批处理模式显示均值影像的信息
    void showImageAttribute(GDALDataset *ImageDataset);
    //!绘制直方图
    //!适用情况，
    //! 1单处理模式显示打开的图像的当前波段的直方图，
    //! 2单处理模式显示结果图像的直方图
    //! 3批处理模式显示均值影像的直方图
    void showHistogram(double dfMin,double dfMax,int nBuckets,GUIntBig *histogramData,int currentID);

    void showImage(GDALRasterBand *displayBand);

    uint8_t *imageStretch(float *buffer, GDALRasterBand *band, int bandShape, double noDataValue);

    //    uint8_t *imageClassRGB(float *buffer, int bandShape, double noDataValue);

    std::tuple<QString,QString,QString,GDALRasterBand*> getSingleArgument();

    QStringList getWSRHFile(QString year,QString date);

    QVector<int> outputArguments(QStringList WSRHFile,GDALRasterBand* outputBand,QStringList Year_dayOfYear,const QString& outputFile);

    void showResult(const QString &resultFile);
    //!调用R语言计算PM2.5数值
    void doPerdict(int index,QString date,QString argumentFile,QString outputPredictFilePath,bool isBatch);

    void writeOutTIF(QString predictFile,QString filePath,int outWidth,int outHeight,double geo[6],const char*projection);
private slots:
    void chooseMultiFile();
    void showSinglePwidget();
    void changShowBand(int currentIndex);
    void loadOriginalImage();
    void showGradient(int currentIndex);
    //!单处理
    void doSingleProcess();
    //!批处理
    void doBatchProcess();
    //!关于按钮
    void on_about();
    //!清理工作空间
    //! TODO:并不能恢复内存占用，待解决。20200906记
    void clearWorkSpace();

};

#endif // GETPM2D5_H
