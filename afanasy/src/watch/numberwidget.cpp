#include "numberwidget.h"

#include "../libafqt/attrnumber.h"

#include "watch.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

NumberWidget::NumberWidget( QWidget * parent, afqt::AttrNumber * attrNumber):
    QWidget( parent),
    num( attrNumber)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
    #if QT_VERSION >= 0x040300
    layout->setContentsMargins( 1, 1, 1, 1);
    #endif
    layout->setSpacing( 2);
    QLabel * label = new QLabel( attrNumber->getLabel(), this);
    QSpinBox * spinbox = new QSpinBox( this);
    spinbox->setMinimum( -1000);
    spinbox->setMaximum( 1000);
    spinbox->setValue( attrNumber->n);
    connect( spinbox, SIGNAL( valueChanged( int)), this, SLOT( valueChanged(int)));

    layout->addWidget( label);
    layout->addWidget( spinbox);
}

NumberWidget::~NumberWidget()
{
}

void NumberWidget::valueChanged( int value)
{
    num->n = value;
    Watch::refreshGui();
}
