#ifndef CH_SEQUENCE_GENERATOR_H
#define CH_SEQUENCE_GENERATOR_H

// Standard Library Includes
#include <array>

// Qt Includes
#include <QRandomGenerator>

// Project Includes
#include "codec/encdec.h"

namespace PxCryptPrivate
{

class ChSequenceGenerator
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using ChannelTracker = std::array<bool, CH_COUNT>;

//-Inner Class------------------------------------------------------------------------------------------------------
public:
    class State;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRandomGenerator mGenerator;
    ChannelTracker mUsedChannels; // Wastes 1 element slot, but keeps indexing clean
    int mStep;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    ChSequenceGenerator(QByteArrayView seed);
    ChSequenceGenerator(const State& state);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void reset();

public:
    bool pixelExhausted() const;
    int step() const;
    State state() const;

    Channel next();

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    bool operator==(const State& state) const;
};

class ChSequenceGenerator::State
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRandomGenerator mRng;
    ChannelTracker mChannels;
    int mStep;

//-Constructor-------------------------------------------------------------------------------------------------------------
public:
    State(const QRandomGenerator& rng, const ChannelTracker& channels, int step);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QRandomGenerator rng() const;
    ChannelTracker channels() const;
    int step() const;
};

}

#endif // CH_SEQUENCE_GENERATOR_H
