#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtXml/QDomDocument>
#include <QModelIndex>
#include <QStandardItemModel>

#include "updatelistmodel.h"
#include "manager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onOpen();
    void onExit();
    void onSave();
    void onNewRelease();
    void onAddRelease();
    void onSetParent();
    void onSetBranchLeaf();
    void onOSChanged(QString);
    void contextualMenu(const QPoint &p);
    void updateReleaseList(const QString &os);
    void updateBranches(const QString &os);
protected:
    QString currentOS();
    QString getCurrentBranch();
    void save();
    void openFile(const QString &);
    int parseCurrentFile();
    void showError(const QString &msg);

private:
    Ui::MainWindow *ui;
    QDomDocument *master;
    QString currentFile;
    Manager manager;
    QDomElement releasesNode;
    QStandardItemModel *m_releaseModel;
};

#endif // MAINWINDOW_H
