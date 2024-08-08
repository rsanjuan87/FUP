#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "config.h"

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

public slots:
    void accept();
    void editAdbPath();
    void editScrcpyPath();
    void editScrcpyServerPath();
    void toggleCustomAdbPath(bool v);
    void toggleCustomScrcpyPath(bool v);
    void toggleCustomScrcpyServerPath(bool v);
    void setAdbPath(QString v);
    void setScrcpyPath(QString v);
    void setScrcpyServerPath(QString v);
private:
    Ui::ConfigDialog *ui;
    Config* config;
};

#endif // CONFIGDIALOG_H
