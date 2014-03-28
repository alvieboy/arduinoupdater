#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <QString>
#include <QByteArray>
#include <QVector>
#include <QHash>
#include <QtXml/QDomDocument>
#include <cassert>
#include <QFile>

typedef QByteArray SHA;

struct ReleaseFile
{
    QString name;
    SHA sha;
    bool exec;
    qint64 size;
};


typedef QVector<ReleaseFile> ReleaseFileList;

struct OSRelease
{
    QString name;
    ReleaseFileList files;
    QString parent;
    bool haveFile(const QString &name) const;
    const ReleaseFile &getFile(const QString &name) const;
};

struct OSBranch
{
    QString name;
    QString description;
    QString leaf;
};

typedef QVector<OSRelease> OSReleaseList;
typedef QVector<OSBranch> OSBranchList;

typedef QHash<QString, OSReleaseList> AllReleaseList;
typedef QHash<QString, OSBranchList> AllBranchList;
class Manager
{
public:
    bool exists(const ReleaseFile &f) const ;
    void add   (const ReleaseFile &f);
    const OSReleaseList &getReleaseList(const QString &os) const;
    const OSBranchList &getBranchList(const QString &os) const;

    void updateReleasesFromXML(QDomElement release);
    void updateBranchesFromXML(QDomElement release);
    void saveBranchesToXML(QDomDocument doc, QDomElement bl);
    void saveReleasesToXML(QDomDocument doc, QDomElement rl);
    void addRelease(const QString &os, const OSRelease &release);
    void setParentRelease(const QString &os, const QString &release, const QString &parent);

    void reset();
    void addOS(const QString &os);
    void setDeployPath(const QString&path);
    void setServerPath(const QString&path);
    const QString &getDeployPath() const;
    const QString &getServerPath() const;
    void createLists();
protected:
    void createOSList(const QString &os, QFile &file);
    bool isNewFile(const OSReleaseList &list, const OSRelease &release, const ReleaseFile &file);
    const OSRelease &getReleaseByName(const QString &name, const OSReleaseList &list) const;
    OSRelease &getReleaseByName(const QString &name, OSReleaseList &list);

    const ReleaseFile &getParentFile( const OSReleaseList &list, const OSRelease &release, const ReleaseFile &file );


private:
    /* Fast list with all hashes */
    QHash<SHA,quint32> m_shaMap;
    /* Ordered resources */
    QVector<SHA> m_resources;
    AllReleaseList m_releaseList;
    AllBranchList m_branchList;
    QString m_deployPath;
    QString m_serverPath;
    QVector<QString> m_osList;
};

#endif
