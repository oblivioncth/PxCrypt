#ifndef CANVAS_TRAVERSER_H
#define CANVAS_TRAVERSER_H

// Project Includes
#include "medium_io/traverse/frame_traverser.h"

namespace PxCryptPrivate
{

class CanvasTraverser
{
//-Inner Class------------------------------------------------------------------------------------------------------
public:
    class State;

//-Inner Struct-----------------------------------------------------------------------------------------------------------
private:
    struct Pos
    {
        quint64 px;
        int ch;
        int bit;

        static quint64 channelsBeteween(const Pos& a, const Pos& b);
        static Pos fromBitPos(quint64 bitPos, quint8 bpc);
        quint64 toBitPos(quint8 bpc) const;
        bool operator==(const Pos& other) const = default;
        auto operator<=>(const Pos& other) const noexcept = default;
    };

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    FrameTraverser mFrameTraverser;
    quint8 mBpc;
    int mChBitIdx;
    Pos mEnd;
    std::function<void(void)> mPrePxChange; // Not saved as part of state
    std::function<void(void)> mPostPxChange; // Not saved as part of state

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    CanvasTraverser(const FrameTraverser& frameTraverser, quint8 bpc);
    CanvasTraverser(const State& state);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    Pos currentPos() const;

public:
    // Stat
    State state() const;
    quint8 bpc() const;
    bool atEnd() const;

    quint64 pixelIndex() const;
    Channel channel() const;
    int channelBitIndex() const;

    // Manipulation
    void advanceBits(int bitCount);
    qint64 skip(qint64 bytes);

    // Callback
    void setPrePixelChange(const std::function<void(void)>& ppc); // Callback over signal to avoid use of QObject for something so simple.
    void setPostPixelChange(const std::function<void(void)>& ppc); // Callback over signal to avoid use of QObject for something so simple.

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    bool operator==(const State& state) const;
};

class CanvasTraverser::State
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    FrameTraverser::State mFrameState;
    quint8 mBpc;
    int mChBitIdx;
    Pos mEnd;

//-Constructor-------------------------------------------------------------------------------------------------------------
public:
    State(const FrameTraverser::State& frameState, quint8 bpc, int chBitIdx, Pos end);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    FrameTraverser::State frameState() const;
    quint8 bpc() const;
    int chBitIndex() const;
    Pos end() const;
};

}

#endif // CANVAS_TRAVERSER_H
