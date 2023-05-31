#ifndef BYTE_COMPOSITER_H
#define BYTE_COMPOSITER_H

// Qt Includes
#include <QByteArrayView>

namespace pxcrypt
{

class ByteCompositer
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArray* mOutput;
    quint8 mBuffer;

    quint8 mBitIdx;
    quint8 mChunkSize;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    ByteCompositer(QByteArray* bufferOut, quint8 chunkSize);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advance();

public:
    void composite(quint8 chunk);
    void flush();
};

}

#endif // BYTE_COMPOSITER_H
