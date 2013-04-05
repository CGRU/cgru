#pragma once

namespace AFGUI
{
    const char THEME[] = "Civil";

    const int SAVEPREFSONEXIT = 1;
    const int SAVEWNDRECTS = 1;
    const int SAVEGUI = 0;
    const int SHOWOFFLINENOISE = 1;

    const char FONT_FAMILY[] = "SansSerif";

    const int STAR_NUMPOINTS = 5;
    const int STAR_RADIUSOUT = 100;
    const int STAR_RADIUSIN  = 40;
    const int STAR_ROTATE    = 0;

#ifdef MACOSX

    const int  FONT_SIZENAME            = 11;
    const int  FONT_SIZEINFO            = 10;
    const int  FONT_SIZEPLOTTER         = 7;
    const int  FONT_SIZEMIN             = 5;

    // Qt GUI Palette:
    const char CLR_WINDOW[]          = "4E4E4E";
    const char CLR_WINDOWTEXT[]      = "CFCFCF";
    const char CLR_BASE[]            = "3F3F3F";
    const char CLR_TEXT[]            = "BABABA";
    const char CLR_BUTTON[]          = "919191";

    const char CLR_LIGHT[]           = "FFFFFF";
    const char CLR_MIDLIGHT[]        = "9F9F9F";
    const char CLR_MID[]             = "404040";
    const char CLR_DARK[]            = "616161"; //  user mode
    const char CLR_SHADOW[]          = "AEAEAE";

    const char CLR_HIGHLIGHT[]       = "D5D5D5";
    const char CLR_HIGHLIGHTEDTEXT[] = "003F00";

    // Qt GUI Palette - not used, for user and super user modes:
    const char CLR_LINK[]            = "F7B312"; // Visor mode
    const char CLR_LINKVISITED[]     = "CAFFB9"; //  GOD  mode

    // Watch specific colors:
    const char CLR_ITEM[]            = "5E5E5E";
    const char CLR_SELECTED[]        = "5D6770";
    const char CLR_ITEMJOB[]         = "3A464C";
    const char CLR_ITEMJOBERROR[]    = "7F6550";
    const char CLR_ITEMJOBOFF[]      = "50544C";
    const char CLR_ITEMJOBWTIME[]    = "477777";
    const char CLR_ITEMJOBWDEP[]     = "61546E";
    const char CLR_ITEMJOBDONE[]     = "2A394C";
    const char CLR_TASKSKIPPED[]     = "555555";
    const char CLR_TASKWARNINGRUN[]  = "CCCC55";
    const char CLR_TASKWARNINGDONE[] = "D67F50";
    const char CLR_ITEMRENDER[]      = "6D6C69";
    const char CLR_ITEMRENDEROFF[]   = "50544C";
    const char CLR_ITEMRENDERBUSY[]  = "637840";
    const char CLR_ITEMRENDERNIMBY[] = "5E778F";
    const char CLR_ITEMRENDERPLTCLR[]= "FAFA32";
    const char CLR_RUNNING[]         = "FFFFFF";
    const char CLR_DONE[]            = "D67F00";
    const char CLR_ERROR[]           = "B92803";
    const char CLR_OUTLINE[]         = "727272";
    const char CLR_STAR[]            = "9099A3";
    const char CLR_STARLINE[]        = "000000";

    const char CLR_TEXTBRIGHT[]      = "FFFFFF";
    const char CLR_TEXTMUTED[]       = "AAAAAA";
    const char CLR_TEXTDONE[]        = "E7E2FF";
    const char CLR_TEXTSTARS[]       = "112C00";

#else

    const int  FONT_SIZENAME            = 10;
    const int  FONT_SIZEINFO            = 8;
    const int  FONT_SIZEPLOTTER         = 7;
    const int  FONT_SIZEMIN             = 5;

    // Qt GUI Palette:
    const char CLR_WINDOW[]          = "5F7844";
    const char CLR_WINDOWTEXT[]      = "320000";
    const char CLR_BASE[]            = "AAE682";
    const char CLR_TEXT[]            = "0A1E05";
    const char CLR_BUTTON[]          = "93BE57";

    const char CLR_LIGHT[]           = "FAFAFA";
    const char CLR_MIDLIGHT[]        = "AADC82";
    const char CLR_MID[]             = "89AA67";
    const char CLR_DARK[]            = "5A7846";
    const char CLR_SHADOW[]          = "14320A";

    const char CLR_HIGHLIGHT[]       = "003200";
    const char CLR_HIGHLIGHTEDTEXT[] = "A0D26E";

    // Qt GUI Palette - not used, for user and super user modes:
    const char CLR_LINK[]            = "F7B312"; // Visor mode
    const char CLR_LINKVISITED[]     = "CAFFB9"; //  GOD  mode

    // Watch specific colors:
    const char CLR_ITEM[]            = "638245";
    const char CLR_SELECTED[]        = "AADC82";
    const char CLR_ITEMJOB[]         = "56763B";
    const char CLR_ITEMJOBERROR[]    = "7F6550";
    const char CLR_ITEMJOBOFF[]      = "50544C";
    const char CLR_ITEMJOBWTIME[]    = "477777";
    const char CLR_ITEMJOBWDEP[]     = "61546E";
    const char CLR_ITEMJOBDONE[]     = "3D664C";
    const char CLR_TASKSKIPPED[]     = "444444";
    const char CLR_TASKWARNINGRUN[]  = "DC7C50";
    const char CLR_TASKWARNINGDONE[] = "246221";
    const char CLR_ITEMRENDER[]      = "43764A";
    const char CLR_ITEMRENDEROFF[]   = "50544C";
    const char CLR_ITEMRENDERBUSY[]  = "637840";
    const char CLR_ITEMRENDERNIMBY[] = "5E778F";
    const char CLR_ITEMRENDERPLTCLR[]= "FAFA32";
    const char CLR_RUNNING[]         = "A0D26E";
    const char CLR_DONE[]            = "14320A";
    const char CLR_ERROR[]           = "FA320A";
    const char CLR_OUTLINE[]         = "ADBD1F";
    const char CLR_STAR[]            = "FA320A";
    const char CLR_STARLINE[]        = "FFE838";

    const char CLR_TEXTBRIGHT[]      = "D2FFA0";
    const char CLR_TEXTMUTED[]       = "465A32";
    const char CLR_TEXTDONE[]        = "F0C83C";
    const char CLR_TEXTSTARS[]       = "14320A";

#endif

//   const char CLR_ALTERNATEBASE[]   = "000000";
}
