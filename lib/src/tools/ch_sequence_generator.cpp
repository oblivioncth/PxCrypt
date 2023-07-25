// Unit Include
#include "ch_sequence_generator.h"

// Qx Includes
#include <qx/core/qx-integrity.h>

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxSequenceGenerator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ChSequenceGenerator::ChSequenceGenerator(QByteArrayView seed) :
    mGenerator(Qx::Integrity::crc32(seed))
{
    Q_ASSERT(!seed.isEmpty());
    reset();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void ChSequenceGenerator::reset()
{
    mUsedChannels[Channel::Red] = false;
    mUsedChannels[Channel::Green] = false;
    mUsedChannels[Channel::Blue] = false;
    mExhausted = false;
}

//Public:
bool ChSequenceGenerator::pixelExhausted() const { return mExhausted; }

void ChSequenceGenerator::exhaust() { mExhausted = true; }

Channel ChSequenceGenerator::next()
{
    // Reset if on new pixels
    if(mExhausted)
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

    if(min == max) // Ignore warning about garbage value
    {
        mExhausted = true;
        // No need to update mUsedChannels, it will be reset on next call
        return static_cast<Channel>(min);
    }

    quint32 ch;
    do
        ch = mGenerator.bounded(min, max + 1);
    while(mUsedChannels[ch]);

    // Update state and return
    mUsedChannels[ch] = true;
    return static_cast<Channel>(ch);
}

/*! @endcond */
}
