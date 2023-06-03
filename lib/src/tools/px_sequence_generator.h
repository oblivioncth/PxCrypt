#ifndef PX_SEQUENCE_GENERATOR
#define PX_SEQUENCE_GENERATOR

// Qt Includes
#include <QRandomGenerator>
#include <QSize>
#include <QPoint>

// Qx Includes
#include <qx/core/qx-freeindextracker.h>
#include <qx/utility/qx-macros.h>

namespace PxCrypt
{

class PxSequenceGenerator
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    static inline const QByteArray DEFAULT_SEED = QBAL("The best and most secure seed that is possible to exist!");

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QSize mDimensions;
    QRandomGenerator mGenerator;
    Qx::FreeIndexTracker mPixelTracker;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxSequenceGenerator(const QSize& dim, QStringView seedStr);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    int next();
    bool hasNext() const;
};

}

#endif // PX_SEQUENCE_GENERATOR
