// Unit Include
#include "px_sequence_generator.h"

namespace PxCrypt
{

//===============================================================================================================
// PxSequenceGenerator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxSequenceGenerator::PxSequenceGenerator(const QSize& dim, QByteArrayView seed) :
    mDimensions(dim),
    mPixelTracker(0, (dim.width() * dim.height()) - 1, {0})
{
    // Seed generator
    bool e = seed.empty();
    std::seed_seq ss(!e ? seed.cbegin() : DEFAULT_SEED.cbegin(), !e ? seed.cend() : DEFAULT_SEED.cend());
    mGenerator.seed(ss);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
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

}
