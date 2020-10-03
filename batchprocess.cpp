#include "batchprocess.h"
#include "ui_batchprocess.h"

BatchProcess::BatchProcess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BatchProcess)
{
    ui->setupUi(this);
}

BatchProcess::~BatchProcess()
{
    delete ui;
}
