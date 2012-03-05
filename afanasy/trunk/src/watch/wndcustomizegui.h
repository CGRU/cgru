#pragma once

#include "../libafanasy/name_af.h"

#include "wnd.h"

class WndCustomizeGUI : public Wnd
{
Q_OBJECT
public:
    WndCustomizeGUI();
    ~WndCustomizeGUI();

private slots:
    void save();
};
