#include "compressor.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <errno.h>
int Compressor::compressFile(const QString &name, bool force)
{
    const QString final = name + ".gz";
    qint64 r;

    QFile ffile(final);
    if (ffile.exists()) {
        if (force)
            ffile.remove();
        else
            return 0;
    }

    gzFile gzout = gzopen((const char*)final.toStdString().c_str(), "wb9");

    if (gzout==NULL) {
        qDebug()<<"Cannot open"<<final<<"for compressing"<<gzerror(gzout,NULL)<<strerror(errno);
        return -1;
    }

    QFile input(name);
    if (!input.open(QIODevice::ReadOnly)) {
        gzclose(gzout);
        return -1;
    }

    while ((r=input.read((char*)buffer,sizeof(buffer)))>0) {
        if (gzwrite(gzout,buffer,(int)r)<0) {
            gzclose(gzout);
            ffile.remove();
            return -1;
        }
    }
    gzclose(gzout);

    return 0;
}
