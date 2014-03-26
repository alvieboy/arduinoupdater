#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QDebug"
#include "QtXml/QXmlReader"
#include "QErrorMessage"
#include <QStandardItemModel>
#include "scanner.h"
#include "releasedialog.h"

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

void MainWindow::contextualMenu(const QPoint &p)
{
    QModelIndex index = ui->treeView->currentIndex();

    QMenu *menu = new QMenu;
    menu->addAction("New release...", this, SLOT(onAddRelease()));

    if (index.isValid()) {
        menu->addAction("Set parent...");
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
}

void MainWindow::onNewRelease()
{
    ReleaseDialog d(this,true);
    /*
    foreach (const QString &r, manager.getReleaseList(ui->osComboBox->currentText())) {
        d.addRelease(r);
        }
        */
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

    qDebug()<<"Loaded all releases";

    /* Parse the configuration items */
    /*
     <Configuration>
     <URL base="http://localhost/~alvieboy/arduino/"/>
     <Branches>
     <Branch name="master" leaf="1.0.0">
     <Description lang="en">Arduino 1.0.X Series</Description>
     </Branch>
     <Branch name="1.5" leaf="1.0.0">
     <Description lang="en">Arduino 1.5.X Series</Description>
     </Branch>
     <Branch name="development" leaf="1.0.1">
     <Description lang="en">Arduino 1.5.X Development</Description>
     </Branch>
     </Branches>
     </Configuration>
     */


    
#if 0


    //defining a couple of items
    QStandardItem *americaItem = new QStandardItem("Release 1.0.0");
    QStandardItem *mexicoItem =  new QStandardItem("Release 1.1");
    QStandardItem *usaItem =     new QStandardItem("Release 1.1.1");
    QStandardItem *bostonItem =  new QStandardItem("Release 1.1.2");
    QStandardItem *europeItem =  new QStandardItem("Release 2.0.0");
    QStandardItem *italyItem =   new QStandardItem("Release 2.0.1 alpha");
    QStandardItem *romeItem =    new QStandardItem("Release 2.0.1 beta 1");
    QStandardItem *veronaItem =  new QStandardItem("Release 2.0.1 beta 2");
    QStandardItem *nItem =  new QStandardItem("Release 2.0.2");

    //building up the hierarchy
    rootNode->    appendRow(americaItem);
    rootNode->    appendRow(europeItem);
    americaItem-> appendRow(mexicoItem);
    mexicoItem-> appendRow(usaItem);
    usaItem->     appendRow(bostonItem);
    europeItem->  appendRow(italyItem);
    europeItem->   appendRow(romeItem);
    europeItem->   appendRow(veronaItem);
    veronaItem->appendRow(nItem);
 
#endif
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
    QStandardItemModel *standardModel = new QStandardItemModel ;
    QStandardItem *rootNode = standardModel->invisibleRootItem();

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
        if (r.parent.size()==0  || items.find(r.parent)==items.end()) {
            qDebug()<<"No parent";
            rootNode->appendRow( items.find(r.name).value() );
        } else {

        }
    }
    view->setModel( standardModel );
}
