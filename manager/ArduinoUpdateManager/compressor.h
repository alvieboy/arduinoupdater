#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <QString>
#include <zlib.h>

class Compressor
{
public:
    int compressFile(const QString &name,bool force=false);
private:
    unsigned char buffer[8192];
};

#endif
