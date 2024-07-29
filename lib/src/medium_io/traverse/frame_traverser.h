#ifndef FRAME_TRAVERSER_H
#define FRAME_TRAVERSER_H

// Qt Includes
#include <QRandomGenerator>

// Project Includes
#include "medium_io/sequence/px_sequence_generator.h"
#include "medium_io/sequence/ch_sequence_generator.h"

namespace PxCryptPrivate
{

class FrameTraverser
{
//-Inner Class------------------------------------------------------------------------------------------------------
public:
    class State;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    PxSequenceGenerator mPxSequence;
    ChSequenceGenerator mChSequence;

    qint64 mCurrentIndex;
    Channel mCurrentChannel;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    FrameTraverser(const QImage& image, const QByteArray& seed);
    FrameTraverser(const State& state);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advancePixel();

public:
    // Stat
    State state() const;
    bool atEnd() const;
    bool pixelExhausted() const;

    quint64 pixelStep() const;
    quint64 remainingPixels() const;
    quint64 totalPixels() const;

    qint64 pixelIndex() const;
    Channel channel() const;
    int channelStep() const;

    // Manipulation
    void nextChannel();

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    bool operator==(const State& state) const;
};

class FrameTraverser::State
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    PxSequenceGenerator::State mPxState;
    ChSequenceGenerator::State mChState;
    qint64 mIndex;
    Channel mChannel;

//-Constructor-------------------------------------------------------------------------------------------------------------
public:
    State(const PxSequenceGenerator::State& pxState, const ChSequenceGenerator::State& chState, qint64 index, Channel channel);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    PxSequenceGenerator::State pxState() const;
    ChSequenceGenerator::State chState() const;
    qint64 index() const;
    Channel channel() const;
};

}

#endif // FRAME_TRAVERSER_H
