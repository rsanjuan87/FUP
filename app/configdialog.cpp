#include "configdialog.h"
#include "ui_configdialog.h"

#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConfigDialog){
    this->config = new Config();
    ui->setupUi(this);
    ui->adbEdit->setText(config->adbPath(false).remove("!"));
    ui->adbCheck->setChecked(config->adbPath(false) != Config::adbDefaultPath ||
                             config->adbPath(false).startsWith("!"));

    ui->scrcpyEdit->setText(config->scrcpyPath(false).remove("!"));
    ui->scrcpyCheck->setChecked(config->scrcpyPath(false) !=
                                    Config::scrcpyDefaultPath ||
                                config->scrcpyPath(false).startsWith("!"));

    ui->scrcpyServerEdit->setText(config->scrcpyServerPath(false).remove("!"));
    ui->scrcpyServerCheck->setChecked(
        config->scrcpyServerPath(false) != Config::scrcpyServerDefaultPath ||
        config->scrcpyServerPath(false).startsWith("!"));


    ui->castAudioCheck->setChecked(config->coherenceMode);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::accept(){
    config->save();
    QDialog::accept();
}

void ConfigDialog::editAdbPath(){
    QString v = QFileDialog::getOpenFileName(this, tr("Select adb executable"),
                                 config->adbPath(false));
    if(!v.isEmpty()){
        QString a = ui->adbCheck->isChecked() ? "" : "!";
        config->setAdbPath(a+v);
        ui->adbEdit->setText(v);
    }
}

void ConfigDialog::editScrcpyPath(){
    QString v = QFileDialog::getOpenFileName(this, tr("Select scrcpy executable"),
                                             config->scrcpyPath(false));
    if(!v.isEmpty()){
        QString a = ui->scrcpyCheck->isChecked() ? "" : "!";
        config->setScrcpyPath(a+v);
        ui->scrcpyEdit->setText(v);
    }
}

void ConfigDialog::editScrcpyServerPath(){

    QString v = QFileDialog::getOpenFileName(this, tr("Select scrcpy-server executable"),
                                             config->scrcpyServerPath(false));
    if(!v.isEmpty()){
        QString a = ui->scrcpyServerCheck->isChecked() ? "" : "!";
        config->setScrcpyServerPath(a+v);
        ui->scrcpyServerEdit->setText(v);
    }
}

void ConfigDialog::toggleCustomAdbPath(bool v){
    if(!v){
        config->setAdbPath("!"+config->adbPath(false));
    }else{
        config->setAdbPath(config->adbPath(false).remove("!"));
    }
}

void ConfigDialog::toggleCustomScrcpyPath(bool v){
    if(!v){
        config->setScrcpyPath("!"+config->scrcpyPath(false));
    }else{
        config->setScrcpyPath(config->scrcpyPath(false).remove("!"));
    }
}

void ConfigDialog::toggleCustomScrcpyServerPath(bool v){
    if(!v){
        config->setScrcpyServerPath("!"+config->scrcpyServerPath(false));
    }else{
        config->setScrcpyServerPath(config->scrcpyServerPath(false).remove("!"));
    }
}

void ConfigDialog::setAdbPath(QString v){
    config->setAdbPath(v);
}

void ConfigDialog::setScrcpyPath(QString v){
    config->setScrcpyPath(v);
}

void ConfigDialog::setScrcpyServerPath(QString v){
    config->setScrcpyServerPath(v);
}
