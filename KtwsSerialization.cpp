#include "KtwsSerialization_p.hpp"

#include <QDir>

namespace Ktws {

// Global settings

// Session settings and metadata

// Worksheet settings and metadata

// Helpers
void rm_r(const QString &parent, const QString &dirname) {
     QDir p(parent);
     QDir root = p;
     if(p.exists(dirname)) {
         root.cd(dirname);
         QFileInfoList entries = root.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
         foreach(QFileInfo e, entries) {
             if(e.isDir()) {
                 rm_r(root.absolutePath(), e.fileName());
                 root.rmdir(e.fileName());
             } else root.remove(e.fileName());
         }
         p.rmdir(dirname);
     }
}

void cp_r(const QString &parent, const QString &dirname, const QString &newparent, const QString &newdirname) {
     QDir p(parent);
     QDir np(newparent);
     QDir root = p;
     np.mkdir(newdirname);
     QDir newroot = np;
     newroot.cd(newdirname);
     if(p.exists(dirname)) {
         root.cd(dirname);
         QFileInfoList entries = root.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
         foreach(QFileInfo e, entries) {
             if(e.isDir()) {
                 root.mkdir(e.fileName());
                 cp_r(root.absolutePath(), e.fileName(), newroot.absolutePath(), e.fileName());
             } else QFile::copy(root.filePath(e.fileName()), newroot.filePath(e.fileName()));
         }
     }
}
} // namespace Ktws