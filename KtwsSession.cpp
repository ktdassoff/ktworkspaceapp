#include "KtwsSession.hpp"
#include "KtwsSession_p.hpp"
#include "KtwsSerialization_p.hpp"

#include <QDir>
#include <QSettings>

namespace Ktws {
Session::Session(const QUuid &id, Workspace *wspace, QObject *parent)
    : QObject(parent), d(new SessionImpl(wspace, id)) {}
Session::~Session() {
    delete d;
}

QUuid Session::id() const { return d->m_id; }

QString Session::name() const { return d->m_name; }
void Session::setName(const QString &name) {
    SessionMd md = { d->m_id, name, d->m_timestamp };
    d->m_name = name;
    writeSessionMetadata(d->m_wspace->appId(), md);
}

QDateTime Session::lastUsedTimestamp() const { return d->m_timestamp; }
void Session::setLastUsedTimestamp(const QDateTime &ts) { d->m_timestamp = ts; }

bool Session::exists() const { return d->m_wspace->sessionById(d->m_id) != nullptr; }
bool Session::isCurrent() const {
    Session *cur = d->m_wspace->currentSession();
    return cur != nullptr && d->m_id == cur->d->m_id;
}
bool Session::isLastUsed() const {
    Session *lu = d->m_wspace->lastUsedSession();
    return lu != nullptr && d->m_id == lu->d->m_id;
}
Session *Session::clone(const QString &new_name) const {
    return d->m_wspace->cpSession(new_name, d->m_id);
}
void Session::remove() { return d->m_wspace->rmSession(d->m_id); }
bool Session::switchTo() { return d->m_wspace->selectSession(d->m_id); }

QSettings *Session::settings() {
    if(!d->m_settings) {
        d->m_settings = openSessionSettings(d->m_wspace->appId(), d->m_id);
        d->m_settings->setParent(this);
    }
    return d->m_settings;
}
QString Session::dataDir() const { return getSessionDataDir(d->m_wspace->appId(), d->m_id); }
} // namespace Ktws