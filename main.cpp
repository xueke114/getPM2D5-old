#include "getpm2d5.h"

#include <QApplication>
#include<QTranslator>
#include<QLocale>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

#ifdef Q_OS_WIN
    if (QLocale::system().country() == QLocale::China) {
        a.setFont(QFont("Microsoft Yahei", 9, QFont::Normal, false));
    }
#endif

    auto *getpmtt= new QTranslator(&a);
    getpmtt->load(QLocale::system(), "getPM2D5", "_", ":/i18n");
    QApplication::installTranslator(getpmtt);
    getPM2D5 w;
    w.show();
    return QApplication::exec();
}

//    //渐变色的图例
//    int ww=ui->Thermo_2->width();
//    QLinearGradient gradient(0, 0, ww, 0);
//    gradient.setColorAt(0, QColor("#050FD9"));
//    gradient.setColorAt(0.1,QColor("#2050FF"));
//    gradient.setColorAt(0.3,QColor("#86DAFF"));
//    gradient.setColorAt(0.4, QColor("#AFF6FF"));
//    gradient.setColorAt(0.5,QColor("#CFFFFF"));
//    gradient.setColorAt(0.6,QColor("#FFEC00"));
//    gradient.setColorAt(0.7,QColor("#FF9000"));
//    gradient.setColorAt(0.8, QColor("#FF4900"));
//    gradient.setColorAt(0.9,QColor("#D50000"));
//    gradient.setColorAt(1, QColor("#9F0000"));
//    //    gradient.setColorAt(0.6,Qt::yellow);

//    QBrush brush(gradient);
//    ui->Thermo_2->setValue(100);
//    ui->Thermo_2->setFillBrush(brush);
//    QColor co=brush.color();

//uint8_t *getPM2D5::imageClassRGB(float *buffer, int bandShape, double noDataValue){
//    /*颜色分为8份
//    A 255 255 204 #FFFFCC
//    B 255, 237, 160 #FFEDA0
//    C 254, 217, 118 #FED976
//    D 254, 178, 76 #FEB24C
//    E 253, 141, 60 #FD8D3C
//    F 252, 78, 42 #FC4E2A
//    G 227, 26, 28 #E31A1C
//    H 117, 0 ,38 #B10026
//*/
//    //    int ww=ui->Thermo_2->width();
//    //    QLinearGradient gradient(0, 0, ww, 0);
//    //    gradient.setColorAt(0, QColor("#050FD9"));
//    //    gradient.setColorAt(0.1,QColor("#2050FF"));
//    //    gradient.setColorAt(0.3,QColor("#86DAFF"));
//    //    gradient.setColorAt(0.4, QColor("#AFF6FF"));
//    //    gradient.setColorAt(0.5,QColor("#CFFFFF"));
//    //    gradient.setColorAt(0.6,QColor("#FFEC00"));
//    //    gradient.setColorAt(0.7,QColor("#FF9000"));
//    //    gradient.setColorAt(0.8, QColor("#FF4900"));
//    //    gradient.setColorAt(0.9,QColor("#D50000"));
//    //    gradient.setColorAt(1, QColor("#9F0000"));


//    //    QVector<QRgb> myTable(256);
//    //    for(int i=0;i<255;i++)
//    //        myTable[i]=qRgb(255-i,i,i);
//    //    uint8_t *returnBuffer=new uint8_t[bandShape];



//    //    return returnBuffer;
//}
