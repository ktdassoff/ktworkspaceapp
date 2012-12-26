#ifndef QTWORKSHEET_HPP
#define QTWORKSHEET_HPP

#include <ktworkspaceapp_export.h>
#include "KtWorkspaceApp.hpp"

#include <QMainWindow>
#include <QIcon>
#include <QUuid>

struct KtWorksheetImpl;
class KTWORKSPACEAPP_EXPORT KtWorksheet : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(KtWorksheet)
    KtWorksheetImpl * self;
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className CONSTANT)
    Q_PROPERTY(QUuid id READ id CONSTANT)
    Q_PROPERTY(QIcon customIcon READ customIcon WRITE setCustomIcon RESET unsetCustomIcon)
    bool m_explicit_close;

public:
    KtWorksheet(const QString & class_name, const QUuid & id = QUuid(), const QString & title = QString(), const QIcon & custom_icon = QIcon(), QWidget * parent = 0, KtWorkspaceApp * app = KtWorkspaceApp::instance());
    virtual ~KtWorksheet();

    QString title() const;
    void setTitle(const QString & val);

    QString className() const;
    QUuid id() const;
    
    QIcon customIcon() const;
    void setCustomIcon(const QIcon & val);
    void unsetCustomIcon();

public slots:
    bool explicitClose();

signals:
    void titleChanged(const QString &new_title);

protected:
    KtWorkspaceApp * const wsApp;
    QSettings * getWorksheetSettings() const;

    virtual void closeEvent(QCloseEvent * event);
    enum CloseType {
        GeneralClose,
        ExplicitClose,
        SessionEndClose
    };
    virtual bool confirmClose(CloseType close_type);
};

#endif // !defined(KTWORKSHEET_HPP)
