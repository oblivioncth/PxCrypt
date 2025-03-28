// Unit Include
#include "px_sequence_generator.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// PxSequenceGenerator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxSequenceGenerator::PxSequenceGenerator(const QSize& dim, const QByteArray& seed) :
    mSeed(seed),
    mPixelTracker(0, (dim.width() * dim.height()) - 1),
    mAtEnd(false)
{
    Q_ASSERT(!seed.isEmpty());
    Q_ASSERT(!dim.isEmpty());

    // Seed generator
    std::seed_seq ss(seed.cbegin(), seed.cend());
    mGenerator.seed(ss);
}

PxSequenceGenerator::PxSequenceGenerator(const State& state) :
    mPixelTracker(state.start(), state.end())
{
    // Seed generator
    std::seed_seq ss(state.seed().cbegin(), state.seed().cend());
    mGenerator.seed(ss);

    // Advance generator to reach state coverage
    while(mPixelTracker.reserved() < state.coverage())
        next();

    // Set at end flag
    mAtEnd = state.atEnd();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
quint64 PxSequenceGenerator::pixelCoverage() const { return mPixelTracker.reserved(); }
quint64 PxSequenceGenerator::pixelTotal() const { return mPixelTracker.range(); }

PxSequenceGenerator::State PxSequenceGenerator::state() const
{
    return State{mSeed, mPixelTracker.minimum(), mPixelTracker.maximum(), mPixelTracker.reserved(), mAtEnd};
}

qint64 PxSequenceGenerator::next()
{
    if(Q_UNLIKELY(atEnd()))
    {
        qWarning("next() called at end!.");
        return -1;
    }

    // Handle going to end case
    if(mPixelTracker.isBooked())
    {
        mAtEnd = true;
        return -1;
    }

    // Determine next index
    quint64 naturalIdx = mGenerator.bounded(mPixelTracker.maximum() + 1);
    std::optional<quint64> actualIdx = mPixelTracker.reserveNearestFree(naturalIdx);
    Q_ASSERT(actualIdx.has_value());

    return static_cast<qint64>(actualIdx.value());
}

bool PxSequenceGenerator::atEnd() const { return mAtEnd; }

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
bool PxSequenceGenerator::operator==(const State& state) const
{
    return mSeed == state.seed() &&
           mPixelTracker.minimum() == state.start() &&
           mPixelTracker.maximum() == state.end() &&
           mPixelTracker.reserved() == state.coverage();
}

//===============================================================================================================
// PxSequenceGenerator::State
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxSequenceGenerator::State::State(const QByteArray& seed, quint64 start, quint64 end, quint64 coverage, bool atEnd) :
    mSeed(seed),
    mStart(start),
    mEnd(end),
    mCoverage(coverage),
    mAtEnd(atEnd)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
QByteArray PxSequenceGenerator::State::seed() const { return mSeed; }
quint64 PxSequenceGenerator::State::start() const { return mStart; }
quint64 PxSequenceGenerator::State::end() const { return mEnd; }
quint64 PxSequenceGenerator::State::coverage() const { return mCoverage; }
bool PxSequenceGenerator::State::atEnd() const { return mAtEnd; }

}
