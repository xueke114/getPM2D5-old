#include "getpm2d5.h"
#include "./ui_getpm2d5.h"

#include <QFile>
#include <QTextStream>
#include <QGraphicsPixmapItem>
#include <QtWidgets/QDialog>
#include <qwt_plot_histogram.h>
#include <QGraphicsScene>
#include <QProcess>

getPM2D5::getPM2D5(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::getPM2D5) {
    ui->setupUi(this);

    init();

}

void getPM2D5::doSingleProcess(){
    //!切换到输出窗口
    singlePwidget->ui->tabWidget->setCurrentIndex(1);
    //!禁止取消，禁止关闭
    singlePwidget->ui->pushButtonClose->setEnabled(false);

    //!获取指定的参数
    std::tuple<QString,QString,QString,GDALRasterBand*> selectedArgument=getSingleArgument();
    //!切换到输出文本浏览器
    //    singlePDialog->ui->tabWidget->setCurrentIndex(0);
    //!解析返回的参数
    QString year=std::get<0>(selectedArgument);
    QString date=std::get<1>(selectedArgument);
    QString dayofYear=std::get<2>(selectedArgument);
    GDALRasterBand* selectedBand=std::get<3>(selectedArgument);

    if (year.isEmpty()) {
        QMessageBox::information(this, tr("Information"), tr("Operation cancelled"));
        return;
    }
    //!由时间找到风速相对湿度文件{getWSRH()}
    QStringList WSRHFile=getWSRHFile(year,date);
    //! 比较读入影像和WSRH文件的长度,输出为文件{outputArguments()}
    QString outputArgumentFile=QString("./tmp/arguments%1.csv").arg(date);


   QVector<int>outWH=outputArguments(WSRHFile,selectedBand,QStringList({year,dayofYear}),outputArgumentFile);
    //! 调用R语言计算PM2.5{doPredict(参数文本文件路径)}
    QString outputResult=QString("./tmp/PredictPM%1.csv").arg(date);



    doPerdict(0,date,outputArgumentFile,outputResult,false);

    //        //!切换到pm2.5那一页
    ui->tabWidgetImage->setTabEnabled(1, true);
    ui->tabWidgetImage->setCurrentIndex(1);
    qDebug("dfdfdfd");

    //! {showResult()}
    connect(singlePwidget,&QWidget::destroyed,[=](){
        showResult(outputResult);
    });

}
void getPM2D5::doBatchProcess(){
    //!切换到输出窗口
    batchPwidget->ui->tabWidget->setCurrentIndex(1);
    //!禁止取消，禁止关闭
    batchPwidget->ui->pushButtonClose->setEnabled(false);

    //!判断时间时长与文件数是否相等
    //! TODO:跨年的问题,toJulianDay()
    int infileCount=batchFileNames.length();
    int lastDateJulian=batchPwidget->ui->dateEditLast->date().toJulianDay();
    int startDateJulian=batchPwidget->ui->dateEditStart->date().toJulianDay();
    int dateLength=lastDateJulian-startDateJulian;
    if(dateLength!=infileCount){
        QMessageBox::critical(this,"Error","时间长度与文件个数不符合");
        return;
    }

    //!循环单处理
    for (int i=0;i<batchFileNames.length() ;i++ ) {
        //!获取时间和当前波段
        QString year=QString::number(QDate::fromJulianDay(startDateJulian).year());
        QString date=QDate::fromJulianDay(startDateJulian).toString("yyyyMMdd");
        QString dayOfYear=QString::number(QDate::fromJulianDay(startDateJulian).dayOfYear());
        GDALDataset* currentDataset=(GDALDataset *)GDALOpen(batchFileNames[i].toLatin1(),GA_ReadOnly);
        GDALRasterBand *selectedBand=currentDataset->GetRasterBand(batchPwidget->ui->comboBoxOutBand->currentIndex()+1);
        double geoInfo[6];
        currentDataset->GetGeoTransform(geoInfo);
        const char *project = currentDataset->GetProjectionRef();
        //!由时间找到风速相对湿度文件{getWSRH()}
        QStringList WSRHFile=getWSRHFile(year,date);
        //! 比较读入影像和WSRH文件的长度,输出为文件{outputArguments()}
        QString outputArgumentFile=QString("./tmp/arguments%1.csv").arg(date);
       QVector<int>putWH=outputArguments(WSRHFile,selectedBand,QStringList({year,dayOfYear}),outputArgumentFile);
        //! 调用R语言计算PM2.5{doPredict(参数文本文件路径)}
        QString outputResult=QString("./tmp/PredictPM%1.csv").arg(date);
        doPerdict(i,date,outputArgumentFile,outputResult,false);
        //!connect finish
//        connect(batchPwidget,&QWidget::destroyed,[=](){
//            writeOutTIF(outputResult,"")
//        });

    }

}

void getPM2D5::writeOutTIF(QString predictFile,QString filePath,int outWidth,int outHeight,double geo[6],const char*projection){
    GDALDriver *driver=GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset *outputDataset=driver->Create(filePath.toLatin1(),outWidth,outHeight,1,GDT_Float32,nullptr);
    outputDataset->SetProjection(projection);
    outputDataset->SetGeoTransform(geo);
/////////////
    //读进来数据
    QFile pm25info(predictFile);
    if (!pm25info.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Internal error!"), tr("PM2.5文件读取失败"));
        return;
    }
    QTextStream pm25stream(&pm25info);
    auto *pm25array = new float[outWidth * outHeight];
    int i = 0;
    while (!pm25stream.atEnd()) {

        pm25array[i] = pm25stream.readLine().toFloat();
        i++;
    }

    CPLErr err = outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, outWidth, outHeight, pm25array, outWidth,
                                                           outHeight, GDT_Float32, 0, 0);
    if (err != CPLE_None) {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred "
                                                    "reading the data of the PM2.5 file"));
        return;
    }
///////////////////
}

//![1]
//! 初始化界面
void getPM2D5::init() {
    //![1]
    //! 初始隐藏图层样式窗口,隐藏渐变设置
    ui->dockWidgetLayerStyle->setHidden(true);
    ui->comboBoxGradient->setHidden(true);
    ui->labelGradient->setHidden(true);
    ui->Thermo->setHidden(true);

    //![2]
    //! 设置初始窗口为原图显示窗口，PM2.5显示窗口设置为不可用
    ui->tabWidgetImage->setCurrentIndex(0);
    ui->tabWidgetImage->setTabEnabled(1, false);

    //![3]
    //!全局信号与槽绑定
    connect(ui->actionOpen       , &QAction::triggered, this, &getPM2D5::loadOriginalImage);
    connect(ui->actionExecute    , &QAction::triggered, this, &getPM2D5::showSinglePwidget);
    connect(ui->actionBatch      , &QAction::triggered, this, &getPM2D5::chooseMultiFile);
    connect(ui->actionAbout      , &QAction::triggered, this, &getPM2D5::on_about);
    connect(ui->actionClear      , &QAction::triggered, this, &getPM2D5::clearWorkSpace);
    connect(ui->actionAboutQt    , &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->comboBoxShowMode , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &getPM2D5::showGradient);
    connect(ui->comboBoxShowBand , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &getPM2D5::changShowBand);


    singlePwidget = new SingleProcess;
    singlePwidget->setAttribute(Qt::WA_DeleteOnClose);


    //![4]
    //!注册GDAL全部文件驱动
    GDALAllRegister();
}
//![2]
//! 单处理时加载图像
void getPM2D5::loadOriginalImage() {
    //!打开文件选择对话框
    oImageName = QFileDialog::getOpenFileName(this, tr("Open a Image"), "./",
                                              tr("Tif File(*.tif *.tiff);;ENVI File (*.dat *.img)"));
    //!如果用户取消了文件选择，则什么也不做
    if (oImageName == "")return;
    //!载入文件
    poDataSet = (GDALDataset *) GDALOpen(oImageName.toStdString().c_str(), GA_ReadOnly);
    //!如果读取失败了，则弹出错误提示，并截断
    if (poDataSet == nullptr) {
        QMessageBox::critical(this, tr("Error!"),
                              tr("File opening failed. Please check the file"));
        return;
    }

    //!为单处理窗口赋初值

    singlePwidget->setImageFileName(oImageName);

    //!显示属性信息
    showImageAttribute(poDataSet);
    //!显示图层样式窗口
    ui->dockWidgetLayerStyle->setHidden(false);

}
//!显示数据集的属性信息
//! \brief getPM2D5::showImageAttribute
//! \param ImageDataset 待显示属性的影像数据集
//!
void getPM2D5::showImageAttribute(GDALDataset *ImageDataset) {
    //!传进来如果是空的数据集就什么也不做
    if (ImageDataset == nullptr) return;
    //!显示基本属性
    ui->tableWidget->item(0, 0)->setText(QString::number(ImageDataset->GetRasterXSize()));
    ui->tableWidget->item(1, 0)->setText(QString::number(ImageDataset->GetRasterYSize()));
    ui->tableWidget->item(4, 0)->setText(QString::number(ImageDataset->GetRasterCount()));
    ui->tableWidget->item(3, 0)->setText(GDALGetDataTypeName(ImageDataset->GetRasterBand(1)->GetRasterDataType()));
    //!显示投影信息
    QString projectionRef = ImageDataset->GetProjectionRef();
    if (projectionRef.isEmpty()) {
        ui->tableWidget->item(2, 0)->setText(tr("None"));
        QMessageBox::critical(this, "Error!", tr("The image has no projected information"));
    } else {
        QStringList projectionList = projectionRef.split('\"');
        QString projection = projectionList.at(1).toLocal8Bit().constData();
        ui->tableWidget->item(2, 0)->setText(projection);
        ui->tableWidget->item(2, 0)->setToolTip(projectionRef);
    }
    //!如果comboBoxShowBand非空，则清理一下
    //! 目的是避免新打开一张新影像造成内容的追加
    if(!ui->comboBoxShowBand->itemText(0).isEmpty())
    {
        ui->comboBoxShowBand->clear();
        singlePwidget->ui->comboBoxOutBand->clear();
    }
    //!comboBoxShowBand添加项目的同时，
    //!导致其currentIndex发生了变化(-1变成了0)，就调用了显示图像的函数
    for (int i = 0; i < poDataSet->GetRasterCount(); i++) {
        QString bandDescription = poDataSet->GetRasterBand(i + 1)->GetDescription();
        ui->comboBoxShowBand->addItem(QString(tr("Band %1: ")).arg(i + 1) + bandDescription);
        singlePwidget->ui->comboBoxOutBand->addItem(QString(tr("Band %1: ")).arg(i + 1) + bandDescription);
    }
}

void getPM2D5::chooseMultiFile(){
    batchFileNames=QFileDialog::getOpenFileNames(this, tr("Open some Image"), "./",
                                                 tr("Tif File(*.tif *.tiff);;ENVI File (*.dat *.img)"));
    if(batchFileNames.isEmpty())
        return;
    batchPwidget=new BatchProcess;
    batchPwidget->setAttribute(Qt::WA_DeleteOnClose);
    connect(batchPwidget->ui->pushButtonRun,&QPushButton::clicked,this,&getPM2D5::doBatchProcess);
    //!将文件名填充到tableWidget
    for(int i=0;i<batchFileNames.length();i++){
        batchPwidget->ui->tableWidget->insertRow(i);
        batchPwidget->ui->tableWidget->update();
        auto model=batchPwidget->ui->tableWidget->model();
        model->setData(model->index(i,1),batchFileNames[i]);
        model->setData(model->index(i,0),QFileInfo(batchFileNames[i]).suffix());
        batchPwidget->ui->tableWidget->item(i,1)->setToolTip(batchFileNames[i]);
    }
    //!假设都是输入文件都是一样的类型文件，波段相同
    GDALDataset *file1Dataset=(GDALDataset*)GDALOpen(batchFileNames[0].toLatin1(),GA_ReadOnly);
    for (int i = 0; i < file1Dataset->GetRasterCount(); i++) {
        QString bandDescription = file1Dataset->GetRasterBand(i + 1)->GetDescription();
        batchPwidget->ui->comboBoxOutBand->addItem(QString(tr("Band %1: ")).arg(i + 1) + bandDescription);
    }
    batchPwidget->show();
}

void getPM2D5::showSinglePwidget(){

    connect(singlePwidget->ui->pushButtonRun, &QPushButton::clicked,this,&getPM2D5::doSingleProcess);
    singlePwidget->ui->tabWidget->setCurrentIndex(0);
    singlePwidget->show();
}

//暂时不能自由选择波段，不过可以做。
// 只能显示单波段
void getPM2D5::changShowBand(int currentIndex) {
    if (poDataSet == nullptr)
        return;
    currentIndex = (currentIndex == -1) ? 0 : currentIndex;//抵消清理combox_3触发的信号，导致currentIndex==-1的情况

    currentBand = poDataSet->GetRasterBand(currentIndex + 1);
    showImage(currentBand);

    //!画直方图
    double minmax[2];
    currentBand->ComputeRasterMinMax(1, minmax);
    //    double min=minmax[0];
    double max = minmax[1];
    GUIntBig his[int(max)];
    currentBand->GetHistogram(minmax[0], max + 0.5, int(max), his, false, false, GDALDummyProgress, nullptr);
    QVector<GUIntBig> hisData;
    for (auto i:his)
        hisData.append(i);
    showHistogram(minmax[0], max + 0.5, int(max), hisData, currentIndex);
}

void getPM2D5::showImage(GDALRasterBand *displayBand) {
    if (displayBand==nullptr) {
        QMessageBox::critical(this, tr("Error Calls!"), tr("The band to be displayed is empty"));
        return;
    }
    int imWidth = displayBand->GetXSize();
    int imHeight = displayBand->GetYSize();
    auto *rBand = new float[imWidth * imHeight];
    uint8_t *rBand8UC3, *gBand8UC3, *bBand8UC3;
    /*读取传过来的三个波段的第一波段的数据，赋值给rBand*/
    CPLErr err;
    err = displayBand->RasterIO(GF_Read, 0, 0, imWidth, imHeight, rBand, imWidth, imHeight, GDT_Float32, 0, 0);
    if (err == CPLE_None) {
        rBand8UC3 = imageStretch(rBand, displayBand, imWidth * imHeight, -9999);
        bBand8UC3 = rBand8UC3;
        gBand8UC3 = rBand8UC3;

    } else {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred "
                                                    "reading the band of the input file"));
        return;
    }

    // 将三个波段组合起来
    int bytePerLine = (imWidth * 24 + 31) / 8;
    auto *allBandUC = new uint8_t[bytePerLine * imHeight * 3];
    for (int h = 0; h < imHeight; h++) {
        for (int w = 0; w < imWidth; w++) {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBand8UC3[h * imWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBand8UC3[h * imWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBand8UC3[h * imWidth + w];
        }
    }

    // 构造图像并显示
    auto *myImage = new QImage(allBandUC, imWidth, imHeight, bytePerLine, QImage::Format_RGB888);

    int displayWidth = ui->graphicsView->viewport()->width();
    int displayHeight = ui->graphicsView->viewport()->height();
    auto *newImage = new QImage;
    *newImage = myImage->scaled(displayWidth, displayHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
    //    delete  myImage;
    auto *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*newImage));
    auto *myScene = new QGraphicsScene;
    myScene->addItem(imgItem);
    ui->graphicsView->setScene(myScene);
    //        ui->graphicsView->fitInView(imgItem);
    //!不可用的都变成可用
    ui->actionExecute->setEnabled(true);
    ui->actionClear->setEnabled(true);
}

//noDataValue暂定为-9999
uint8_t *getPM2D5::imageStretch(float *buffer, GDALRasterBand *band, int bandShape, double noDataValue) {
    auto *returnBuffer = new uint8_t[bandShape];
    // minmax之所以是double是因为下面的函数返回值为double.
    band->ComputeRasterMinMax(1, minmax);
    double minDN = minmax[0];
    double maxDN = minmax[1];
    qDebug()<<minDN<<maxDN;
    if (minDN <= noDataValue) minDN = 0;

    for (int i = 0; i < bandShape; i++) {
        if (buffer[i] > maxDN) {
            returnBuffer[i] = 255;
        } else if (buffer[i] <= maxDN && buffer[i] >= minDN) {
            returnBuffer[i] = static_cast<uchar>(255 - 255 * (maxDN - buffer[i]) / (maxDN - minDN));
        } else if (buffer[i] == noDataValue) {
            returnBuffer[i] = 233;
        } else {
            returnBuffer[i] = 0;
        }
    }
    //
    //    for (int i = 0; i < bandShape; i++) {
    //        if (buffer[i] > maxDN) {
    //            returnBuffer[i] = 255;
    //        } else if (buffer[i] <= maxDN && buffer[i] >= minDN) {
    //            returnBuffer[i] = static_cast<uchar>(255 - 255 * ( maxDN -buffer[i] ) / ( maxDN - minDN ));
    //        }else {
    //            returnBuffer[i] = 233;
    //        }
    //    }


    return returnBuffer;
}
void getPM2D5::doPerdict(int index,QString date,QString argumentFile,QString outputPredictFilePath,bool isBatch){
    //!调用R语言计算
    QString Rprogram="Rscript";
    QString file="./R/script.r";
    QStringList arguments{file,argumentFile,outputPredictFilePath};
    auto *Rprocess=new QProcess;
    Rprocess->setReadChannel(QProcess::StandardError);

    connect(Rprocess,&QProcess::readyReadStandardError,[=](){
        singlePwidget->ui->textBrowserSingleP->append(Rprocess->readAllStandardError());
    });
    connect(Rprocess,&QProcess::started,[=](){
        singlePwidget->ui->textBrowserSingleP->append(QString("开始处理%1数据，请稍等。").arg(date));
    });
    connect(Rprocess,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),[=](int exitcode,QProcess::ExitStatus exitStatus){

        singlePwidget->ui->textBrowserSingleP->append(QString("%1进程已结束，返回错误代码为%2").arg(date).arg(exitcode));
        singlePwidget->ui->pushButtonClose->setEnabled(true);
    });

        connect(Rprocess,&QProcess::stateChanged,[=](QProcess::ProcessState state){
            if(state==QProcess::ProcessState::NotRunning){
                QMap<int,QProcess*>::iterator it=m_processMap.find(index);
                if(it!=m_processMap.end()){

                }
            }
        });
    Rprocess->start(Rprogram,arguments);
    m_processMap[index]=Rprocess;
    //  Rprocess->waitForFinished();
}

void getPM2D5::showResult(const QString &resultFile) {
    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset *outputDataset = driver->Create("./tmp/result.tif", outWidth, outHeight, 1, GDT_Float32, nullptr);
    double geoInfo[6];
    poDataSet->GetGeoTransform(geoInfo);
    const char *project = poDataSet->GetProjectionRef();

    outputDataset->SetGeoTransform(geoInfo);
    outputDataset->SetProjection(project);

    //读进来数据
    QFile pm25info(resultFile);
    if (!pm25info.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Internal error!"), tr("PM2.5文件读取失败"));
        return;
    }
    QTextStream pm25stream(&pm25info);
    auto *pm25array = new float[outWidth * outHeight];
    int i = 0;
    while (!pm25stream.atEnd()) {

        pm25array[i] = pm25stream.readLine().toFloat();
        i++;
    }

    CPLErr err = outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, outWidth, outHeight, pm25array, outWidth,
                                                           outHeight, GDT_Float32, 0, 0);
    if (err != CPLE_None) {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred "
                                                    "reading the data of the PM2.5 file"));
        return;
    }

    //!直方图
    //!读进来输出的文件
    double minmax[2];
    outputDataset->GetRasterBand(1)->ComputeRasterMinMax(1,minmax);
    double max = minmax[1];
    GUIntBig his[int(max)];
    currentBand->GetHistogram(minmax[0], max + 0.5, int(max), his, false, false, GDALDummyProgress, nullptr);
    QVector<GUIntBig> hisData;
    for (auto i:his)
        hisData.append(i);
    showHistogram(minmax[0], max + 0.5, int(max), hisData, 0);




    uint8_t *rBand8UC3, *gBand8UC3, *bBand8UC3;
    rBand8UC3 = imageStretch(pm25array, outputDataset->GetRasterBand(1), outWidth * outHeight, 0);
    bBand8UC3 = rBand8UC3;
    gBand8UC3 = rBand8UC3;

    // 将三个波段组合起来
    int bytePerLine = (outWidth * 24 + 31) / 8;
    auto *allBandUC = new uint8_t[bytePerLine * outHeight * 3];
    for (int h = 0; h < outHeight; h++) {
        for (int w = 0; w < outWidth; w++) {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBand8UC3[h * outWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBand8UC3[h * outWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBand8UC3[h * outWidth + w];
        }
    }

GDALClose(outputDataset);




    // 构造图像并显示
    QImage *myImage = new QImage(allBandUC, outWidth, outHeight, bytePerLine, QImage::Format_RGB888);

    if (!myImage->isNull()) {
        QMessageBox::information(this, tr("output"), tr("Run Successfully (๑˃̵ᴗ˂̵)👍 "));
    }
    int displayWidth = ui->graphicsView_2->width();
    int displayHeight = ui->graphicsView_2->height();
    auto *newImage = new QImage;
    *newImage = myImage->scaled(displayWidth, displayHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    //    delete  myImage;
    auto *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*newImage));
    auto *pm2D5Scene = new QGraphicsScene();
    pm2D5Scene->addItem(imgItem);
    ui->graphicsView_2->setScene(pm2D5Scene);


}



QStringList getPM2D5::getWSRHFile(QString year,QString date) {
    /*获取day日的WS和RH文件文件名*/
    QDir WSFileDir("./WSRHdata/WS-Warp-Henan/" + year);
    QDir RHFileDir("./WSRHdata/RH-Warp-Henan/" + year);
    QStringList fileFilter;
    if (WSFileDir.isEmpty() || RHFileDir.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Unable to perform operation"
                                                     " because wind speed data does not exist"));
        return {};
    }
    fileFilter << "*.tif";
    QStringList WSFiles = WSFileDir.entryList(fileFilter, QDir::Files | QDir::Readable, QDir::Name);
    QStringList RHFiles = RHFileDir.entryList(fileFilter, QDir::Files | QDir::Readable, QDir::Name);
    QString matchedWSFileName = "";
    QString matchedRHFileName = "";
    for (auto i:WSFiles) {
        QString filetime = i.split('.')[4].split('_')[0];
        if (filetime == date) {
            //风速文件和相对湿度文件文件名相同，文件夹不同
            matchedWSFileName = i;
            matchedRHFileName = i;
            break;
        }
    }
    QString matchedWSFilePath = WSFileDir.absolutePath() + QDir::separator() + matchedWSFileName;
    QString matchedRHFilePath = RHFileDir.absolutePath() + QDir::separator() + matchedRHFileName;
    if (!QFile(matchedWSFilePath).exists() || !QFile(matchedRHFilePath).exists()) {
        QMessageBox::critical(this, "Internal error!", "No matching wind speed"
                                                       " and relative humidity files found");
        return {};
    }
    return QStringList{matchedWSFilePath,matchedRHFilePath};
}

//!输出风速，相对湿度，影像，时间。并按最小长度输出，输出到一个文件
QVector<int> getPM2D5::outputArguments(QStringList WSRHFile,GDALRasterBand *outputBand,QStringList Year_dayOfYear,const QString& outputFile){
    //!输出风速相对湿度
    //TODO:将风速相对湿度文件输出到/tmp文件夹下QString(QStandardPaths::TempLocation)。由于调试需要，先放在执行文件文件夹下。

    auto *WSDataset = (GDALDataset *) GDALOpen(WSRHFile[0].toLatin1(), GA_ReadOnly);
    GDALRasterBand *WSBand = WSDataset->GetRasterBand(1);
    double WSextend[6];
    WSDataset->GetGeoTransform(WSextend);

    auto *RHDataset = (GDALDataset *) GDALOpen(WSRHFile[1].toLatin1(), GA_ReadOnly);
    GDALRasterBand *RHBand = RHDataset->GetRasterBand(1);
    double RHextend[6];
    RHDataset->GetGeoTransform(RHextend);




    //比较各文件的大小，以对齐
    int imageW = outputBand->GetXSize();
    int imageH = outputBand->GetYSize();
    int WSW = WSDataset->GetRasterXSize();
    int WSH = WSDataset->GetRasterYSize();
    int RHW = RHDataset->GetRasterXSize();
    int RHH = RHDataset->GetRasterYSize();

    QVector<int> W = {imageW, WSW, RHW};
    auto minW = std::min_element(std::begin(W), std::end(W));
    QVector<int> H = {imageH, WSH, RHH};
    auto minH = std::min_element(std::begin(H), std::end(H));
    outWidth = *minW;
    outHeight = *minH;



    QString filetime = QString(WSRHFile[0]).split('.')[4].split('_')[0];

    QFile oF(outputFile);
    if (!oF.open(QIODevice::ReadWrite | QIODevice::Text))
        return QVector<int>{0,0};
    QTextStream oFstream(&oF);
    auto *bandData=new float[outWidth*outHeight];
    auto *WSData=new float[outWidth*outHeight];
    auto *RHData=new float[outWidth*outHeight];

    QVector<CPLErr> errs;
    errs.append(outputBand->RasterIO(GF_Read, 0, 0, outWidth, outHeight, bandData, outWidth, outHeight, GDT_Float32, 0, 0));
    errs.append(WSBand->RasterIO(GF_Read, 0, 0, outWidth, outHeight, WSData, outWidth, outHeight, GDT_Float32, 0, 0));
    errs.append(RHBand->RasterIO(GF_Read, 0, 0, outWidth, outHeight, RHData, outWidth, outHeight, GDT_Float32, 0, 0));

    for(auto i:errs)
        if(i!=CPLE_None){
            QMessageBox::critical(this, tr("Error"), tr("An error occurred "
                                                        "reading the data of the PM2.5 Image"));
            return QVector<int>{0,0};
        }

    for (int row = 0; row < outHeight; row++) {
        for (int col = 0; col < outWidth; col++) {
            double x = WSextend[0] + col * WSextend[1] + row * WSextend[2];
            double y = WSextend[3] + col * WSextend[4] + row * WSextend[5];
            oFstream <<WSData[row * outWidth + col]<<","
                                                  <<RHData[row * outWidth + col]*100<<","
                                                 <<bandData[row * outWidth + col]/1000 << ","
                                                <<QString("%1%2").arg(Year_dayOfYear[0]).arg(Year_dayOfYear[1], 3, '0')<<","<< x << "," << y << "\n";
        }
    }
    singlePwidget->ui->textBrowserSingleP->append(tr("All files have been read"
                                                     " and the R language is about to be invoked"));
    return QVector<int>{outWidth,outHeight};
}


std::tuple<QString,QString,QString,GDALRasterBand*> getPM2D5::getSingleArgument()
{
    QString date = singlePwidget->ui->dateEdit->date().toString("yyyyMMdd");
    QString year = QString::number(singlePwidget->ui->dateEdit->date().year());
    QString dayOfYear = QString::number(singlePwidget->ui->dateEdit->date().dayOfYear());

    GDALDataset* selectedDataset=(GDALDataset *)GDALOpen(singlePwidget->ui->lineEditloadedImage->text().toLatin1(),GA_ReadOnly);
    GDALRasterBand *selectedBand=selectedDataset->GetRasterBand(singlePwidget->ui->comboBoxOutBand->currentIndex()+1);

    return std::make_tuple(year, date, dayOfYear,selectedBand);
    //已经在ui中限制了可选时间
}



void getPM2D5::showGradient(int currentIndex) {
    /**************************
     *   渐变控件的显示与隐藏     *
    ***************************/

    if (currentIndex == 1) {
        ui->Thermo->show();
        ui->comboBoxGradient->show();
        ui->labelGradient->show();
    } else {
        ui->Thermo->hide();
        ui->comboBoxGradient->hide();
        ui->labelGradient->hide();
    }
    //    GDALColorTable *ct;


}
void getPM2D5::clearWorkSpace() {
    poDataSet = nullptr;

    ui->graphicsView->scene()->clear();
    //    ui->graphicsView->viewport()->update();
    ui->tableWidget->item(0, 0)->setText("");
    ui->tableWidget->item(1, 0)->setText("");
    ui->tableWidget->item(2, 0)->setText("");
    ui->tableWidget->item(3, 0)->setText("");
    ui->tableWidget->item(4, 0)->setText("");
    ui->comboBoxShowBand->clear();
    ui->qwtPlotHistogram->setTitle(tr("Histogram"));
    ui->qwtPlotHistogram->detachItems();
    if (!ui->graphicsView_2->items().isEmpty())
        ui->graphicsView_2->scene()->clear();
}

void
getPM2D5::showHistogram(double dfMin, double dfMax, int nBuckets, QVector<GUIntBig> &histogramData, int currentID) {

    ui->qwtPlotHistogram->setTitle(QString(tr("Band%1 Histogram")).arg(currentID + 1));
    ui->qwtPlotHistogram->setAutoFillBackground(true);

    //!新建直方图对象
    auto *histPlot = new QwtPlotHistogram();

    //!设置直方图数据
    QVector<QwtIntervalSample> sample(nBuckets);
    double bucketsSize = (dfMax - dfMin) / nBuckets;
    for (int i = 0; i < nBuckets; i++) {
        QwtInterval intv(dfMin + i * bucketsSize, dfMin + (i + 1) * bucketsSize);
        intv.setBorderFlags(QwtInterval::ExcludeMaximum);
        sample[i] = QwtIntervalSample(histogramData[i], intv);
    }
    histPlot->setData(new QwtIntervalSeriesData(sample));
    ui->qwtPlotHistogram->detachItems();
    //!将直方图画在坐标系中
    histPlot->attach(ui->qwtPlotHistogram);
}

/*关于消息框,当点击菜单栏关于按钮后弹出*/
void getPM2D5::on_about() {
    QString text = tr("<h1>getPM2.5</h1><p><b>Version 1.0</b><br/>"
                      "Using R 4.0.2 and lmerTest 3.1-2</p>"
                      "<p>Model reference： YANG Liuan,XU Hanqiu,JIN Zhifan.Estimation of ground-level PM2.5"
                      " concentrations using MODIS satellite data in Fuzhou, China"
                      "[J].Journal of Remote Sensing,2018,22(01):64-75.</p>");

    QMessageBox::about(this, tr("About"), text);

}

getPM2D5::~getPM2D5() {
    delete ui;
}
