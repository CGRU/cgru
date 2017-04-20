#include "checkboxwidget.h"

#include "../libafqt/attrnumber.h"

#include "watch.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>

CheckBoxWidget::CheckBoxWidget( QWidget * parent, afqt::AttrNumber * attrNumber):
	QWidget( parent),
	num( attrNumber)
{
	QHBoxLayout * layout = new QHBoxLayout(this);

	QLabel * label = new QLabel( attrNumber->getLabel(), this);

	QCheckBox * checkbox = new QCheckBox( this);
	checkbox->setCheckState( attrNumber->n ? Qt::Checked : Qt::Unchecked);
	connect( checkbox, SIGNAL( stateChanged( int)), this, SLOT( valueChanged(int)));

	layout->addWidget( label);
	layout->addWidget( checkbox);
}

CheckBoxWidget::~CheckBoxWidget()
{
}

void CheckBoxWidget::valueChanged( int value)
{
	num->n = ( value == Qt::Checked ? 1 : 0 );
}
