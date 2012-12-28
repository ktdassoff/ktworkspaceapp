#include "KtwsSession.hpp"
#include "KtwsSession_p.hpp"

#include <QDir>

namespace Ktws {
QUuid Session::id() const { return d->m_id; }

QString Session::name() const { return d->m_name; }
bool Session::setName(const QString &name) { return d->m_wspace->renameSession(name, d->m_id); }

QDateTime Session::lastUsedTimestamp() const { return d->m_timestamp; }

bool Session::exists() const { return d->m_wspace->existsSession(d->m_id); }
bool Session::isCurrent() const { return d->m_id == d->m_wspace->currentSessionId(); }
bool Session::isLastUsed() const { return d->m_id == d->m_wspace->lastUsedSessionId(); }
Session *Session::clone(const QString &new_name) const {
    QUuid nid = d->m_wspace->mkSession(new_name, d->m_id);
    if(!nid.isNull()) return d->m_wspace->sessionById(nid);
    else return nullptr;
}
bool Session::remove() { return d->m_wspace->rmSession(d->m_id); }
bool Session::switchTo() { return d->m_wspace->selectSession(d->m_id); }

QVariantHash &Session::settings() { return d->m_settings; }
const QVariantHash &Session::settings() const { return d->m_settings; }
void Session::replaceSettings(const QVariantHash &settings) { d->m_settings = settings; }
QString Session::dataDir() const {
    QDir bd(d->m_wspace->baseDataDir());
    QString idstr = d->m_id.toString();
    if(!bd.exists(idstr) && !bd.mkdir(idstr)) return QString();
    else return QDir::cleanPath(bd.absoluteFilePath(d->m_id.toString()));
}
} // namespace Ktws