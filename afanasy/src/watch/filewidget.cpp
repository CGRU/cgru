#include "filewidget.h"

#include "../libafqt/attr.h"

#include "watch.h"

#include <QtGui/QFileDialog>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

FileWidget::FileWidget( QWidget * i_parent, afqt::Attr * i_attr,
                        const QString & i_filesmask):
    QWidget( i_parent),
    m_attr( i_attr),
    m_filesmask( i_filesmask)
{
    QHBoxLayout * layout = new QHBoxLayout(this);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 1, 1, 1, 1);
#endif
    layout->setSpacing( 2);
    QSizePolicy policy;
    policy.setHorizontalPolicy( QSizePolicy::Minimum);

    QLabel * label = new QLabel( m_attr->getLabel(), this);
    policy.setHorizontalPolicy( QSizePolicy::Maximum);
    label->setSizePolicy( policy);
    layout->addWidget( label);

    m_lineedit = new QLineEdit( this);
    policy.setHorizontalPolicy( QSizePolicy::Minimum);
    m_lineedit->setSizePolicy( policy);
    m_lineedit->setText( m_attr->str);
    connect( m_lineedit, SIGNAL( editingFinished()), this, SLOT( editingFinished()));
    layout->addWidget( m_lineedit);

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
    QString afile = QFileDialog::getOpenFileName( this,
                                                  QString("Browse %1 File").arg( m_attr->getLabel()),
                                                  m_lineedit->text(), m_filesmask);

    if( afile.isEmpty())
        return;

    m_lineedit->setText( afile);
    m_attr->str = m_lineedit->text();
    Watch::refreshGui();
}

void FileWidget::editingFinished()
{
    m_attr->str = m_lineedit->text();
    Watch::refreshGui();
}
