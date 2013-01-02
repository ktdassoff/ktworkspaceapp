#ifndef KTWSSESSION_HPP_
#define KTWSSESSION_HPP_

#include <KtwsGlobal.hpp>
#include "KtwsWorkspace.hpp"

#include <QObject>
#include <QString>
#include <QUuid>
#include <QDateTime>
class QSettings;

namespace Ktws {
class Workspace;

struct SessionImpl;
class KTWORKSPACEAPP_EXPORT Session : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Session)

    SessionImpl *d;
    friend class Workspace;

    Session(const QUuid &id, Workspace *wspace, QObject *parent = nullptr);
    void setLastUsedTimestamp(const QDateTime &ts);

public:
    virtual ~Session();

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    QDateTime lastUsedTimestamp() const;

    bool exists() const;
    bool isPhantom() const { return !exists(); }
    bool isCurrent() const;
    bool isLastUsed() const;

    Session *clone(const QString &new_name) const;
    void remove();
    bool switchTo();

    QSettings *settings();
    QString dataDir() const;
};
} // namespace Ktws

#endif // KTWSSESSION_HPP_
