// Unit Include
#include "canvas_traverser.h"

// Project Includes
#include "medium_io/operate/meta_access.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// CanvasTraverser
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
CanvasTraverser::CanvasTraverser(MetaAccess& meta) :
    mMeta(meta),
    mLinearPosition{0, 0, 0} // Ignore meta pixels
{
    CanvasTraverserPrime& prime = mMeta.surrenderTraverser();
    mPxSequence = prime.surrenderPxSequence();
    mChSequence = prime.surrenderChSequence();
    mCurrentSelection = Selection{prime.pixelIndex(), prime.channel()};

    mInitialState = std::make_unique<State>(state());
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void CanvasTraverser::restoreState(const State& state)
{
    mPxSequence = std::make_unique<PxSequenceGenerator>(state.pxState);
    mChSequence = std::make_unique<ChSequenceGenerator>(state.chState);
    mLinearPosition = state.linearPosition;
    mCurrentSelection = state.currentSelection;

}
void CanvasTraverser::calculateEnd()
{
    /* Determine end Position
     *
     * We determine the number of available bits, based on the pixel-sequence's remaining
     * pixels (which handles ignoring meta-pixels) in order to determine the last byte boundary
     * correctly (since this depends strictly on the starting Position and BPC).
     */

    quint64 writeablePixels = Qx::length(mPxSequence->pixelCoverage(), mPxSequence->pixelTotal());
    quint64 bitsAvailable = (writeablePixels * 3 * mMeta.bpc());
    bitsAvailable &= ~7; // Same as (x / 8) * 8, only count whole bytes
    mLinearEnd = Position::fromBits(bitsAvailable, mMeta.bpc());
}

void CanvasTraverser::advanceChannel()
{
    Q_ASSERT(!mPxSequence->atEnd()); // Shouldn't happen with current implementation

    if(mChSequence->pixelExhausted())
        advancePixel();
    else
    {
        mLinearPosition.ch++;
        mCurrentSelection.ch = mChSequence->next();
    }
}

void CanvasTraverser::advancePixel()
{
    mLinearPosition.px += 1;
    mCurrentSelection.px = mPxSequence->next();
    mLinearPosition.ch = 0;
    mCurrentSelection.ch = mChSequence->next();
}

//Public:
void CanvasTraverser::init()
{
    if(*this != *mInitialState)
        restoreState(*mInitialState);
    calculateEnd();
}

CanvasTraverser::State CanvasTraverser::state() const
{
    return State{
        mPxSequence->state(),
        mChSequence->state(),
        mLinearPosition,
        mCurrentSelection
    };
}

bool CanvasTraverser::atEnd() const
{
    Q_ASSERT(mLinearPosition <= mLinearEnd); // Debug check to ensure end is never passed
    return mLinearPosition >= mLinearEnd; // Release fallback to check for at or over end
}

quint64 CanvasTraverser::pixelIndex() const { return mCurrentSelection.px; }
Channel CanvasTraverser::channel() const { return mCurrentSelection.ch; }
int CanvasTraverser::channelBitIndex() const { return mLinearPosition.bit; }
int CanvasTraverser::remainingChannelBits() const { return mMeta.bpc() - channelBitIndex(); }

void CanvasTraverser::advanceBits(int bitCount)
 {
    if(Q_UNLIKELY(atEnd()))
    {
        qCritical("Attempted to advance past end!");
        return;
    }

    mLinearPosition.bit += bitCount;
    Q_ASSERT(mLinearPosition.bit <= mMeta.bpc()); // Current design dictates that advancement is capped at the number of bits left on the current channel

    if(mLinearPosition.bit == mMeta.bpc())
    {
        mLinearPosition.bit = 0;
        advanceChannel();
    }

    Q_ASSERT(mLinearPosition <= mLinearEnd); // Never should be possible to go past end
}

bool CanvasTraverser::bitAdvanceWillChangePixel(int bitCount)
{
    return mLinearPosition.bit + bitCount >= mMeta.bpc() && mChSequence->pixelExhausted();
}

qint64 CanvasTraverser::skip(qint64 bytes)
{
    if(atEnd())
        return -1;

    quint64 bitsToSkip = (static_cast<quint64>(bytes) * 8);
    quint64 bitPos = mLinearPosition.toBits(mMeta.bpc());
    Position newPos = Position::fromBits(bitPos + bitsToSkip, mMeta.bpc());
    if(newPos > mLinearEnd)
        newPos = mLinearEnd;

    /* Have to walk through each channel in order for the pixel sequence to be properly generated,
     * but channel bit index can be set directly since we manage it here.
     */
    quint64 chDist = Position::channelsBeteween(mLinearPosition, newPos);
    for(quint64 skip = 0; skip < chDist; skip++)
        advanceChannel();
    mLinearPosition.bit = newPos.bit;

    // Return actual bytes skipped
    return (newPos.toBits(mMeta.bpc()) - bitPos)/8;
}

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool CanvasTraverser::operator==(const State& state) const
{
    return *mPxSequence == state.pxState &&
           *mChSequence == state.chState &&
           mLinearPosition == state.linearPosition &&
           mCurrentSelection == state.currentSelection;
}

//===============================================================================================================
// CanvasTraverser::Position
//===============================================================================================================

quint64 CanvasTraverser::Position::channelsBeteween(const Position& a, const Position& b)
{
    return (b.px - a.px)*3 + (b.ch - a.ch);
}

CanvasTraverser::Position CanvasTraverser::Position::fromBits(quint64 bitPosistion, quint8 bpc)
{
    return {
        .px = bitPosistion / (bpc * 3),
        .ch = static_cast<int>((bitPosistion / bpc) % 3),
        .bit = static_cast<int>(bitPosistion % bpc)
    };
}

quint64 CanvasTraverser::Position::toBits(quint8 bpc) const
{
    return (((px * 3) + ch) * bpc) + bit;
}

}
