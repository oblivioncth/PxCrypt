// Unit Include
#include "ch_sequence_generator.h"

// Qx Includes
#include <qx/core/qx-integrity.h>

namespace PxCryptPrivate
{

//===============================================================================================================
// ChSequenceGenerator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ChSequenceGenerator::ChSequenceGenerator(QByteArrayView seed) :
    mGenerator(Qx::Integrity::crc32(seed)),
    mStep(-1)
{
    Q_ASSERT(!seed.isEmpty());
    reset();
}

ChSequenceGenerator::ChSequenceGenerator(const State& state) :
    mGenerator(state.rng()),
    mUsedChannels(state.channels()),
    mStep(state.step())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void ChSequenceGenerator::reset()
{
    mUsedChannels.fill(false);
    mStep = -1; // Before 1st step, pushed to 0 by next
}

//Public:
bool ChSequenceGenerator::pixelExhausted() const { return mStep == 2; }
int ChSequenceGenerator::step() const { return mStep; }

ChSequenceGenerator::State ChSequenceGenerator::state() const
{
    return State{mGenerator, mUsedChannels, mStep};
}

Channel ChSequenceGenerator::next()
{
    // Reset if on new pixels
    if(pixelExhausted())
        reset();

    // Determine next channel
    quint32 min;
    for(uint i = Channel::Red; i <= Channel::Blue; i++)
    {
        if(!mUsedChannels[i])
        {
            min = i;
            break;
        }
    }

    quint32 max;
    for(uint i = Channel::Blue; i >= Channel::Red; i--)
    {
        if(!mUsedChannels[i])
        {
            max = i;
            break;
        }
    }

    quint32 ch;
    if(min == max) // Ignore warning about garbage value
        ch = min;
    else
    {
        do
            ch = mGenerator.bounded(min, max + 1);
        while(mUsedChannels[ch]);
    }

    // Update state and return
    mUsedChannels[ch] = true;
    mStep++;
    return static_cast<Channel>(ch);
}

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool ChSequenceGenerator::operator==(const State& state) const
{
    return mGenerator == state.rng() &&
           mUsedChannels == state.channels();
}

//===============================================================================================================
// ChSequenceGenerator::State
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ChSequenceGenerator::State::State(const QRandomGenerator& rng, const ChannelTracker& channels, int step) :
    mRng(rng),
    mChannels(channels),
    mStep(step)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QRandomGenerator ChSequenceGenerator::State::rng() const { return mRng; }
ChSequenceGenerator::ChannelTracker ChSequenceGenerator::State::channels() const { return mChannels; }
int ChSequenceGenerator::State::step() const { return mStep; }

}
