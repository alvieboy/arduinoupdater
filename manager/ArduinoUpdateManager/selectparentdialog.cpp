#include "selectparentdialog.h"
#include "ui_selectparentdialog.h"

SelectParentDialog::SelectParentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectParentDialog)
{
    ui->setupUi(this);
}

SelectParentDialog::~SelectParentDialog()
{
    delete ui;
}

void SelectParentDialog::addRelease(const QString&s)
{
    ui->parentReleaseCombo->addItem(s);
}

QString SelectParentDialog::getParent()const {
    return ui->parentReleaseCombo->currentText();
}
