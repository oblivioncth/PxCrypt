#ifndef CANVAS_TRAVERSER_H
#define CANVAS_TRAVERSER_H

// Project Includes
#include "medium_io/sequence/px_sequence_generator.h"
#include "medium_io/sequence/ch_sequence_generator.h"

namespace PxCryptPrivate
{

class MetaAccess;

class CanvasTraverser
{
//-Inner Struct-----------------------------------------------------------------------------------------------------------
private:
    struct Position
    {
        quint64 px;
        int ch;
        int bit;

        static quint64 channelsBeteween(const Position& a, const Position& b);
        static Position fromBits(quint64 bitPos, quint8 bpc);
        quint64 toBits(quint8 bpc) const;
        bool operator==(const Position& other) const = default;
        auto operator<=>(const Position& other) const noexcept = default;
    };

    struct Selection
    {
        qint64 px;
        Channel ch;
        //int bit; Always the same as linear position bit so just share that

        bool operator==(const Selection& other) const = default;
    };

    struct State
    {
        PxSequenceGenerator::State pxState;
        ChSequenceGenerator::State chState;
        Position linearPosition;
        Selection currentSelection;
    };

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    // Meta
    MetaAccess& mMeta;

    // Generator
    std::unique_ptr<PxSequenceGenerator> mPxSequence;
    std::unique_ptr<ChSequenceGenerator> mChSequence;

    // Location
    Position mLinearPosition;
    Selection mCurrentSelection;
    Position mLinearEnd;

    // State
    std::unique_ptr<State> mInitialState;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    CanvasTraverser(MetaAccess& meta);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void restoreState(const State& state);
    void calculateEnd();
    void advanceChannel();
    void advancePixel();

public:
    void init();

    // Stat
    State state() const;
    bool atEnd() const;

    quint64 pixelIndex() const;
    Channel channel() const;
    int channelBitIndex() const;
    int remainingChannelBits() const;

    // Manipulation
    void advanceBits(int bitCount);
    bool bitAdvanceWillChangePixel(int bitCount);
    qint64 skip(qint64 bytes);

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    bool operator==(const State& state) const;
};

}

#endif // CANVAS_TRAVERSER_H
