// Unit Include
#include "byte_compositer.h"

// Qt Includes
#include <QByteArray>

namespace PxCrypt
{

//===============================================================================================================
// ByteCompositer
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ByteCompositer::ByteCompositer(QByteArray* bufferOut, quint8 chunkSize) :
    mOutput(bufferOut),
    mChunkSize(chunkSize)
{
    Q_ASSERT(mChunkSize <= 7);
    advance();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
bool ByteCompositer::dataInBuffer() { return mBitIdx > 0; }

void ByteCompositer::advance()
{
    mBuffer = 0;
    mBitIdx = 0;
}

//Public:
void ByteCompositer::composite(quint8 chunk)
{
    // Append uninitialized byte after flush
    if(!dataInBuffer())
        mOutput->resize(mOutput->size() + 1);

    int composited = 0;

    while(composited < mChunkSize)
    {
        // Move bits
        int availableBits = mChunkSize - composited;
        int needed = 8 - mBitIdx;
        int compositing = std::min(availableBits, needed);
        quint8 bits = chunk >> composited; // Shift-out already used bits
        mBuffer |= bits << mBitIdx;

        // Update state
        composited += compositing;
        mBitIdx += compositing;

        if(mBitIdx > 7)
        {
            flush();
            advance();
        }
    }
}

void ByteCompositer::flush()
{
    if(dataInBuffer())
        mOutput->back() = mBuffer;
}

}
