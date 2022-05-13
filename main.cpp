#include "getpm2d5.h"

#include <QApplication>
#include<QTranslator>
#include<QLocale>

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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
