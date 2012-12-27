#ifndef KTWSSESSION_P_HPP_
#define KTWSSESSION_P_HPP_

#include "KtwsSession.hpp"
#include "KtwsApplication_p.hpp"

#include <QUuid>
#include <QString>
#include <QDir>

namespace Ktws {
class SessionImpl : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SessionImpl)

    Session *m_cat;
    Workspace *m_wspace;
    QUuid m_id;

public:
    SessionImpl(Workspace *wspace, const QUuid &id, Session *cat)
        : QObject(cat), m_cat(cat),
          m_wspace(wspace),
          m_id(id)
    {}
    virtual ~Session() {}

    QUuid id() const { return m_id; }
    Workspace *wspace() { return m_wspace; }
};
} // namespace Ktws

#endif // KTWSSESSION_P_HPP_
