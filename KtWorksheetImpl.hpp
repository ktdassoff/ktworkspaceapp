#ifndef KTWORKSHEETIMPL_HPP
#define KTWORKSHEETIMPL_HPP

#include "KtWorksheet.hpp"

struct KtWorksheetImpl {
    QString title, class_name;
    QUuid id;
    QIcon custom_icon;
    QSettings *wscfg;
};

#endif // !defined(KTWORKSHEETIMPL_HPP)
