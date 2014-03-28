#ifndef SELECTPARENTDIALOG_H
#define SELECTPARENTDIALOG_H

#include <QDialog>

namespace Ui {
class SelectParentDialog;
}

class SelectParentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectParentDialog(QWidget *parent = 0);
    ~SelectParentDialog();
    void addRelease(const QString&);
    QString getParent()const;
private:
    Ui::SelectParentDialog *ui;
};

#endif // SELECTPARENTDIALOG_H
