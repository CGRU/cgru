#include "qaftextwidget.h"

#include <QKeyEvent>
#include <QLayout>
#include <QPushButton>

#define AFOUTPUT
#undef AFOUTPUT
#include "../libafanasy/logger.h"

QAfTextWidget::QAfTextWidget( QWidget * i_parent):
	QWidget( i_parent),
	m_empty( true),
	m_find_opened( false)
{
	QVBoxLayout * layout = new QVBoxLayout( this);

	m_text_edit = new QTextEdit( this);
	layout->addWidget( m_text_edit);

	m_find_wnd = new QFrame( this);
	layout->addWidget( m_find_wnd);
	m_find_wnd->setFrameShape( QFrame::NoFrame);

	QHBoxLayout * find_layout = new QHBoxLayout( m_find_wnd);
	find_layout->setContentsMargins(0,0,0,0);

	QPushButton * btnFind = new QPushButton("Find");
	find_layout->addWidget( btnFind);
	btnFind->setToolTip("Ctrl+F\nF3\nCtrl+G");

	m_find_le = new QLineEdit( this);
	find_layout->addWidget( m_find_le);

	m_find_info = new QLabel( this);
	find_layout->addWidget( m_find_info);

	QPushButton * btnPrev = new QPushButton("Prev");
	find_layout->addWidget( btnPrev);
	btnPrev->setToolTip("Shift+F3\nCtrl+Shift+G");

	QPushButton * btnClose = new QPushButton("Close");
	find_layout->addWidget( btnClose);
	btnClose->setToolTip("Escape");

	connect( m_find_le, SIGNAL( returnPressed()), this, SLOT( findNext()));
	connect( btnFind, SIGNAL( pressed()), this, SLOT( findNext()));
	connect( btnPrev, SIGNAL( pressed()), this, SLOT( findPrevious()));
	connect( btnClose, SIGNAL( pressed()), this, SLOT( findClose()));

	findOpen();
}

QAfTextWidget::~QAfTextWidget()
{
}

void QAfTextWidget::keyPressEvent( QKeyEvent * i_evt)
{
	if( i_evt->matches( QKeySequence::Find))
	{
		findOpen();
		return;
	}

	if( i_evt->key() == Qt::Key_Enter )
	{
		findNext();
		return;
	}

	if( i_evt->matches( QKeySequence::FindNext))
	{
		findSearch( true);
		return;
	}

	if( i_evt->matches( QKeySequence::FindPrevious))
	{
		findSearch( false);
		return;
	}

	if( i_evt->key() == Qt::Key_Escape )
	{
		findClose();
		return;
	}
}

void QAfTextWidget::findOpen()
{
	m_find_le->setFocus();
	m_find_le->selectAll();

	if( m_find_opened )
	{
		if( false == m_find_le->text().isEmpty())
		{
			findNext();
		}

		return;
	}

	m_find_wnd->setHidden( false);

	m_find_opened = true;
}

void QAfTextWidget::findClose()
{
	if( false == m_find_opened )
		return;

	m_find_wnd->setHidden( true);
	m_text_edit->setFocus();

	m_find_opened = false;
}

void QAfTextWidget::findNext() { findSearch( true); }
void QAfTextWidget::findPrevious() { findSearch( false); }
void QAfTextWidget::findSearch( bool i_next)
{
	QTextDocument::FindFlags flags = 0;
	if( false == i_next )
	{
		flags |= QTextDocument::FindBackward;
	}

	if( m_text_edit->find( m_find_le->text(), flags))
	{
		m_find_info->clear();
	}
	else
	{
		if( i_next )
			m_text_edit->moveCursor( QTextCursor::Start);
		else
			m_text_edit->moveCursor( QTextCursor::End);

		if( m_text_edit->find( m_find_le->text(), flags))
			m_find_info->clear();
		else
			m_find_info->setText("Not found.");
	}
}

