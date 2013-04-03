#include "actionid.h"

ActionId::ActionId( int i_id, const QString & text, QObject * parent ):
	QAction( text, parent),
	m_id(i_id)
{
	connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}
void ActionId::triggeredId_Slot()
{
	emit triggeredId( m_id);
}


ActionIdId::ActionIdId( int i_id_1, int i_id_2, const QString & text, QObject * parent ):
	QAction( text, parent),
	m_id_1(i_id_1),
	m_id_2(i_id_2)
{
	connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}
void ActionIdId::triggeredId_Slot()
{
	emit triggeredId( m_id_1, m_id_2);
}

ActionIdIdId::ActionIdIdId( int i_id_1, int i_id_2, int i_id_3, const QString & text, QObject * parent ):
	QAction( text, parent),
	m_id_1(i_id_1),
	m_id_2(i_id_2),
	m_id_3(i_id_3)
{
	connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}
void ActionIdIdId::triggeredId_Slot()
{
	emit triggeredId( m_id_1, m_id_2, m_id_3);
}


ActionString::ActionString( const QString & i_string, const QString & i_text, QObject * i_parent ):
	QAction( i_text, i_parent),
	m_string(i_string)
{
	connect( this, SIGNAL(triggered()), this, SLOT( triggeredString_Slot() ));
}
void ActionString::triggeredString_Slot()
{
	emit triggeredString( m_string);
}

ActionIdString::ActionIdString( int i_id, const QString & i_string, const QString & i_text, QObject * i_parent ):
	QAction( i_text, i_parent),
	m_id( i_id),
	m_string( i_string)
{
	connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}
void ActionIdString::triggeredId_Slot()
{
	emit triggeredId( m_id, m_string);
}

