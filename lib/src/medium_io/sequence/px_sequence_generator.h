#ifndef PX_SEQUENCE_GENERATOR_H
#define PX_SEQUENCE_GENERATOR_H

// Qt Includes
#include <QRandomGenerator>
#include <QSize>
#include <QPoint>

// Qx Includes
#include <qx/core/qx-freeindextracker.h>

namespace PxCryptPrivate
{

class PxSequenceGenerator
{
//-Inner Class------------------------------------------------------------------------------------------------------
public:
    class State;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArray mSeed;
    QRandomGenerator mGenerator;
    Qx::FreeIndexTracker mPixelTracker;
    bool mAtEnd;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxSequenceGenerator(const QSize& dim, const QByteArray& seed);
    PxSequenceGenerator(const State& state);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    quint64 pixelCoverage() const;
    quint64 pixelTotal() const;
    State state() const;

    qint64 next();
    bool atEnd() const;

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    bool operator==(const State& state) const;
};

class PxSequenceGenerator::State
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArray mSeed;
    quint64 mStart;
    quint64 mEnd;
    quint64 mCoverage;
    bool mAtEnd;

//-Constructor-------------------------------------------------------------------------------------------------------------
public:
    State(const QByteArray& seed, quint64 start, quint64 end, quint64 coverage, bool atEnd);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QByteArray seed() const;
    quint64 start() const;
    quint64 end() const;
    quint64 coverage() const;
    bool atEnd() const;
};

}

#endif // PX_SEQUENCE_GENERATOR_H
