// Unit Include
#include "canvas_traverser.h"

// Project Includes
#include "medium_io/frame.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// CanvasTraverser
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
CanvasTraverser::CanvasTraverser(const FrameTraverser& frameTraverser, quint8 bpc) :
    mFrameTraverser(frameTraverser),
    mBpc(bpc),
    mChBitIdx(0)
{
    // Should be created at first non-meta-pixel, start of byte boundary, much of the implementation relies on this
    Q_ASSERT(mFrameTraverser.pixelStep() == Frame::metalPixelCount() && mFrameTraverser.channelStep() == 0);
    //TODO: See if frame can just be merged into canvas, or if not, if canvas traverser can properly cut off the
    // first two pixels consumed by frametraverser so that here we dont even need to account for the meta pixels

    /* Determine end position
     *
     * We determine the number of available bits, starting from the first non-meta pixels
     * in order to determine the last byte boundary correctly (since this depends strictly
     * on the starting pos and BPC), but then add two to the pixel portion of the pos to
     * account for the meta pixels that were skipped
     */
    uint writeablePixels = mFrameTraverser.remainingPixels() + 1; // +1 to include current
    quint64 bitsAvailable = (writeablePixels * 3 * mBpc);
    bitsAvailable &= ~7; // Same as (x / 8) * 8, only count whole bytes
    mEnd = Pos::fromBitPos(bitsAvailable, mBpc);
    mEnd.px += Frame::metalPixelCount();
}

CanvasTraverser::CanvasTraverser(const State& state) :
    mFrameTraverser(state.frameState()),
    mBpc(state.bpc()),
    mChBitIdx(state.chBitIndex()),
    mEnd(state.end())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
CanvasTraverser::Pos CanvasTraverser::currentPos() const
{
    return {.px = mFrameTraverser.pixelStep(), .ch = mFrameTraverser.channelStep(), .bit = mChBitIdx};
}

//Public:
CanvasTraverser::State CanvasTraverser::state() const
{
    return State{mFrameTraverser.state(), mBpc, mChBitIdx, mEnd};
}

quint8 CanvasTraverser::bpc() const { return mBpc; }

bool CanvasTraverser::atEnd() const
{
    Pos cp = currentPos();
    Q_ASSERT(currentPos() <= mEnd);
    bool reached = cp >= mEnd;
    Q_ASSERT(reached || !mFrameTraverser.atEnd());
    return reached;
}

quint64 CanvasTraverser::pixelIndex() const { return mFrameTraverser.pixelIndex(); }
Channel CanvasTraverser::channel() const { return mFrameTraverser.channel(); }
int CanvasTraverser::channelBitIndex() const { return mChBitIdx; }

void CanvasTraverser::advanceBits(int bitCount)
{
    if(Q_UNLIKELY(atEnd()))
    {
        qCritical("Attempted to advance past end!");
        return;
    }

    mChBitIdx+= bitCount;
    Q_ASSERT(mChBitIdx <= mBpc); // Current design dictates that advancement is capped at the number of bits left on the current channel

    if(mChBitIdx == mBpc)
    {
        mChBitIdx = 0;
        bool changingPx = mFrameTraverser.pixelExhausted();
        if(changingPx && mPrePxChange)
            mPrePxChange();
        mFrameTraverser.nextChannel();
        if(changingPx && !atEnd() && mPostPxChange)
            mPostPxChange();
    }

    Q_ASSERT(currentPos() <= mEnd); // Never should be possible to go past end
}

qint64 CanvasTraverser::skip(qint64 bytes)
{
    if(atEnd())
        return -1;

    quint64 bitsToSkip = (static_cast<quint64>(bytes) * 8);
    Pos pos = currentPos();
    quint64 bitPos = pos.toBitPos(mBpc);
    Pos newPos = Pos::fromBitPos(bitPos + bitsToSkip, mBpc);
    if(newPos > mEnd)
        newPos = mEnd;

    /* Have to walk through each channel in order for the pixel sequence to be properly generated,
     * but channel bit index can be set directly since we manage it here.
     */
    quint64 chDist = Pos::channelsBeteween(pos, newPos);
    for(quint64 skip = 0; skip < chDist; skip++)
        mFrameTraverser.nextChannel();
    mChBitIdx = newPos.bit;

    // Return actual bytes skipped
    return (newPos.toBitPos(mBpc) - bitPos)/8;
}

void CanvasTraverser::setPrePixelChange(const std::function<void(void)>& ppc) { mPrePxChange = ppc; }
void CanvasTraverser::setPostPixelChange(const std::function<void(void)>& ppc) { mPostPxChange = ppc; }

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool CanvasTraverser::operator==(const State& state) const
{
    return mFrameTraverser == state.frameState() &&
           mBpc == state.bpc() &&
           mChBitIdx == state.chBitIndex() &&
           mEnd == state.end();
}

//===============================================================================================================
// CanvasTraverser::Pos
//===============================================================================================================

quint64 CanvasTraverser::Pos::channelsBeteween(const Pos& a, const Pos& b)
{
    return (b.px - a.px)*3 + (b.ch - a.ch);
}

CanvasTraverser::Pos CanvasTraverser::Pos::fromBitPos(quint64 bitPos, quint8 bpc)
{
    return {
        .px = bitPos / (bpc * 3),
        .ch = static_cast<int>((bitPos / bpc) % 3),
        .bit = static_cast<int>(bitPos % bpc)
    };
}

quint64 CanvasTraverser::Pos::toBitPos(quint8 bpc) const
{
    return (((px * 3) + ch) * bpc) + bit;
}

//===============================================================================================================
// CanvasTraverser::State
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
CanvasTraverser::State::State(const FrameTraverser::State& frameState, quint8 bpc, int chBitIdx, Pos end) :
    mFrameState(frameState),
    mBpc(bpc),
    mChBitIdx(chBitIdx),
    mEnd(end)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
FrameTraverser::State CanvasTraverser::State::frameState() const { return mFrameState; }
quint8 CanvasTraverser::State::bpc() const { return mBpc; }
int CanvasTraverser::State::chBitIndex() const { return mChBitIdx; }
CanvasTraverser::Pos CanvasTraverser::State::end() const { return mEnd; }

}
