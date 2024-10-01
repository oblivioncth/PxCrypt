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
    mUnusedChannels(ALL_CHANNELS.cbegin(), ALL_CHANNELS.cend())
{
    Q_ASSERT(!seed.isEmpty());
}

ChSequenceGenerator::ChSequenceGenerator(const State& state) :
    mGenerator(state.rng()),
    mUnusedChannels(state.channels())
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void ChSequenceGenerator::reset()
{
    mUnusedChannels.append(ALL_CHANNELS.data(), ALL_CHANNELS.size());
}

//Public:
bool ChSequenceGenerator::pixelExhausted() const { return mUnusedChannels.isEmpty(); }

ChSequenceGenerator::State ChSequenceGenerator::state() const
{
    return State{mGenerator, mUnusedChannels};
}

Channel ChSequenceGenerator::next()
{
    // Reset if on new pixel
    if(pixelExhausted())
        reset();

    // Determine next channel
    auto rem = mUnusedChannels.size();
    quint32 idx = rem == 1 ? 0 : mGenerator.bounded(rem);
    Channel ch = mUnusedChannels[idx];

    // Update state and return
    mUnusedChannels.remove(idx);
    return ch;
}

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool ChSequenceGenerator::operator==(const State& state) const
{
    return mGenerator == state.rng() &&
           mUnusedChannels == state.channels();
}

//===============================================================================================================
// ChSequenceGenerator::State
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ChSequenceGenerator::State::State(const QRandomGenerator& rng, const ChannelTracker& channels) :
    mRng(rng),
    mChannels(channels)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QRandomGenerator ChSequenceGenerator::State::rng() const { return mRng; }
ChSequenceGenerator::ChannelTracker ChSequenceGenerator::State::channels() const { return mChannels; }

}
