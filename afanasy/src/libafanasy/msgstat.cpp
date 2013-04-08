#include "msgstat.h"

#include <string.h>
#include <time.h>

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

MsgStat::MsgStat():
   initialized( false)
{
   uint32_t time_cur = time( NULL);
   for( int s = 1; s < STORE; s++)
   {
      lasttime_S[s] = time_cur;
      lastmsgs_S[s] = 0;
   }

   for( int t = 0; t < af::Msg::TLAST; t++)
   {
      msgs_T[t].size  = 0;
      msgs_T[t].count = 0;
      msgsizemax_T[t] = 0;
      for( int s = 1; s < STORE; s++)
      {
         for( int d = 0; d < DIVISIONS; d++)
         {
            msgs_STD[s][t][d].size  = 0;
            msgs_STD[s][t][d].count = 0;
         }
      }
   }

   initialized = true;
}

MsgStat::~MsgStat()
{
}

void MsgStat::put( int type, int size)
{
   if( type >= af::Msg::TLAST)
   {
      AFERRAR("MsgStat::put: type >= af::Msg::TLAST ( %d >= %d )\n", type, af::Msg::TLAST);
      return;
   }

//BEGIN mutex WRITE
    m_mutex.Lock();;
/*-------------------------------------------------------------*/

   msgs_T[type].size += size;
   msgs_T[type].count++;

   if( msgsizemax_T[type] < size) msgsizemax_T[type] = size;

   uint32_t time_cur = time( NULL);
   if( lasttime_S[0] < (time_cur-DTIME))
   {
      for( int t = 0; t < af::Msg::TLAST; t++)
      {
         msgs_STD[0][t][lastmsgs_S[0]].size  = msgs_T[t].size;
         msgs_STD[0][t][lastmsgs_S[0]].count = msgs_T[t].count;

         msgs_T[t].size = 0;
         msgs_T[t].count = 0;
      }
      lastmsgs_S[0]++;
      if( lastmsgs_S[0] >= DIVISIONS) lastmsgs_S[0] = 0;
      lasttime_S[0] = time_cur;

      uint32_t dtime = DTIME;
      for( int s = 1; s < STORE; s++)
      {
         dtime *= DIVISIONS;
         if((lasttime_S[s]+dtime) > time_cur) break;

         for( int t = 0; t < af::Msg::TLAST; t++)
         {
            int size_sum = 0;
            int count_sum = 0;
            for( int d = 0; d < DIVISIONS; d++)
            {
               size_sum  += msgs_STD[s-1][t][d].size;
               count_sum += msgs_STD[s-1][t][d].count;
            }
            msgs_STD[s][t][lastmsgs_S[s]].size  = size_sum;
            msgs_STD[s][t][lastmsgs_S[s]].count = count_sum;
         }

         lasttime_S[s] = time_cur;
         lastmsgs_S[s]++;
         if( lastmsgs_S[s] >= DIVISIONS) lastmsgs_S[s] = 0;
      }
   }

/*-------------------------------------------------------------*/
   m_mutex.Unlock();;
//END mutex WRITE

}

void MsgStat::writeStat( af::Msg * msg)
{
//BEGIN mutex
    m_mutex.Lock();
/*-------------------------------------------------------------*/

   for( int s = 0; s < STORE; s++)
   {
      for( int t = 0; t < af::Msg::TLAST; t++)
      {
         msgs_ST[s][t].size  = 0;
         msgs_ST[s][t].count = 0;
         for( int d = 0; d < DIVISIONS; d++)
         {
            msgs_ST[s][t].size  += msgs_STD[s][t][d].size;
            msgs_ST[s][t].count += msgs_STD[s][t][d].count;
         }
      }
   }

/*-------------------------------------------------------------*/
   m_mutex.Unlock();
//END mutex

   if( msg == NULL ) return;

   msg->set( af::Msg::TStatData, this);

}

void MsgStat::readStat( af::Msg * msg)
{
   read( msg);
}

void MsgStat::v_readwrite( af::Msg * msg)
{
   for( int s = 0; s < STORE; s++)
   {
      for( int t = 0; t < af::Msg::TLAST; t++)
      {
         rw_int32_t( msgs_ST[s][t].size,  msg);
         rw_int32_t( msgs_ST[s][t].count, msg);
      }
   }
   for( int t = 0; t < af::Msg::TLAST; t++)
   {
      rw_int32_t( msgsizemax_T[t], msg);
   }
}

void MsgStat::v_stdOut( bool full ) const
{
   v_stdOut( DIVISIONS, true );
}

void MsgStat::v_stdOut( int columns, int sorting) const
{
   if((columns > STORE) || ( columns < 0 )) columns = STORE;

//
// Some output formatting constants

   const int msgtypenumberlen = 5;
   const int columnlen = 12;
   const int avgmaxcolumnlen = 8;
   char sep[] = "   |";

//
// Some variables to store summary

   int average = 0;
   int average_count = 0;
   int maxsize = 0;
   int totalcount_S[ STORE];
   int totalsize_S[  STORE];

//
// Some initializations

   char buf[128];
   int maxnamelen = 0;

   for( int t = 0; t < af::Msg::TLAST; t++)
   {
      int namelen = strlen( af::Msg::TNAMES[t]);
      if( maxnamelen < namelen ) maxnamelen = namelen;
   }

   for( int s = 0; s < STORE; s++)
   {
      totalcount_S[ s] = 0;
      totalsize_S[  s] = 0;
   }

//
// Table headers output

   printf("\n");
   int len;
   {
      {char str[] = "time:"; printf( "%s", str); len = strlen( str);
      for( int s = 0; s < maxnamelen+msgtypenumberlen-len; s++) printf(" ");
      printf( "%s", sep);}
      {char str[] = " messages size:"; printf( "%s", str); len = strlen( str);}
      for( int s = 0; s < (avgmaxcolumnlen<<1)-len; s++) printf(" ");

      uint32_t dtime = DTIME;
      for( int c = 0; c < columns; c++)
      {
         printf( "%s", sep);
         dtime *= DIVISIONS;
         sprintf( buf, " %u secs:", dtime);
         printf( "%s", buf); len = strlen( buf);
         for( int s = 0; s < (columnlen<<1)-len; s++) printf(" ");
      }
      printf("\n");

      {char str[] = "TYPE:"; printf( "%s", str); len = strlen( str);}
      for( int s = 0; s < maxnamelen+msgtypenumberlen-len; s++) printf(" ");

      printf( "%s", sep);

      {char str[] = "average"; len = strlen( str);
      for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
      printf( "%s", str);}

      {char str[] = "maximum"; len = strlen( str);
      for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
      printf( "%s", str);}

      for( int c = 0; c < columns; c++)
      {
         printf( "%s", sep);

         {char str[] = "count"; len = strlen( str);
         for( int s = 0; s < columnlen-len; s++) printf(" ");
         printf( "%s", str);}

         {char str[] = "size"; len = strlen( str);
         for( int s = 0; s < columnlen-len; s++) printf(" ");
         printf( "%s", str);}
      }
      printf("\n");
   }

//
// averages calculations

   int32_t avg_T[af::Msg::TLAST];
   for( int t = 0; t < af::Msg::TLAST; t++)
   {
      double size = 0;
      int count = 0;
      for( int col = 0; col < columns; col++)
      {
         size += msgs_ST[col][t].size;
         count += msgs_ST[col][t].count;
      }
      if( count == 0 )
         avg_T[t] = 0;
      else
      {
         avg_T[t] = int( size/count);
         average += avg_T[t];
         average_count++;
      }
   }

//
// Indexes table for sorted output
   int index[af::Msg::TLAST]; // indexes table
   for( int i = 0; i < af::Msg::TLAST; i++) index[i] = i;
   if( sorting >= 0 )
   {
      int value[af::Msg::TLAST]; // values array
      if( sorting < 2 ) // For "average size" and "maximum size" columns
      {
         for( int i = 0; i < af::Msg::TLAST; i++)
            if( sorting == 0) value[i] = avg_T[i]; // copy values from "average size" column
            else value[i] = msgsizemax_T[i]; // copy values from "maximum size" column
      }
      else // For specified column "count" and "size"
      {
         const int maxsorting = 1 + 2*columns; // maximum column index
         if( sorting > maxsorting ) sorting = maxsorting; // clamp
         int sortcolumn = ( sorting - 2 ) / 2;
         bool countORsize = ( sorting % 2 ) == 0;
         for( int i = 0; i < af::Msg::TLAST; i++)
            if( countORsize) value[i] = msgs_ST[sortcolumn][i].count; // copy values from "count" column
            else             value[i] = msgs_ST[sortcolumn][i].size;  // copy values from "size" column
      }
      // Sorting indexes according to values (the simplest and slowest method)
      for( int i = 0; i < af::Msg::TLAST; i++)
      {
         for( int j = af::Msg::TLAST-1; j > i; j--)
         {
            int value_a = value[j-1];
            int value_b = value[j];
            int index_a = index[j-1];
            int index_b = index[j];
            if( value_a < value_b )
            {
               value[j-1] = value_b;
               value[j]   = value_a;
               index[j-1] = index_b;
               index[j]   = index_a;
            }
         }
      }

   }

//
// Table data output

   printf("\n");
   for( int t = 0; t < af::Msg::TLAST; t++)
   {
      sprintf( buf, "#%d", t);
      printf( "%s", buf); len = strlen( buf);
      for( int s = 0; s < msgtypenumberlen-len; s++) printf(" ");

      printf( "%s", af::Msg::TNAMES[index[t]]);
      int spaces = maxnamelen - strlen( af::Msg::TNAMES[index[t]]);
      for( int s = 0; s < spaces; s++) printf(" ");

      printf( "%s", sep);

      sprintf( buf, "%u", avg_T[index[t]]); len = strlen( buf);
      for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
      printf( "%s", buf);

      sprintf( buf, "%u", msgsizemax_T[index[t]]); len = strlen( buf);
      for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
      printf( "%s", buf);

      if( maxsize < msgsizemax_T[index[t]]) maxsize = msgsizemax_T[index[t]];

      for( int col = 0; col < columns; col++)
      {
         printf( "%s", sep);

         sprintf( buf, "%u", msgs_ST[col][index[t]].count); len = strlen( buf);
         for( int s = 0; s < columnlen-len; s++) printf(" ");
         printf( "%s", buf);

         sprintf( buf, "%u", msgs_ST[col][index[t]].size); len = strlen( buf);
         for( int s = 0; s < columnlen-len; s++) printf(" ");
         printf( "%s", buf);

         totalcount_S[col] += msgs_ST[col][index[t]].count;
         totalsize_S[ col] += msgs_ST[col][index[t]].size;
      }
      printf("\n");
   }

//
// TOTAL statistics output:

   printf("\n");
   if( average_count == 0 ) average = 0;
   else average = average/average_count;

   for( int s = 0; s < msgtypenumberlen; s++) printf(" ");
   {char str[] = "TOTAL:"; printf( "%s", str); len = strlen( str);}
   for( int s = 0; s < maxnamelen-len; s++) printf(" ");

   printf( "%s", sep);

   sprintf( buf, "%u", average); len = strlen( buf);
   for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
   printf( "%s", buf);

   sprintf( buf, "%u", maxsize); len = strlen( buf);
   for( int s = 0; s < avgmaxcolumnlen-len; s++) printf(" ");
   printf( "%s", buf);

   for( int col = 0; col < columns; col++)
   {
      printf( "%s", sep);

      sprintf( buf, "%u", totalcount_S[col]); len = strlen( buf);
      for( int s = 0; s < columnlen-len; s++) printf(" ");
      printf( "%s", buf);

      sprintf( buf, "%u", totalsize_S[col]); len = strlen( buf);
      for( int s = 0; s < columnlen-len; s++) printf(" ");
      printf( "%s", buf);
   }

   printf("\n");
   printf("\n");
}
