#ifndef ENCDEC_H
#define ENCDEC_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>
#include <QSize>

// Qx Includes
#include <qx/utility/qx-macros.h>

namespace pxcrypt
{
//-Namespace Enums-----------------------------------------------------------------------------------------------------
enum EncType : quint8 {
    Relative = 0,
    Absolute = 1
};

}


#endif // ENCDEC_H
