#ifndef CANVAS_TRAVERSER_PRIME_H
#define CANVAS_TRAVERSER_PRIME_H

// Project Includes
#include "medium_io/sequence/px_sequence_generator.h"
#include "medium_io/sequence/ch_sequence_generator.h"

namespace PxCryptPrivate
{

class CanvasTraverserPrime
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    std::unique_ptr<PxSequenceGenerator> mPxSequence;
    std::unique_ptr<ChSequenceGenerator> mChSequence;

    qint64 mCurrentIndex;
    Channel mCurrentChannel;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    CanvasTraverserPrime(const QImage& image, const QByteArray& seed);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advancePixel();

public:
    qint64 pixelIndex() const;
    Channel channel() const;
    void nextChannel();

    std::unique_ptr<PxSequenceGenerator> surrenderPxSequence();
    std::unique_ptr<ChSequenceGenerator> surrenderChSequence();
};

}

#endif // CANVAS_TRAVERSER_PRIME_H
