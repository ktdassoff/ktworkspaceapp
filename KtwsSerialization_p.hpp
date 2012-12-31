#ifndef KTWSSERIALIZATION_P_HPP_
#define KTWSSERIALIZATION_P_HPP_

#include <QVariant>
#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QList>

namespace Ktws {
QString getBaseDir(const QString &app_id);

// Global settings
QString getGlobalDataDir(const QString &app_id);
bool readGlobalSettings(const QString &app_id, QVariantHash &target);
bool writeGlobalSettings(const QString &app_id, const QVariantHash &source);

// Session settings and metadata
struct SessionMd {
	QUuid id;
	QString name;
	QDateTime timestamp;
};
QList<SessionMd> scanSessions(const QString &app_id);
QString getSessionDataDir(const QString &app_id, const QUuid &session_id);
QString getSessionSettingsDir(const QString &app_id, const QUuid &session_id);

bool readSessionMetadata(const QString &app_id, const QUuid &session_id, SessionMd &target);
bool writeSessionMetadata(const QString &app_id, const SessionMd &source);
bool deleteSession(const QString &app_id, const QUuid &session_id);
bool readSessionSettings(const QString &app_id, const QUuid &session_id, QVariantHash &target);
bool writeSessionSettings(const QString &app_id, const QUuid &session_id, const QVariantHash &source);

// Worksheet settings and metadata
struct WorksheetMd {
	QUuid id;
	QString class_name;
};
QList<WorksheetMd> scanWorksheets(const QString &app_id, const QUuid &session_id);
bool updateWorksheets(const QString &app_id, const QUuid &session_id, const QList<WorksheetMd> &worksheets);

bool readWorksheetMetadata(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id, WorksheetMd &target);
bool writeWorksheetMetadata(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id, const WorksheetMd &source);
bool deleteWorksheet(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id);
bool readWorksheetSettings(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id, QVariantHash &target);
bool writeWorksheetSettings(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id, const QVariantHash &source);

// Helpers
void rm_r(const QString &path);
void cp_r(const QString &src_path, const QString &dest_path);
} // namespace Ktws

#endif // KTWSSERIALIZATION_P_HPP_