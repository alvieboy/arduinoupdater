#ifndef RELEASEDIALOG_H
#define RELEASEDIALOG_H

#include <QDialog>

namespace Ui {
class ReleaseDialog;
}

class ReleaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseDialog(QWidget *parent = 0, bool bare=false);
    ~ReleaseDialog();
    QString getReleaseName() const;
    QString getParent() const;
    void addRelease(const QString &);
    QString getDirectory() const;

public slots:
    void onChooseDirectory();

private:
    Ui::ReleaseDialog *ui;
};

#endif // RELEASEDIALOG_H
