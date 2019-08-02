#pragma once

#include "../libafqt/name_afqt.h"

#include <QWidget>

class QFrame;
class QLabel;
class QVBoxLayout;

class Item;

class ParamsPanel : public QWidget
{
Q_OBJECT
public:
	ParamsPanel();
	virtual ~ParamsPanel();

	virtual void v_updatePanel(Item * i_item = NULL);

protected:
	QVBoxLayout * m_panel_layout;

private:
	QLabel * m_name;

	QFrame      * m_info_frame;
	QVBoxLayout * m_info_layout;
	QLabel      * m_info_label;
	QLabel      * m_info_text;
};

