#include "KtwsSerialization_p.hpp"

#include <QDir>

namespace Ktws {

// Global settings

// Session settings and metadata

// Worksheet settings and metadata

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