#include "KtwsWorksheetHandler.hpp"

#include <QString>
#include <QIcon>

namespace Ktws {
QString WorksheetHandler::classDescription(QString ws_class) const {
    return QString();
}
QIcon WorksheetHandler::classIcon(QString ws_class) const {
    return QIcon();
}
} // namespace Ktws
