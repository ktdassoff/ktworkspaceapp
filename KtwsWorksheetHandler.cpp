#include "KtwsWorksheetHandler.hpp"

#include <QString>
#include <QIcon>

namespace Ktws {
QString WorksheetHandler::classDescription(const QString &ws_class) const {
    return QString();
}
QIcon WorksheetHandler::classIcon(const QString &ws_class) const {
    return QIcon();
}
} // namespace Ktws
