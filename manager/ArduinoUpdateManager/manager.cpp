#include "manager.h"
#include <QDomElement>
#include <cassert>
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>
#include "compressor.h"

bool OSRelease::haveFile(const QString &name) const
{
    foreach (const ReleaseFile &f, files) {
        if (f.name==name)
            return true;
    }
    return false;
}

ReleaseFile emptyFile;

const ReleaseFile &OSRelease::getFile(const QString &name) const
{
    foreach (const ReleaseFile &f, files) {
        if (f.name==name)
            return f;
    }
    return emptyFile;
}


bool Manager::exists(const ReleaseFile &f) const
{
    return m_shaMap.contains(f.sha);
}

void Manager::add(const ReleaseFile &f)
{
    m_shaMap[f.sha] = true;
}

static OSReleaseList emptyList;

const OSReleaseList &Manager::getReleaseList(const QString &os) const
{
    AllReleaseList::const_iterator it = m_releaseList.find(os);
    if (it==m_releaseList.end()) {
        qDebug()<<"no releases for os"<<os;
        return emptyList;
    }
    return it.value();
}

void Manager::updateReleasesFromXML(QDomElement rl)
{
    QDomElement r = rl.firstChildElement("Release");
    m_releaseList.clear();

    for (; !r.isNull(); r = r.nextSiblingElement("Release")) {
        QString osname = r.attribute("os");
        //assert();
        OSRelease rel;
        rel.name = r.attribute("name");
        rel.parent = r.attribute("parent");
        QDomElement fl;
        fl = r.firstChildElement("Files");
        QDomElement f = fl.firstChildElement("File");
        for (; !f.isNull(); f = f.nextSiblingElement("File")) {
            ReleaseFile rf;
            rf.sha = QByteArray::fromHex( f.attribute("sha").toLatin1());
            rf.name = f.attribute("target");
            rf.exec = f.attribute("exec") == "yes";
            rel.files.push_back(rf);
        }
        m_releaseList[osname].push_back(rel);
    }
}

void Manager::saveReleasesToXML(QDomDocument doc, QDomElement rl)
{
    while (rl.hasChildNodes()) {
        rl.removeChild(rl.firstChild());
    }
    foreach (const QString &os, m_releaseList.keys()) {
        qDebug()<<"Handling os"<<os;
        foreach (const OSRelease &osrl, m_releaseList[os]) {
            qDebug()<<"Handling release"<<osrl.name;
            QDomElement r = doc.createElement("Release");
            r.setAttribute("os",os);
            r.setAttribute("name",osrl.name);
            rl.appendChild(r);
            if (osrl.parent.size()) {
                r.setAttribute("parent",osrl.parent);
            }
            // Files...
            QDomElement files = doc.createElement("Files");
            r.appendChild(files);
            foreach (const ReleaseFile &f, osrl.files) {
                QDomElement file=doc.createElement("File");
                QString digest = f.sha.toHex();
                file.setAttribute("sha", digest );
                file.setAttribute("target", f.name);
                file.setAttribute("size", f.size);
                if (f.exec)
                    file.setAttribute("exec", "true");
                files.appendChild(file);
            }
        }
    }
}

void Manager::saveBranchesToXML(QDomDocument doc, QDomElement rl)
{
    while (rl.hasChildNodes()) {
        rl.removeChild(rl.firstChild());
    }
    foreach (const QString &os, m_releaseList.keys()) {
        qDebug()<<"Handling os"<<os;
        foreach (const OSRelease &osrl, m_releaseList[os]) {
            qDebug()<<"Handling release"<<osrl.name;
            QDomElement r = doc.createElement("Release");
            r.setAttribute("os",os);
            r.setAttribute("name",osrl.name);
            rl.appendChild(r);
            if (osrl.parent.size()) {
                r.setAttribute("parent",osrl.parent);
            }
            // Files...
            QDomElement files = doc.createElement("Files");
            r.appendChild(files);
            foreach (const ReleaseFile &f, osrl.files) {
                QDomElement file=doc.createElement("File");
                QString digest = f.sha.toHex();
                file.setAttribute("sha", digest );
                file.setAttribute("target", f.name);
                file.setAttribute("size", QString::number(f.size));
                if (f.exec)
                    file.setAttribute("exec", "true");
                files.appendChild(file);
            }
        }
    }
}

void Manager::addRelease(const QString &os, const OSRelease &release)
{
    m_releaseList[os].push_back(release);
}


void Manager::reset()
{
    m_releaseList.clear();
    m_osList.clear();
}

void Manager::addOS(const QString &os)
{
    m_osList.push_back(os);
}

void Manager::setDeployPath(const QString &path)
{
    m_deployPath = path;
}
const QString &Manager::getDeployPath() const
{
    return m_deployPath;
}

void Manager::setServerPath(const QString &path)
{
    m_serverPath = path;
}
const QString &Manager::getServerPath() const
{
    return m_serverPath;
}

void Manager::createLists()
{
    Compressor *c = new Compressor();

    foreach(const QString os, m_osList) {
        QString tmpl = m_deployPath + QDir::separator() + os + QDir::separator() + "updatelist-XXXXXX";
        QString final = m_deployPath + QDir::separator() + os + QDir::separator() + "updatelist.xml";
        QTemporaryFile temp(tmpl);
        temp.setAutoRemove(false);
        temp.open();
        createOSList(os, temp);
        temp.close();
        QFile f(final);
        if (f.exists())
            f.remove();
        temp.rename(final);

        if (c->compressFile(final, true)<0) {
            // Erorr...
        }

    }
    delete(c);
}

const OSRelease &Manager::getReleaseByName(const QString &name, const OSReleaseList &list) const
{
    foreach (const OSRelease &r, list) {
        if (r.name == name)
            return r;
    }
    qDebug()<<"Invalid release "<<name;
    throw "Invalid release";
}

OSRelease &Manager::getReleaseByName(const QString &name, OSReleaseList &list)
{
    for (OSReleaseList::iterator i = list.begin(); i!=list.end();i++) {
        if (i->name == name)
            return *i;
    }
    qDebug()<<"Invalid release "<<name;
    throw "Invalid release";
}

const ReleaseFile &Manager::getParentFile( const OSReleaseList &list, const OSRelease &release, const ReleaseFile &file )
{
    if (release.parent.size()==0)
        return emptyFile;

    const OSRelease &prelease = getReleaseByName(release.parent, list);
    if (prelease.name.size()) {
        return emptyFile;
    }
    return prelease.getFile(file.name);
}

QString Manager::getDownloadSize(const QString &sha)
{
    QString target = m_deployPath + QDir::separator() + "blobs" + QDir::separator() + sha + ".gz";

    QFile f(target);

    return QString::number(f.size());
}

void Manager::createOSList(const QString &os, QFile &file)
{
    QDomDocument d;
    uint rsid=0;

    QDomProcessingInstruction xmlDeclaration =
        d.createProcessingInstruction("xml", "version=\"1.0\"");
    d.appendChild(xmlDeclaration);

    QDomElement rootNode = d.createElement("UpdateList");
    QDomElement configNode = d.createElement("Configuration");
    rootNode.appendChild(configNode);
    d.appendChild(rootNode);

    QDomElement url = d.createElement("URL");
    url.appendChild(d.createTextNode( getServerPath() ));
    configNode.appendChild(url);

    QDomElement resources = d.createElement("Resources");
    rootNode.appendChild(resources);

    QDomElement releases = d.createElement("Releases");
    rootNode.appendChild(releases);

    QDomElement branches = d.createElement("Branches");
    configNode.appendChild(branches);

    foreach (const OSBranch &b, getBranchList(os)) {
        QDomElement branch = d.createElement("Branch");
        branch.setAttribute("name", b.name);
        branch.setAttribute("leaf", b.leaf);
        branches.appendChild(branch);
    }

    /* Add resources and releases */

    QHash<SHA,uint> shaMap;
    QHash<QString,bool> filesMap;

    foreach (const OSRelease &r, getReleaseList(os)) {
        uint thisId;
        QDomElement release = d.createElement("Release");
        release.setAttribute("name", r.name);
        release.setAttribute("parent", r.parent);
        releases.appendChild(release);
        foreach (const ReleaseFile &f, r.files) {

            const ReleaseFile &parentFile = getParentFile( getReleaseList(os), r, f);
            bool addFile = true;

            if (parentFile.name.size()) {
                // Same SHA ?
                if (parentFile.sha == f.sha) {
                    addFile = false;
                }
            }
            if (addFile) {
                if (shaMap.find(f.sha) == shaMap.end()) {
                    rsid++;
                    thisId=rsid;
                    shaMap[f.sha]=rsid;
                    QDomElement rs = d.createElement("Res");
                    rs.setAttribute("id",rsid);
                    rs.setAttribute("sha",QString(f.sha.toHex()));
                    rs.setAttribute("size", QString::number(f.size));
                    rs.setAttribute("dsize", getDownloadSize(f.sha.toHex()));
                    resources.appendChild(rs);
                } else {
                    thisId = shaMap[f.sha];
                }
                QDomElement file = d.createElement("File");
                file.setAttribute("target", f.name);
                file.setAttribute("rsid", thisId);
                if (f.exec)
                    file.setAttribute("exec", "yes");
                release.appendChild(file);
            }
            filesMap[f.name]=true;
        }
        /* Now, iterate through all parent files. If not present any more,
         deprecate them. */
        if (r.parent.size()) {
            foreach (const ReleaseFile &f, getReleaseByName(r.parent,getReleaseList(os)).files) {
                if (filesMap.find(f.name)==filesMap.end()) {
                    QDomElement file = d.createElement("File");
                    file.setAttribute("target", f.name);
                    file.setAttribute("rsid", "-1");
                    release.appendChild(file);
                    qDebug()<<f.name<<"deprecated.";
                } 
            }
        }
    }


    QTextStream stream( &file );
    qDebug()<<file.fileName();
    stream << d.toString();
    stream.flush();
}

void Manager::updateBranchesFromXML(QDomElement rl)
{
    QDomElement r = rl.firstChildElement("Branch");
    m_branchList.clear();

    for (; !r.isNull(); r = r.nextSiblingElement("Branch")) {
        QString osname = r.attribute("os");
        //assert();
        OSBranch br;
        br.name = r.attribute("name");
        br.leaf = r.attribute("leaf");
        br.description = br.name; // TODO
        m_branchList[osname].push_back(br);
    }
}


static OSBranchList noBranchList;

const OSBranchList &Manager::getBranchList(const QString &os) const
{
    AllBranchList::const_iterator it = m_branchList.find(os);
    if (it==m_branchList.end()) {
        return noBranchList;
    }
    return it.value();
}

void Manager::setBranchLeaf(const QString &os, const QString &branch, const QString &leaf)
{
    AllBranchList::iterator it = m_branchList.find(os);
    if (it!=m_branchList.end()) {
        OSBranchList::iterator i;

        for ( i=it->begin(); i!=it->end(); i++ ) {
            if (i->name==branch) {
                i->leaf=leaf;
                break;
            }
        }
    }
}



void Manager::setParentRelease(const QString &os, const QString &release, const QString &parent)
{
    OSRelease &r = getReleaseByName(release, m_releaseList[os]);
    r.parent = parent;
}

