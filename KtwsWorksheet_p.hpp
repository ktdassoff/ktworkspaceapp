#ifndef KTWSWORKSHEET_P_HPP_
#define KTWSWORKSHEET_P_HPP_

#include "KtwsGlobal.hpp"
#include "KtwsWorksheet.hpp"

#include <QString>
#include <QUuid>
#include <QObject>
#include <QVariant>

namespace Ktws {
class WorksheetImpl : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(WorksheetImpl)

	Worksheet *m_cat;

    QString m_class_name;
    Workspace *m_wspace;
    QUuid m_session_id, m_wsheet_id;
    bool m_explicit_close;

public:
	WorksheetImpl(const QString &cn, Workspace *wspace, const QUuid &sid, const QUuid &wid, Worksheet *cat)
		: QObject(cat), m_cat(cat),
		  m_class_name(cn),
		  m_wspace(wspace),
		  m_session_id(sid),
		  m_wsheet_id(wid)
		  m_explicit_close(false)
	{}
	virtual ~WorksheetImpl() {}

	QString className() const { return m_class_name; }
	Workspace *wspace() { return m_wspace; }
	QUuid id() const { return m_wsheet_id; }
	QUuid sessionId() const { return m_session_id; }
	bool isExplicitClose() const { return m_explicit_close; }
	void setExplicitClose(bool ec = true) { m_explicit_close = ec; }
};
} // namespace Ktws

#endif // KTWSWORKSHEET_P_HPP_
