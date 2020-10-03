#ifndef BATCHPROCESS_H
#define BATCHPROCESS_H

#include <QWidget>

namespace Ui {
class BatchProcess;
}

class BatchProcess : public QWidget
{
    Q_OBJECT

public:
    explicit BatchProcess(QWidget *parent = nullptr);
    ~BatchProcess();
    Ui::BatchProcess *ui;
private:

};

#endif // BATCHPROCESS_H
