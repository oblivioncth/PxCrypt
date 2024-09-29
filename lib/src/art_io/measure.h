#ifndef MEASURE_H
#define MEASURE_H

// Qt Includes
#include <QtGlobal>

// Project Includes
#include "medium_io/canvas.h"

namespace PxCryptPrivate
{

class IMeasure
{
//-Constructor---------------------------------------------------------------------------------------------------------
protected:
    IMeasure();

//-Instance Functions----------------------------------------------------------------------------------------------
protected:
    virtual quint64 renditionSize() const = 0;

public:
    quint64 size() const;
    Canvas::metavalue_t minimumBpc(const QSize& dim) const;
};

template<typename T>
concept measureable = std::derived_from<typename T::Measure, IMeasure>;

}

#endif // MEASURE_H
