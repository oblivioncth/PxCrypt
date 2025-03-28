#ifndef DATA_TRANSLATOR_H
#define DATA_TRANSLATOR_H

// Qt Includes
#include <QRandomGenerator>

// Qx Includes
#include <qx/utility/qx-concepts.h>

// Project Includes
#include "medium_io/operate/px_access.h"

namespace PxCryptPrivate
{
/*! @cond */

class DataTranslator
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    PxAccess& mAccess;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    DataTranslator(PxAccess& access);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    template<typename F>
        requires Qx::defines_call_for_s<F, void, int, int>
    bool translate(F procedure);

    void weaveBits(quint8 bits, int count);
    quint8 skimBits(int count);

public:
    bool weaveByte(quint8 byte);
    bool skimByte(quint8& byte);
};

/*! @endcond */
}

#endif // DATA_TRANSLATOR_H
