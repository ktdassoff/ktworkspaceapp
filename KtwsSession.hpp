#ifndef KTWSSESSION_HPP_
#define KTWSSESSION_HPP_

#include <KtwsGlobal.hpp>

#include <QObject>
#include <QString>
#include <QVariant>
class QUuid;
class QDateTime;

namespace Ktws {
class Workspace;

struct SessionImpl;
class KTWORKSPACEAPP_EXPORT Session : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Session)

    SessionImpl *d;

    Session(const QUuid &id, Workspace *wspace, QObject *parent = nullptr);

    Q_PROPERTY(QUuid id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged USER true)
    Q_PROPERTY(QDateTime lastUsedTimestamp READ lastUsedTimestamp)
    Q_PROPERTY(bool exists READ exists STORED false)
    Q_PROPERTY(bool current READ isCurrent NOTIFY currentChanged STORED false)
    Q_PROPERTY(bool lastUsed READ isLastUsed STORED false)

public:
    virtual ~Session();

    QUuid id() const;

    QString name() const;
    bool setName(QString name);

    QDateTime lastUsedTimestamp() const;

    bool exists() const;
    bool isPhantom() const { return !exists(); }
    bool isCurrent() const;
    bool isLastUsed() const;

    SessionId clone(QString new_name);
    bool remove();
    bool switchTo();

    QVariantHash &settings();
    const QVariantHash &settings() const;
    void replaceSettings(const QVariantHash &settings);
    QString dataDir() const;

    static bool isValidSessionName(QString name);

signals:
    void nameChanged(QString name);
    void currentChanged(bool isCurrent);
    void cloned(QString new_name);
    void removed();
};
} // namespace Ktws

#endif // KTWSSESSION_HPP_
