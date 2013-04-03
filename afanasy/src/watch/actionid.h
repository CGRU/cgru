#pragma once

#include "../libafqt/qenvironment.h"

#include <QtGui/QAction>

class ActionId : public QAction
{

Q_OBJECT

public:
	ActionId( int i_id, const QString & text, QObject * parent );

signals:
	void triggeredId( int);

private slots:
	void triggeredId_Slot();

private:
	int m_id;
};

class ActionIdId : public QAction
{

Q_OBJECT

public:
	ActionIdId ( int i_id_1, int i_id_2, const QString & text, QObject * parent );

signals:
	void triggeredId( int, int);

private slots:
	void triggeredId_Slot();

private:
	int m_id_1;
	int m_id_2;
};

class ActionIdIdId : public QAction
{

Q_OBJECT

public:
	ActionIdIdId ( int i_id_1, int i_id_2, int i_id_3, const QString & text, QObject * parent );

signals:
	void triggeredId( int, int, int);

private slots:
	void triggeredId_Slot();

private:
	int m_id_1;
	int m_id_2;
	int m_id_3;
};

class ActionString : public QAction
{

Q_OBJECT

public:
	ActionString( const QString & i_string, const QString & i_text, QObject * i_parent );

signals:
	void triggeredString( QString );

private slots:
	void triggeredString_Slot();

private:
	QString m_string;
};

class ActionIdString : public QAction
{

Q_OBJECT

public:
	ActionIdString( int i_id, const QString & i_string, const QString & i_text, QObject * i_parent );

signals:
	void triggeredId( int, QString);

private slots:
	void triggeredId_Slot();

private:
	int m_id;
	QString m_string;
};

