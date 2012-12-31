#ifndef KTWSSESSION_P_HPP_
#define KTWSSESSION_P_HPP_

#include <QUuid>
#include <QString>
#include <QDateTime>

namespace Ktws {
class Workspace;

struct SessionImpl {
    Workspace *m_wspace;
    QUuid m_id;
    QString m_name;
    QDateTime m_timestamp;
    QVariantHash m_settings;

    SessionImpl(Workspace *wspace, const QUuid &id)
        : m_wspace(wspace), m_id(id), m_name(), m_timestamp(), m_settings() {}
};
} // namespace Ktws

#endif // KTWSSESSION_P_HPP_
