#ifndef KTWSWORKSHEETHANDLER_HPP_
#define KTWSWORKSHEETHANDLER_HPP_

#include <KtwsGlobal.hpp>

class QString;
class QIcon;
class QUuid;

namespace Ktws {
class Worksheet;
class Workspace

class KTWORKSPACEAPP_EXPORT WorksheetHandler {
    virtual QString classDisplayName(const QString &ws_class) const = 0;
    virtual QString classDescription(const QString &ws_class) const;
        // Default implementation return QString()
    virtual QIcon classIcon(const QString &ws_class) const;
        // Default implementation returns QIcon()
    virtual Worksheet *attach(const QString &ws_class, Workspace *wspace, const QUuid &session_id, const QUuid &wsheet_id) = 0;
};
} // namespace Ktws

#endif // KTWSWORKSHEETHANDLER_HPP_
