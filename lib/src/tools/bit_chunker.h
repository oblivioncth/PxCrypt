#ifndef BIT_CHUNKER_H
#define BIT_CHUNKER_H

// Qt Includes
#include <QByteArrayView>

namespace PxCrypt
{
/*! @cond */

class BitChunker
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArrayView::const_iterator mDataItr;
    QByteArrayView::const_iterator mDataEnd;
    quint8 mBitIdx;
    quint8 mChunkSize;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    BitChunker(QByteArrayView data, quint8 chunkSize);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advance();

public:
    quint8 next();
    bool hasNext() const;
};

/*! @endcond */
}

#endif // BIT_CHUNKER_H
