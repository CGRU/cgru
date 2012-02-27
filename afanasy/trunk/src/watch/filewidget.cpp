#include "filewidget.h"

#include <QtGui/QFileDialog>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

#include "../libafqt/attr.h"

#include "watch.h"

FileWidget::FileWidget( QWidget * parent, afqt::Attr * attrString):
    QWidget( parent),
    attr( attrString)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 1, 1, 1, 1);
#endif
    layout->setSpacing( 2);
    QSizePolicy policy;
    policy.setHorizontalPolicy( QSizePolicy::Minimum);

    QLabel * label = new QLabel( attr->getLabel(), this);
    policy.setHorizontalPolicy( QSizePolicy::Maximum);
    label->setSizePolicy( policy);
    layout->addWidget( label);

    lineedit = new QLineEdit( this);
    policy.setHorizontalPolicy( QSizePolicy::Minimum);
    lineedit->setSizePolicy( policy);
    lineedit->setText( attr->str);
    connect( lineedit, SIGNAL( editingFinished()), this, SLOT( editingFinished()));
    layout->addWidget( lineedit);

    QPushButton * button = new QPushButton( "Browse", this);
    connect( button, SIGNAL( pressed()), this, SLOT( browse()));
    policy.setHorizontalPolicy( QSizePolicy::Maximum);
    button->setSizePolicy( policy);
    layout->addWidget( button);
}

FileWidget::~FileWidget()
{
}

void FileWidget::browse()
{
    QString afile = QFileDialog::getOpenFileName( this, "Choose Sound File", "", "Sounds [*.wav] (*.wav)");

    if( afile.isEmpty())
        return;

    lineedit->setText( afile);
    attr->str = lineedit->text();
}

void FileWidget::editingFinished()
{
    attr->str = lineedit->text();
}
