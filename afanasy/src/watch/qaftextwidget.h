#pragma once

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

class QAfTextWidget : public QWidget
{
	Q_OBJECT
public:
	QAfTextWidget( QWidget * i_parent);
	~QAfTextWidget();

	inline QTextEdit * qTE() { return m_text_edit; }
	inline void clearText() { m_empty = true; m_text_edit->clear(); }
	inline bool isEmpty() const { return m_empty; }

	inline void setReadOnly( bool i_ro  ) { m_text_edit->setReadOnly( i_ro);  }
	inline void setLineWrapMode( QTextEdit::LineWrapMode i_lwm ) { m_text_edit->setLineWrapMode( i_lwm); }
	inline void setPlainText( const QString & i_str ) { m_text_edit->setPlainText( i_str); m_empty = false; }
	inline void setHtml( const QString & i_str ) { m_text_edit->setHtml( i_str); m_empty = false; }
	inline void append( const QString & i_str ) { m_text_edit->append( i_str); }

public slots:
	void findOpen();
	void findNext();
	void findPrevious();
	void findClose();

protected:
	virtual void keyPressEvent( QKeyEvent * i_evt);

private:
	void findSearch( bool i_next);

private:
	bool m_empty;
	bool m_find_opened;

	QTextEdit * m_text_edit;
	QFrame    * m_find_wnd;
	QLineEdit * m_find_le;
	QLabel    * m_find_info;
};

