#include "KtwsSerialization_p.hpp"

#include <QFile>

namespace Ktws {
// Constants
const char *DATA_DIR = "data";
const char *SETTINGS_FNAME = "settings.ini";
const char *SESSION_DIR_TMPL = "session_%1";
const char *WS_SETTINGS_FNAME_TMPL = "ws_%1.ini";
const char *SG_SESSIONS = "KtwsSessions";
const char *SG_WORKSHEETS = "KtwsWorksheets";

// Global settings
QString getGlobalDataDir(const QString &app_id) {
    QDir dir(getBaseDir(app_id));
    dir.mkpath(DATA_DIR);
    return dir.absoluteFilePath(DATA_DIR);
}

// Session settings and metadata
QString getSessionDir(const QString &app_id, const QUuid &session_id) {
    QDir dir(getBaseDir(app_id));
    QString sdd = QString(SESSION_DIR_TMPL).arg(session_id.toString());
    dir.mkpath(sdd);
    return dir.absoluteFilePath(sdd);
}
QString getSessionDataDir(const QString &app_id, const QUuid &session_id) {
    QDir dir(getSessionDir(app_id, session_id));
    dir.mkpath(DATA_DIR);
    return dir.absoluteFilePath(DATA_DIR);
}

QList<SessionMd> scanSessions(const QString &app_id) {
    QSettings mcfg(getGlobalSettingsFile(app_id), QSettings::IniFormat);
    QList<SessionMd> lmd;
    mcfg.beginGroup(SG_SESSIONS);
    QStringList sg = mcfg.childGroups();
    foreach(const QString &s, sg) lmd.append(readSessionMetadata(app_id, QUuid(s)));
    mcfg.endGroup();
    return lmd;
}
SessionMd readSessionMetadata(const QString &app_id, const QUuid &session_id) {
    QSettings mcfg(getGlobalSettingsFile(app_id), QSettings::IniFormat);
    SessionMd smd;
    mcfg.beginGroup(SG_SESSIONS);
    if(mcfg.contains(session_id.toString())) {
        mcfg.beginGroup(session_id.toString());
        smd.id = session_id;
        smd.name = mcfg.value("name").toString();
        smd.timestamp = mcfg.value("timestamp").toDateTime();
        mcfg.endGroup();
    }
    mcfg.endGroup();
    return smd;
}
void writeSessionMetadata(const QString &app_id, const SessionMd &md) {
    QSettings mcfg(getGlobalSettingsFile(app_id), QSettings::IniFormat);
    mcfg.beginGroup(SG_SESSIONS);
    mcfg.beginGroup(md.id.toString());
    mcfg.setValue("name", md.name);
    mcfg.setValue("timestamp", md.timestamp);
    mcfg.endGroup();
    mcfg.endGroup();
}
void deleteSession(const QString &app_id, const QUuid &session_id) {
    // Remove session directory
    rm_r(getSessionDir(app_id, session_id));

    // Remove session metadata
    QSettings mcfg(getGlobalSettingsFile(app_id), QSettings::IniFormat);
    mcfg.beginGroup(SG_SESSIONS);
    mcfg.remove(session_id.toString());
    mcfg.endGroup();
}

// Worksheet settings and metadata
QList<WorksheetMd> scanWorksheets(const QString &app_id, const QUuid &session_id) {
    QSettings mcfg(getSessionSettingsFile(app_id, session_id));
    QList<WorksheetMd> lmd;
    mcfg.beginGroup(SG_WORKSHEETS);
    QStringList sg = mcfg.childGroups();
    foreach(const QString &s, sg) lmd.append(readWorksheetMetadata(app_id, session_id, QUuid(s)));
    mcfg.endGroup();
    return lmd;
}

WorksheetMd readWorksheetMetadata(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id) {
    QSettings mcfg(getSessionSettingsFile(app_id, session_id));
    WorksheetMd wmd;
    mcfg.beginGroup(SG_WORKSHEETS);
    if(mcfg.contains(worksheet_id.toString())) {
        mcfg.beginGroup(worksheet_id.toString());
        wmd.id = worksheet_id;
        wmd.class_name = mcfg.value("class_name").toString();
        mcfg.endGroup();
    }
    mcfg.endGroup();
    return wmd;
}
void writeWorksheetMetadata(const QString &app_id, const QUuid &session_id, const WorksheetMd &md) {
    QSettings mcfg(getSessionSettingsFile(app_id, session_id));
    mcfg.beginGroup(SG_WORKSHEETS);
    mcfg.beginGroup(md.id.toString());
    mcfg.setValue("class_name", md.class_name);
    mcfg.endGroup();
    mcfg.endGroup();
}
void deleteWorksheet(const QString &app_id, const QUuid &session_id, const QUuid &worksheet_id) {
    // Remove worksheet settings file
    QFile::remove(getWorksheetSettingsFile(app_id, session_id, worksheet_id));

    // Remove worksheet metadata
    QSettings mcfg(getSessionSettingsFile(app_id, session_id));
    mcfg.beginGroup(SG_WORKSHEETS);
    mcfg.remove(worksheet_id.toString());
    mcfg.endGroup();
}

// Helpers
void rm_r_helper(QDir p) {
    QFileInfoList entries = p.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo e, entries) {
        if(e.isDir()) {
            rm_r_helper(e.absoluteDir());
            p.rmdir(e.fileName());
        } else p.remove(e.fileName());
    }
}
void rm_r(const QString &path) {
    QDir p(path);
    if(p.exists()) {
        rm_r_helper(p);
        QString dn = p.dirName();
        p.cdUp();
        p.rmdir(dn);
    }
}

bool cp_r(const QString &src_path, const QString &dest_path) {
    QDir s(src_path);
    QDir d(dest_path);
    QDir dp = d;
    dp.cdUp();

    if(!s.exists() || !(d.exists() || d.mkpath("."))) return false;

    QFileInfoList entries = s.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo e, entries) {
        if(e.isDir()) cp_r(e.absoluteFilePath(), d.absoluteFilePath(e.fileName()));
        else QFile::copy(e.absoluteFilePath(), d.absoluteFilePath(e.fileName()));
    }

    return true;
}
} // namespace Ktws