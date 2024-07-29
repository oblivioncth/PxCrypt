// Unit Include
#include "frame_traverser.h"

// Qx Includes
#include <qx/core/qx-integrity.h>

namespace PxCryptPrivate
{

//===============================================================================================================
// FrameTraverser
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
FrameTraverser::FrameTraverser(const QImage& image, const QByteArray& seed) :
    mPxSequence(image.size(), seed),
    mChSequence(seed),
    mCurrentIndex(mPxSequence.next()),
    mCurrentChannel(mChSequence.next())
{
    Q_ASSERT(!image.isNull());
}

FrameTraverser::FrameTraverser(const State& state) :
    mPxSequence(state.pxState()),
    mChSequence(state.chState()),
    mCurrentIndex(state.index()),
    mCurrentChannel(state.channel())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void FrameTraverser::advancePixel()
{
    mCurrentIndex = mPxSequence.next();
    mCurrentChannel = mChSequence.next();
}

//Public:
FrameTraverser::State FrameTraverser::state() const
{
    return State{mPxSequence.state(), mChSequence.state(), mCurrentIndex, mCurrentChannel};
}

bool FrameTraverser::atEnd() const { return mPxSequence.atEnd(); }

bool FrameTraverser::pixelExhausted() const { return mChSequence.pixelExhausted(); }
quint64 FrameTraverser::pixelStep() const { return (mPxSequence.pixelCoverage() - 1) + mPxSequence.atEnd(); } // Handle end is "1 past"
quint64 FrameTraverser::remainingPixels() const { return mPxSequence.pixelTotal() - mPxSequence.pixelCoverage(); }
quint64 FrameTraverser::totalPixels() const { return mPxSequence.pixelTotal(); }
qint64 FrameTraverser::pixelIndex() const { return mCurrentIndex; }
Channel FrameTraverser::channel() const { return mCurrentChannel; }
int FrameTraverser::channelStep() const { return mChSequence.step(); }

void FrameTraverser::nextChannel()
{
    Q_ASSERT(!atEnd()); // Shouldn't happen with current implementation

    if(mChSequence.pixelExhausted())
        advancePixel();
    else
        mCurrentChannel = mChSequence.next();
}


//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool FrameTraverser::operator==(const State& state) const
{
    return mPxSequence == state.pxState() &&
           mChSequence == state.chState() &&
           mCurrentIndex == state.index() &&
           mCurrentChannel == state.channel();
}

//===============================================================================================================
// FrameTraverser::State
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
FrameTraverser::State::State(const PxSequenceGenerator::State& pxState, const ChSequenceGenerator::State& chState, qint64 index, Channel channel) :
    mPxState(pxState),
    mChState(chState),
    mIndex(index),
    mChannel(channel)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
PxSequenceGenerator::State FrameTraverser::State::pxState() const { return mPxState; }
ChSequenceGenerator::State FrameTraverser::State::chState() const { return mChState; }
qint64 FrameTraverser::State::index() const { return mIndex; }
Channel FrameTraverser::State::channel() const { return mChannel; }

}
