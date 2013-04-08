#include "itemtalk.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"

#include <QtCore/QEvent>
#include <QtNetwork/QHostAddress>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemTalk::ItemTalk( af::Talk *talk):
   ItemNode( (af::Talk*)talk),
   username( afqt::stoq( talk->getUserName()))
{
   updateValues( talk, 0);
   height = 20;
}

ItemTalk::~ItemTalk()
{
}

void ItemTalk::updateValues( af::Node *node, int type)
{
   af::Talk * talk = (af::Talk*)node;
   tooltip = afqt::stoq( talk->v_generateInfoString( true));
}

void ItemTalk::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   drawBack( painter, option);

//   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   painter->setPen(   clrTextMain( option) );
   painter->setFont(  afqt::QEnvironment::f_name);
   painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignHCenter, name );

   drawPost( painter, option);
}

bool ItemTalk::setSortType(   int type )
{
   resetSorting();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         sort_str = name;
         break;
      default:
         AFERRAR("ItemTalk::setSortType: Invalid type number = %d", type)
         return false;
   }
   return true;
}

bool ItemTalk::setFilterType( int type )
{
   resetFiltering();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         filter_str = name;
         break;
      default:
         AFERRAR("ItemTalk::setFilterType: Invalid type number = %d", type)
         return false;
   }
   return true;
}
