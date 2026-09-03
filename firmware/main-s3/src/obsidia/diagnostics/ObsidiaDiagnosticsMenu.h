#pragma once

#ifdef OBSIDIA_V1

#include "DiagnosticsModel.hpp"
#include "MenuItemInterface.h"

extern obsidia::DiagnosticsModel obsidiaDiagnosticsModel;

class ObsidiaDiagnosticsMenu : public MenuItemInterface {
public:
    ObsidiaDiagnosticsMenu() : MenuItemInterface("OBSIDIA DIAGNOSTICS") {}
    void optionsMenu() override;
    void drawIcon(float scale) override;
    bool hasTheme() override { return false; }
    const String &themePath() override;
};

#endif
