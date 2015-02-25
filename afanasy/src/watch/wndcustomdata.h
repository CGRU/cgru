#pragma once

#include "wnd.h"

class QLineEdit;
class QPlainTextEdit;

class WndCustomData : public Wnd
{
Q_OBJECT
public:
	WndCustomData( const QString & i_name, const QString & i_text);
	~WndCustomData();

signals:
	void textEdited( const QString & i_text);

private slots:
	void checkText();
	void setText();

private:
	QPlainTextEdit * m_qText;
	QLineEdit * m_qLine;

	QString m_text;
	bool m_valid;
	QString m_newText;
};
