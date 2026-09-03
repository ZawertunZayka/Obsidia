#include "ObsidiaDiagnosticsMenu.h"

#ifdef OBSIDIA_V1

#include "core/display.h"

obsidia::DiagnosticsModel obsidiaDiagnosticsModel;

void ObsidiaDiagnosticsMenu::optionsMenu() {
    options.clear();
    for (const auto &entry : obsidiaDiagnosticsModel.entries()) {
        String line(entry.label);
        line += ": ";
        line += obsidia::DiagnosticsModel::stateText(entry.state);
        if (entry.value[0] != '\0') {
            line += " ";
            line += entry.value.data();
        }
        options.emplace_back(line, []() {});
    }
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "OBSIDIA DIAGNOSTICS");
}

void ObsidiaDiagnosticsMenu::drawIcon(float scale) {
    clearIconArea();
    const int radius = static_cast<int>(scale * 28);
    tft.drawCircle(iconCenterX, iconCenterY, radius, bruceConfig.priColor);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(static_cast<int>(scale * 2) > 0 ? static_cast<int>(scale * 2) : 1);
    tft.drawCentreString("D", iconCenterX, iconCenterY - static_cast<int>(scale * 8), 1);
}

const String &ObsidiaDiagnosticsMenu::themePath() {
    static const String empty;
    return empty;
}

#endif
