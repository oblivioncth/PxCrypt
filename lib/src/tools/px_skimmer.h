#ifndef PX_SKIMMER
#define PX_SKIMMER

// Qt Includes
#include <QImage>

// Project Includes
#include "px_access_read.h"

namespace PxCrypt
{
/*! @cond */

class PxSkimmer
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    PxAccessRead* mSurfaceAccess;
    const QRgb* mRefPixels;

    quint8 mMask;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxSkimmer(PxAccessRead* surfaceAccess, const QImage* medium);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint8 referenceChannelValue();

public:
    bool isAtLimit();
    quint8 next();
};

/*! @endcond */
}

#endif // PX_SKIMMER
