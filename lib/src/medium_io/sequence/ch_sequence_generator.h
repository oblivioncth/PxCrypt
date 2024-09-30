#ifndef CH_SEQUENCE_GENERATOR_H
#define CH_SEQUENCE_GENERATOR_H

// Standard Library Includes
#include <array>

// Qt Includes
#include <QRandomGenerator>
#include <QVarLengthArray>

// Project Includes
#include "codec/encdec.h"

namespace PxCryptPrivate
{

class ChSequenceGenerator
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using ChannelTracker = QVarLengthArray<Channel, 3>;

//-Inner Class------------------------------------------------------------------------------------------------------
public:
    class State;

//-Class Variables------------------------------------------------------------------------------------------------------
private:
    static constexpr std::array<Channel, 3> ALL_CHANNELS{Channel::Red, Channel::Green, Channel::Blue};

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRandomGenerator mGenerator;
    ChannelTracker mUnusedChannels;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    ChSequenceGenerator(QByteArrayView seed);
    ChSequenceGenerator(const State& state);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void reset();

public:
    bool pixelExhausted() const;
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

//-Constructor-------------------------------------------------------------------------------------------------------------
public:
    State(const QRandomGenerator& rng, const ChannelTracker& channels);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QRandomGenerator rng() const;
    ChannelTracker channels() const;
};

}

#endif // CH_SEQUENCE_GENERATOR_H
