#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QtXml/QXmlReader>
#include <QErrorMessage>
#include <QStandardItemModel>
#include "scanner.h"
#include "releasedialog.h"
#include "selectparentdialog.h"
#include "compressor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    master(NULL)
{
    ui->setupUi(this);
    ui->centralWidget->hide();

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(contextualMenu(const QPoint &))
           );
}

QString MainWindow::currentOS()
{
    return ui->osComboBox->currentText();
}

void MainWindow::onAddRelease()
{
    QModelIndex index = ui->treeView->currentIndex();
    ReleaseDialog d(this, !index.isValid());

    d.addRelease("<none>");
    foreach (const OSRelease &r,manager.getReleaseList( currentOS() )) {
        d.addRelease( r.name );
    }
    d.exec();
    uint newfiles=0, modifiedfiles=0, removedfiles=0;

    QString dir = d.getDirectory();
    QString name = d.getReleaseName();
    QString parent = d.getParent();

    if (dir.size()==0)
        return;

    Scanner scanner;
    ReleaseFileList l;
    scanner.scan(l, dir, manager.getDeployPath());

    if (parent=="<none>")
        parent.clear();

    OSRelease release;
    release.name = name;
    release.parent = parent;

    foreach(const ReleaseFile &f, l) {
        if (manager.exists(f)) {
        } else {
            manager.add(f);
            newfiles++;
        }
        release.files.push_back(f);
    }

    manager.addRelease(ui->osComboBox->currentText(),release);

    updateReleaseList(currentOS());

    qDebug()<<"Found "<<l.size()<<" files";
    qDebug()<<"Of those,"<<newfiles<<" are new.";

    // Confirm release
}

QString MainWindow::getCurrentBranch()
{
    return ui->branchComboBox->currentText();
}

void MainWindow::onSetParent()
{
    QModelIndex index = ui->treeView->currentIndex();
    QVariant v = m_releaseModel->data(index);

    SelectParentDialog d;
    d.addRelease("<none>");
    foreach (const OSRelease &r, manager.getReleaseList(currentOS())) {
        d.addRelease(r.name);
    }
    if (d.exec()) {
        QString parent =d.getParent();
        if (parent=="<none>")
            parent="";
        manager.setParentRelease(currentOS(), v.toString(), parent);
        updateReleaseList(currentOS());
    }
}

void MainWindow::onSetBranchLeaf()
{
    QModelIndex index = ui->treeView->currentIndex();
    QVariant v = m_releaseModel->data(index);

    qDebug()<<"Set branch"<<getCurrentBranch()<<"leaf to"<< v.toString(); 

    manager.setBranchLeaf(currentOS(),getCurrentBranch(),v.toString());
}

void MainWindow::contextualMenu(const QPoint &p)
{
    QModelIndex index = ui->treeView->currentIndex();

    QMenu *menu = new QMenu;
    menu->addAction("New release...", this, SLOT(onAddRelease()));

    if (index.isValid()) {
        menu->addAction("Set parent...", this, SLOT(onSetParent()));
    }
    /* We need a branch. */
    if (getCurrentBranch().size()) {
        menu->addAction("Set as leaf for '"+ getCurrentBranch()+ "' branch", this, SLOT(onSetBranchLeaf()));
    }
    menu->exec(QCursor::pos());
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpen()
{
    QFileDialog *d = new QFileDialog(this,
                                     tr("Open file"),
                                     ".",
                                     "*.xml");

    d->exec();
    if (d->result()) {
        openFile( d->selectedFiles().first() );
    }
    delete(d);
}

void MainWindow::onOSChanged(QString name)
{
    qDebug()<<"OS changed to"<<name;

    // Update release list.
    updateReleaseList(name);
    updateBranches(name);
}

void MainWindow::onNewRelease()
{
    ReleaseDialog d(this,true);
    d.exec();
}

void MainWindow::onExit()
{
    

}

void MainWindow::onSave()
{
    qDebug()<<"Saving";
    save();
}

void MainWindow::save()
{

    if ((currentFile.size()==0) || (master==NULL))
        return;

    manager.saveReleasesToXML(*master, releasesNode);
    QFile outFile( currentFile );

    if( !outFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        qDebug( "Failed to open file for writing." );
        return;
    }

    QTextStream stream( &outFile );
    stream << master->toString();
    qDebug()<<"Saved to file "<<currentFile;
    stream.flush();
    outFile.close();

    manager.createLists();
}

void MainWindow::openFile(const QString &name)
{
    QDomDocument *d = new QDomDocument();
    if (master) {
        delete(master);
        master=NULL;
    }
    qDebug()<<"Opening "<<name;
    QFile file(name);
    currentFile=name;
    if (d->setContent(&file)) {
        master=d;
        if (parseCurrentFile()<0)
        {
            delete(master);
            master=NULL;
            currentFile="";
        }
    }
    file.close();
}

void MainWindow::showError(const QString &msg)
{
    QErrorMessage m;
    m.showMessage(msg);
    m.exec();

}

int MainWindow::parseCurrentFile()
{
    if (master==NULL)
        return -1;

    manager.reset();

    QDomElement docElem = master->documentElement();

    if (docElem.tagName() != "ManagerConfig") {
        showError("Invalid XML file specified.");
        return -1;
    }

    QDomElement operatingSystems = docElem.firstChildElement("OperatingSystems");

    ui->osComboBox->clear();

    QDomElement os = operatingSystems.firstChildElement("OS");

    ui->osComboBox->setEnabled(false);

    for (; !os.isNull(); os = os.nextSiblingElement("OS")) {
        QString osname =  os.attribute("name");
        manager.addOS(osname);
    }

    QDomElement deployNode = docElem.firstChildElement("DeployLocation");

    manager.setDeployPath(deployNode.text());

    QDomElement serverNode = docElem.firstChildElement("ServerLocation");

    manager.setServerPath(serverNode.text());

    releasesNode = docElem.firstChildElement("Releases");
    manager.updateReleasesFromXML(releasesNode);

    QDomElement branchesNode = docElem.firstChildElement("Branches");
    manager.updateBranchesFromXML(branchesNode);

    qDebug()<<"Loaded all releases";

   
    os = operatingSystems.firstChildElement("OS");
    for (; !os.isNull(); os = os.nextSiblingElement("OS")) {
        QString osname =  os.attribute("name");
        ui->osComboBox->addItem(osname);
        manager.addOS(osname);
    }

    ui->centralWidget->show();
    ui->osComboBox->setEnabled(true);


    return 0;

}

void MainWindow::updateReleaseList(const QString &os)
{
    /* Load all releases */
    if (os.size()==0)
        return;

    qDebug()<<"Updating list for os"<<os;

    QTreeView *view = ui->treeView;

    m_releaseModel = new QStandardItemModel ;

    QStandardItem *rootNode = m_releaseModel->invisibleRootItem();

    QHash<QString,QStandardItem*> items;

    foreach (const OSRelease &r, manager.getReleaseList(os)) {
        QStandardItem *item = new QStandardItem( r.name );
        // Assert
        items[ r.name ] = item;
        item->setEditable(false);
        qDebug()<<"New release"<<r.name;
    }
    /* Link them */
    foreach (const OSRelease &r, manager.getReleaseList(os)) {
        QStandardItem *item = items.find(r.name).value();
        if (r.parent.size()==0  || items.find(r.parent)==items.end()) {
            qDebug()<<"No parent";
            rootNode->appendRow( item );
        } else {
            items[ r.parent ]->appendRow( item );
        }
    }
    view->setModel( m_releaseModel );
}

void MainWindow::updateBranches(const QString &os)
{
    /* Load all releases */
    if (os.size()==0)
        return;

    qDebug()<<"Updating branches for os"<<os;

    ui->branchComboBox->clear();

    foreach (const OSBranch &b, manager.getBranchList(os)) {
        ui->branchComboBox->addItem( b.name );
    }
}
