#ifndef SINGLEPROCESS_H
#define SINGLEPROCESS_H

#include <QWidget>

namespace Ui {
class SingleProcess;
}

class SingleProcess : public QWidget
{
    Q_OBJECT

public:
    explicit SingleProcess(QWidget *parent = nullptr);
    ~SingleProcess();
    Ui::SingleProcess *ui;
    void setImageFileName(QString fileName);
private:
    QString imageFileName;
protected:
//void closeEvent(QCloseEvent *event);
};

#endif // SINGLEPROCESS_H
