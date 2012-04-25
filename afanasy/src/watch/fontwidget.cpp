#include "fontwidget.h"

#include "../libafqt/attr.h"

#include "watch.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

FontWidget::FontWidget( QWidget * parent, afqt::Attr * attrString):
      QWidget( parent),
      attr( attrString)
{
   QHBoxLayout * layout = new QHBoxLayout(this);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 1, 1, 1, 1);
#endif
   layout->setSpacing( 2);
   QLabel * label = new QLabel( attr->getLabel(), this);
   lineedit = new QLineEdit( this);
   QSizePolicy policy;
   policy.setHorizontalPolicy( QSizePolicy::Minimum);
   lineedit->setSizePolicy( policy);
   lineedit->setText( attr->str);
   connect( lineedit, SIGNAL( editingFinished()), this, SLOT( editingFinished()));

   layout->addWidget( label);
   layout->addWidget( lineedit);
}

FontWidget::~FontWidget()
{
}

void FontWidget::editingFinished()
{
   attr->str = lineedit->text();
   Watch::repaint();
}
