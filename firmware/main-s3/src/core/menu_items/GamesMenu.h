#pragma once

#ifdef OBSIDIA_V1

#include <MenuItemInterface.h>

class GamesMenu : public MenuItemInterface {
public:
    GamesMenu() : MenuItemInterface("Games") {}
    void optionsMenu() override;
    void drawIcon(float scale) override;
    bool hasTheme() override { return false; }
    const String &themePath() override {
        static const String empty;
        return empty;
    }
};

#endif
