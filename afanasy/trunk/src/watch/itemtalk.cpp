#include "itemtalk.h"

#include <QtCore/QEvent>
#include <QtNetwork/QHostAddress>
#include <QtGui/QPainter>

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemTalk::ItemTalk( af::Talk *talk):
   ItemNode( (af::Talk*)talk),
   username( talk->getUserName())
{
   tip += "Launched at "   + af::time2Qstr( talk->getTimeLaunch()    );
   tip += "\nRegistered at " + af::time2Qstr( talk->getTimeRegister()  );
   tip += "\n" + talk->getAddress()->getIPString()
               + ':' + talk->getAddress()->getPortString();
   tip += "\nLast update ";

   updateValues( talk, 0);

   height = 20;
}

ItemTalk::~ItemTalk()
{
}

void ItemTalk::updateValues( af::Node *node, int type)
{
   af::Talk * talk = (af::Talk*)node;
   tooltip = tip + af::time2Qstr( talk->getTimeUpdate());
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
         sort_str = &name;
         break;
      default:
         AFERRAR("ItemTalk::setSortType: Invalid type number = %d\n", type);
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
         filter_str = &name;
         break;
      default:
         AFERRAR("ItemTalk::setFilterType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}
