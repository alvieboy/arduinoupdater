#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <QString>
#include <QByteArray>
#include <QVector>
#include <QStack>
#include <QDir>
#include "manager.h"

class Scanner
{
public:
    Scanner();
    void scan(ReleaseFileList &r, const QString &directory, const QString &deployPath);
protected:
    void scanRecursive(const QString &deployPath,QDir &d, ReleaseFileList &r, QStack<QString> &dirname);
    void deployFile(const QString &deployPath,QFile &file, const QString &sha);

    QByteArray hashFile(QFile&);
};

#endif
