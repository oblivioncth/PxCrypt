// Unit Include
#include "canvas_traverser_prime.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// CanvasTraverserPrime
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
CanvasTraverserPrime::CanvasTraverserPrime(const QImage& image, const QByteArray& seed) :
    mPxSequence(std::make_unique<PxSequenceGenerator>(image.size(), seed)),
    mChSequence(std::make_unique<ChSequenceGenerator>(seed))
{
    Q_ASSERT(!image.isNull());
    mCurrentIndex = mPxSequence->next();
    mCurrentChannel = mChSequence->next();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void CanvasTraverserPrime::advancePixel()
{
    mCurrentIndex = mPxSequence->next();
    mCurrentChannel = mChSequence->next();
}

//Public:
qint64 CanvasTraverserPrime::pixelIndex() const { return mCurrentIndex; }
Channel CanvasTraverserPrime::channel() const { return mCurrentChannel; }

void CanvasTraverserPrime::nextChannel()
{
    Q_ASSERT(!mPxSequence->atEnd()); // Shouldn't happen with current implementation

    if(mChSequence->pixelExhausted())
        advancePixel();
    else
        mCurrentChannel = mChSequence->next();
}

std::unique_ptr<PxSequenceGenerator> CanvasTraverserPrime::surrenderPxSequence() { return std::move(mPxSequence); }
std::unique_ptr<ChSequenceGenerator> CanvasTraverserPrime::surrenderChSequence() { return std::move(mChSequence); }

}
