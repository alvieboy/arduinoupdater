#include "scanner.h"
#include <QDir>
#include <QFile>
#include <QStack>
#include <QDirIterator>
#include <QDebug>
#include <QCryptographicHash>
#include <QStringList>
#include "compressor.h"

Scanner::Scanner()
{
}

void Scanner::scan(ReleaseFileList &r,const QString &directory, const QString &deployPath)
{
    QStack<QString> dirname;
    QDir d(directory);
    if (d.exists())  {
        scanRecursive( deployPath, d, r , dirname);
    }
}

QByteArray Scanner::hashFile(QFile &f)
{
    uchar *map;

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.reset();

    /* Try map first */
    map = f.map( 0, f.size() );
    if (NULL==map) {
        /* no mmap, read in chunks */
        uchar buffer[512];
        qint64 r;
        do {
            r = f.read((char*)buffer,sizeof(buffer));
            if (r<0){
                throw 1;
            }
            if (r<=0)
                break;
            hasher.addData( (const char*)buffer, r);
        } while (r>0);

    } else {
        hasher.addData((const char*)map,f.size());
        f.unmap(map);
    }

    return hasher.result();
}

QString qstackJoin(const QStack<QString> &stack, const QString &delim)
{
    QString result;
    foreach (const QString &entry, stack) {
        if (result.size()!=0) {
            result+=delim;
        }
        result+=entry;
    }
    return result;
}

void Scanner::scanRecursive(const QString &deployPath, QDir &d, ReleaseFileList &r, QStack<QString> &dirname)
{
    QDirIterator it(d);
    static QString pathDelim="/";

    while (it.hasNext()) {
        it.next();

        QFileInfo info = it.fileInfo();
        if (info.isDir()) {
            if (info.fileName()[0]=='.') {
                continue;
            }
            QDir nd(info.filePath());
            dirname.push(info.fileName());
            scanRecursive(deployPath, nd, r,dirname);
            dirname.pop();
        } else {
            QFile file(info.filePath());
            ReleaseFile rf;

            if (file.open(QIODevice::ReadOnly) <0) {
                throw 2;
            }

            rf.sha = hashFile(file);

            

            dirname.push( info.fileName() );
            rf.name = qstackJoin( dirname, pathDelim);
            rf.size = info.size();
            rf.exec = info.isExecutable();
            if(deployPath.size()) {
                deployFile( deployPath, file, rf.sha.toHex());
            }
            r.push_back(rf);
            dirname.pop();
            file.close();
        }
    }
}

void Scanner::deployFile(const QString &deployPath,QFile &file, const QString &sha)
{
    QString target = deployPath + QDir::separator() + "blobs" + QDir::separator() + sha;
    if (QFile(target).exists()) {
        return;
    }
    qDebug()<<"Writing"<<target;
    if (!file.copy(target)) {
        /* errr..... */
        qDebug()<<file.error();
    }
    Compressor *c = new Compressor();
    if (c->compressFile(target) == 0) {
        QFile(target).remove();
    }
    delete(c);
}
