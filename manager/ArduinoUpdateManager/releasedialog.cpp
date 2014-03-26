#include "releasedialog.h"
#include "ui_releasedialog.h"
#include <QFileDialog>
#include <QDir>


ReleaseDialog::ReleaseDialog(QWidget *parent, bool bare) :
    QDialog(parent),
    ui(new Ui::ReleaseDialog)
{
    ui->setupUi(this);
    if (bare) {

        ui->parentReleaseLabel->hide();
        ui->releaseComboBox->hide();
    }
}

ReleaseDialog::~ReleaseDialog()
{
    delete ui;
}

void ReleaseDialog::addRelease(const QString &s)
{
    ui->releaseComboBox->addItem(s);
}

void ReleaseDialog::onChooseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select installation directory"));
    if (dir.size()) {
        // Just check if it's a directory
        if (QDir(dir).exists()) {
            ui->directory->setText(dir);
        }
    }
}

QString ReleaseDialog::getDirectory() const
{
    return ui->directory->text();
}

QString ReleaseDialog::getReleaseName() const
{
    return ui->releaseName->text();
}

QString ReleaseDialog::getParent() const
{
    return ui->releaseComboBox->currentText();
}
