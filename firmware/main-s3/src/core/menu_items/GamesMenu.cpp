#ifdef OBSIDIA_V1

#include "GamesMenu.h"

#include "core/display.h"
#include "core/utils.h"
#include <obsidia_doom.h>

void GamesMenu::optionsMenu() {
    options = {
        {"DOOM", []() {
             if (!obsidia_doom_wad_present()) {
                 displayError("DOOM WAD missing", true);
                 return;
             }
             obsidia_doom_start();
         }},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Games");
}

void GamesMenu::drawIcon(float scale) {
    clearIconArea();
    const int w = 62 * scale;
    const int h = 38 * scale;
    const int x = iconCenterX - w / 2;
    const int y = iconCenterY - h / 2;
    tft.drawRoundRect(x, y, w, h, 5 * scale, bruceConfig.priColor);
    tft.fillCircle(x + 16 * scale, iconCenterY, 4 * scale, bruceConfig.priColor);
    tft.fillCircle(x + w - 16 * scale, iconCenterY, 4 * scale, bruceConfig.priColor);
}

#endif
