#ifndef BYTE_COMPOSITER_H
#define BYTE_COMPOSITER_H

// Qt Includes
#include <QByteArrayView>

namespace PxCrypt
{
/*! @cond */

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
    /* Flush doesn't need to be public, which would complicate the design of this class since encoded data is only ever
     * composed of complete bytes. If PxSkimmer runs out of data while this is in the middle of a byte then there
     * is a different problem.
     */
    void flush();
    void advance();

public:
    void composite(quint8 chunk);
};

/*! @endcond */
}

#endif // BYTE_COMPOSITER_H
