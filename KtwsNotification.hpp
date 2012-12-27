#ifndef KTWSNOTIFICATION_HPP_
#define KTWSNOTIFICATION_HPP_

#include <KtwsGlobal.hpp>

#include <QObject>
class QString;
class QIcon;

namespace Ktws {
class NotificationImpl;
class Notification : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Notification)

    NotificationImpl *d;
   
    Q_ENUMS(Kind)
    Q_PROPERTY(Kind kind READ kind WRITE setKind NOTIFY kindChanged)
    Q_PROPERTY(QString synopsis READ synopsis WRITE setSynopsis NOTIFY synopsisChanged)
    Q_PROPERTY(QString detail READ detail WRITE setDetail NOTIFY detailChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)

public:
    enum Kind {
        Verbose,
        Question,
        Informational,
        Warning,
        Error,
        Critical,
    };
    Notification(Kind kind, const QString &synopsis, QObject *parent = nullptr);
    Notification(QObject *parent = nullptr);

    Kind kind() const;
    void setKind(Kind kind);

    QString synopsis() const;
    void setSynopsis(const QString &synopsis);

    QString detail() const;
    void setDetail(const QString &detail);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    void show(int timeout = -1);
    void showAsToast(int timeout = -1);
    void showAsDialog();
    void execModalDialog();
    void hide();

signals:
    void kindChanged(Kind kind);
    void synopsisChanged(const QString &synopsis);
    void detailChanged(const QString &detail);
    void iconChanged(const QIcon &icon);
};
} // namespace Ktws

#endif // KTWSNOTIFICATION_HPP_
