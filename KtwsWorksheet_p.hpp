#ifndef KTWSWORKSHEET_P_HPP_
#define KTWSWORKSHEET_P_HPP_

#include "KtwsGlobal.hpp"

#include <QString>
#include <QUuid>
class QSettings;

namespace Ktws {
class Workspace;
class Session;

struct WorksheetImpl {
    QString m_class_name;
    Workspace *m_wspace;
    Session *m_session;
    QUuid m_wsheet_id;
    bool m_explicit_close;
    QSettings *m_settings;

    WorksheetImpl(const QString &cn, Workspace *wspace, Session *session, const QUuid &wid)
    	: m_class_name(cn),
    	  m_wspace(wspace),
    	  m_session(session),
    	  m_wsheet_id(wid),
    	  m_explicit_close(false),
    	  m_settings(nullptr)
    {}
};
} // namespace Ktws

#endif // KTWSWORKSHEET_P_HPP_
