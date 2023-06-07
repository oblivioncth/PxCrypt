// Unit Include
#include "bit_chunker.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// BitChunker
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
BitChunker::BitChunker(QByteArrayView data, quint8 chunkSize) :
    mDataItr(data.cbegin()),
    mDataEnd(data.cend()),
    mBitIdx(0),
    mChunkSize(chunkSize)
{
    Q_ASSERT(mChunkSize <= 7);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void BitChunker::advance()
{
    mDataItr++;
    mBitIdx = 0;
}

//Public:
quint8 BitChunker::next()
{
    quint8 chunk = 0;
    int chunked = 0;

    while(chunked < mChunkSize && mDataItr != mDataEnd)
    {
        // Move bits
        int availableBits = 8 - mBitIdx;
        int needed = mChunkSize - chunked;
        int chunking = std::min(availableBits, needed);
        quint8 mask = (1 << chunking) - 1;
        quint8 bits = (*mDataItr >> mBitIdx) & mask;
        chunk |= bits << chunked;

        // Update state
        chunked += chunking;
        mBitIdx += chunking;

        if(mBitIdx > 7)
            advance();
    }

    return chunk;
}

bool BitChunker::hasNext() const { return mDataItr != mDataEnd; }

/*! @endcond */
}
