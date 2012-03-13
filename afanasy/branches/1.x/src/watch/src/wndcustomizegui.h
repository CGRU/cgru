#pragma once

#include "wnd.h"

#include <name_af.h>

class ColorWidget;

class WndCustomizeGUI : public Wnd
{
Q_OBJECT
public:
   WndCustomizeGUI();
   ~WndCustomizeGUI();

private slots:
   void save();
};
