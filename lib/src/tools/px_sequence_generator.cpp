// Unit Include
#include "px_sequence_generator.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxSequenceGenerator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxSequenceGenerator::PxSequenceGenerator(const QSize& dim, QByteArrayView seed) :
    mPixelTracker(0, (dim.width() * dim.height()) - 1)
{
    Q_ASSERT(!seed.isEmpty());

    // Seed generator
    std::seed_seq ss(seed.cbegin(), seed.cend());
    mGenerator.seed(ss);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
quint64 PxSequenceGenerator::pixelCoverage() const { return mPixelTracker.reserved(); }

int PxSequenceGenerator::next()
{
    // Return if all pixels are accounted for
    if(!hasNext())
    {
        qWarning("next() called when all pixels have already been covered.");
        return -1;
    }

    // Determine next index
    quint64 naturalIdx = mGenerator.bounded(mPixelTracker.maximum() + 1);
    std::optional<quint64> actualIdx = mPixelTracker.reserveNearestFree(naturalIdx);
    Q_ASSERT(actualIdx.has_value());

    return actualIdx.value();
}

bool PxSequenceGenerator::hasNext() const { return !mPixelTracker.isBooked(); }

/*! @endcond */
}
