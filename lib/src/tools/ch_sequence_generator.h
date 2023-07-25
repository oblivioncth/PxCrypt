#ifndef CH_SEQUENCE_GENERATOR_H
#define CH_SEQUENCE_GENERATOR_H

// Qt Includes
#include <QRandomGenerator>

// Project Includes
#include "../encdec.h"

namespace PxCrypt
{
/*! @cond */

class ChSequenceGenerator
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRandomGenerator mGenerator;
    bool mUsedChannels[3];
    bool mExhausted;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    ChSequenceGenerator(QByteArrayView seed);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void reset();

public:
    bool pixelExhausted() const;

    void exhaust();
    Channel next();
};

/*! @endcond */
}

#endif // CH_SEQUENCE_GENERATOR_H
