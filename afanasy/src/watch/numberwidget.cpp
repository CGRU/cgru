#include "numberwidget.h"

#include "../libafqt/attr.h"

#include "watch.h"

#include <QLabel>
#include <QLayout>
#include <QSpinBox>

NumberWidget::NumberWidget(QWidget *parent, afqt::AttrNumber *attrNumber) : QWidget(parent), num(attrNumber)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(1, 1, 1, 1);
	layout->setSpacing(2);
	QLabel *label = new QLabel(attrNumber->getLabel(), this);
	QSpinBox *spinbox = new QSpinBox(this);
	spinbox->setMinimum(-1000);
	spinbox->setMaximum(1000);
	spinbox->setValue(attrNumber->n);
	connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));

	layout->addWidget(label);
	layout->addWidget(spinbox);
}

NumberWidget::~NumberWidget() {}

void NumberWidget::valueChanged(int value)
{
	num->n = value;
	Watch::refreshGui();
}
