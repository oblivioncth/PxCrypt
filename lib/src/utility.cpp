// Unit Includes
#include "utility.h"

// Qt Includes
#include <QLocale>

/*! @cond */
namespace Utility
{

QString dataStr(quint64 bytes)
{
    static QLocale sysLoc = QLocale::system();
    return sysLoc.formattedDataSize(std::min(bytes, static_cast<quint64>(std::numeric_limits<qint64>::max())));
}

}
/*! @endcond */
