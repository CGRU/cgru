#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>


std::ostream& operator<<(std::ostream& out, const std::vector<int>& rhs) 
{
    std::ostringstream joined;
    const char* const delim = ",";
    std::copy(rhs.begin(), rhs.end(), std::ostream_iterator<int>(joined, delim));

    out << "[";
    out << joined.str();
    out << "]";
    return out;
}

const std::string Time2strHMS( int time32, bool clamp)
{
    if( time32 == 0 )
        return "-";

    static const int timeStrLenMax = 64;
    char buffer[timeStrLenMax];

    int hours = time32 / 3600;
    time32 -= hours * 3600;
    int minutes = time32 / 60;
    int seconds = time32 - minutes * 60;
    int days = hours / 24;
    if( days > 1 ) hours -= days * 24;

    std::string str;

    if( days > 1 )
    {
        sprintf( buffer, "%dd ", days);
        str += buffer;
    }

    if( clamp )
    {
        if( hours )
        {
            sprintf( buffer, "%d", hours); str += buffer;
            if( minutes || seconds )
            {
            sprintf( buffer, ":%02d", minutes); str += buffer;
            if( seconds ) { sprintf( buffer, ":%02d", seconds); str += buffer;}
            }
            else str += "h";
        }
        else if( minutes )
        {
            sprintf( buffer, "%d", minutes); str += buffer;
            if( seconds ) { sprintf( buffer, ":%02d", seconds); str += buffer;}
            else str += "m";
        }
        else if( seconds ) { sprintf( buffer, "%ds", seconds); str += buffer;}
    }
    else
    {
        sprintf( buffer, "%d:%02d:%02d", hours, minutes, seconds);
        str += buffer;
    }

    return str;
}


int combine(int a, int b, int c)
{
    return (a << 20) | (b << 10) | c;
}

unsigned short combine2(char a, char b, char c)
{
    unsigned short ret = 0;
    ret |= static_cast<char>(a) << 4;
    ret |= static_cast<char>(b) << 2;
    ret |= static_cast<char>(c);
    return ret;
}

void unpack(int combined, int *a, int *b, int *c)
{
    *a = combined >> 20;
    *b = (combined >> 10) & 0x3ff;
    *c = combined & 0x3ff;
}


size_t combine(int a, int b, int c, int d) 
{
    size_t ret = 0ULL;
    ret |= static_cast<size_t>(a) << 48;
    ret |= static_cast<size_t>(b) << 32;
    ret |= static_cast<size_t>(c) << 16;
    ret |= static_cast<size_t>(d);
    return ret;
}

void unpack(size_t combined, int *a, int *b, int *c, int *d)
{
    *a = (combined >> 48);
    *b = (combined >> 32) & 0xffff;
    *c = (combined >> 16) & 0xffff;
    *d = combined & 0xffff;
}
