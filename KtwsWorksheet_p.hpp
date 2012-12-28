#ifndef KTWSWORKSHEET_P_HPP_
#define KTWSWORKSHEET_P_HPP_

#include "KtwsGlobal.hpp"

#include <QString>
#include <QUuid>
#include <QVariant>

namespace Ktws {
class Workspace;

struct WorksheetImpl {
    QString m_class_name;
    Workspace *m_wspace;
    QUuid m_session_id;
    QUuid m_wsheet_id;
    bool m_explicit_close;
    QVariantHash m_settings;

    WorksheetImpl(const QString &cn, Workspace *wspace, const QUuid &sid, const QUuid &wid)
    	: m_class_name(cn),
    	  m_wspace(wspace),
    	  m_session_id(sid),
    	  m_wsheet_id(wid),
    	  m_explicit_close(false),
    	  m_settings()
    {}
};
} // namespace Ktws

#endif // KTWSWORKSHEET_P_HPP_
