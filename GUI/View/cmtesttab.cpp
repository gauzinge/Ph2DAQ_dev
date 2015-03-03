#pragma once

#include "cmtesttab.h"
#include "ui_cmtesttab.h"

namespace GUI {

    CmTestTab::CmTestTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CmTestTab)
    {
        ui->setupUi(this);
    }

    CmTestTab::~CmTestTab()
    {
        delete ui;
    }

    void CmTestTab::getIsNoiseScan()
    {
        emit sendIsScan(ui->chbxScanNoise->isChecked());
    }

    void CmTestTab::disableLaunch()
    {
        ui->btnLaunch->setEnabled(false);
    }

    void CmTestTab::enableLaunch()
    {
        ui->btnLaunch->setEnabled(true);
    }

    void CmTestTab::on_btnLaunch_clicked()
    {
        emit startCmTest();
    }


}