#include "singleprocess.h"
#include "ui_singleprocess.h"

SingleProcess::SingleProcess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleProcess)
{
    ui->setupUi(this);
}

void SingleProcess::setImageFileName(QString fileName)
{imageFileName=fileName;
    ui->lineEditloadedImage->setText(imageFileName);};

SingleProcess::~SingleProcess()
{
    delete ui;
}
