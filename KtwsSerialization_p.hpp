#ifndef KTWSSERIALIZATION_P_HPP_
#define KTWSSERIALIZATION_P_HPP_

#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QList>
#include <QDir>
#include <QDesktopServices>
#include <QSettings>

namespace Ktws {
// Constants (extern decls)
extern const char *DATA_DIR;
extern const char *SETTINGS_FNAME;
extern const char *SESSION_DIR_TMPL;
extern const char *WS_SETTINGS_FNAME_TMPL;

inline QString getBaseDir(const QString &app_id) {
    QDir dir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    dir.mkpath(app_id);
    return dir.absoluteFilePath(app_id);
}

// Global settings
QString getGlobalDataDir(const QString &app_id);
inline QString getGlobalSettingsFile(const QString &app_id) {
    return QString("%1/%2").arg(getBaseDir(app_id), SETTINGS_FNAME);
}

inline QSettings *openGlobalSettings(const QString &app_id) {
    return new QSettings(getGlobalSettingsFile(app_id), QSettings::IniFormat);
}

// Session settings and metadata
QString getSessionDir(const QString &app_id, const QUuid &session_id);
QString getSessionDataDir(const QString &app_id, const QUuid &session_id);
inline QString getSessionSettingsFile(const QString &app_id, const QUuid &session_id) {
    return QString("%1/%2").arg(getSessionDir(app_id, session_id), SETTINGS_FNAME);
}

struct SessionMd {
	QUuid id;
	QString name;
	QDateTime timestamp;

    bool isNull() const { return id.isNull(); }
};
QList<SessionMd> scanSessions(const QString &app_id);

SessionMd readSessionMetadata(const QString &app_id, const QUuid &session_id);
void writeSessionMetadata(const QString &app_id, const SessionMd &md);
void deleteSession(const QString &app_id, const QUuid &session_id);

inline QSettings *openSessionSettings(const QString &app_id, const QUuid &session_id) {
    return new QSettings(getSessionSettingsFile(app_id, session_id), QSettings::IniFormat);
}

// Worksheet settings and metadata
inline QString getWorksheetSettingsFile(const QString &app_id, const QUuid &session_id, const QUuid &ws_id) {
    return QString("%1/%2").arg(getSessionDir(app_id, session_id), WS_SETTINGS_FNAME_TMPL).arg(ws_id.toString());
}

struct WorksheetMd {
	QUuid id;
	QString class_name;
};
QList<WorksheetMd> scanWorksheets(const QString &app_id, const QUuid &session_id);

WorksheetMd readWorksheetMetadata(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id);
void writeWorksheetMetadata(const QString &app_id, const QUuid &session_id, const WorksheetMd &md);
void deleteWorksheet(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id);

inline QSettings *openWorksheetSettings(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id) {
    return new QSettings(getWorksheetSettingsFile(app_id, session_id, worksheet_id), QSettings::IniFormat);
}

// Helpers
void rm_r(const QString &path);
bool cp_r(const QString &src_path, const QString &dest_path);
} // namespace Ktws

#endif // KTWSSERIALIZATION_P_HPP_