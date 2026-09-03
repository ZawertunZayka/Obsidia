#include "DiagnosticsModel.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

namespace {

const obsidia::DiagnosticsModel::Entry &entry(
    const obsidia::DiagnosticsModel &model,
    obsidia::DiagnosticsModel::Component component
) {
    return model.entries()[static_cast<std::size_t>(component)];
}

void testCompleteInventoryAndBounds() {
    obsidia::DiagnosticsModel model;
    assert(model.entries().size() == 20);
    assert(std::strcmp(model.entries().front().label, "Main MCU") == 0);
    assert(std::strcmp(model.entries().back().label, "FPGA FIFO") == 0);

    model.set(
        obsidia::DiagnosticsModel::Component::ObsidiaFirmwareVersion,
        obsidia::DiagnosticsModel::State::Pass,
        "123456789012345678901234567890"
    );
    const auto &value = entry(model, obsidia::DiagnosticsModel::Component::ObsidiaFirmwareVersion).value;
    assert(value.back() == '\0');
    assert(std::strlen(value.data()) == obsidia::DiagnosticsModel::kValueCapacity - 1);
}

void testCoprocessorMappingDoesNotInventPasses() {
    obsidia::DiagnosticsModel model;
    obsidia::RadioService::Snapshot radio;
    radio.health = obsidia::RadioService::Health::Ready;
    radio.protocolVersion = 1;
    radio.firmwareVersion = {0, 1, 0};
    radio.supportedMask = 0x07;
    radio.readyMask = 0x05;
    model.updateRadio(radio);
    assert(entry(model, obsidia::DiagnosticsModel::Component::RadioMcu).state ==
           obsidia::DiagnosticsModel::State::Pass);
    assert(std::strcmp(
        entry(model, obsidia::DiagnosticsModel::Component::ObsidiaLinkVersion).value.data(), "1"
    ) == 0);
    assert(entry(model, obsidia::DiagnosticsModel::Component::Rdm6300).state ==
           obsidia::DiagnosticsModel::State::Pass);
    assert(entry(model, obsidia::DiagnosticsModel::Component::Cc1101).state ==
           obsidia::DiagnosticsModel::State::Untested);
    assert(entry(model, obsidia::DiagnosticsModel::Component::Nrf24).state ==
           obsidia::DiagnosticsModel::State::Pass);

    obsidia::FpgaService::Snapshot fpga;
    fpga.health = obsidia::FpgaService::Health::Ready;
    fpga.version = {1, 0};
    model.updateFpga(fpga);
    assert(entry(model, obsidia::DiagnosticsModel::Component::Fpga).state ==
           obsidia::DiagnosticsModel::State::Pass);
    assert(entry(model, obsidia::DiagnosticsModel::Component::FpgaIrq).state ==
           obsidia::DiagnosticsModel::State::Untested);
    assert(entry(model, obsidia::DiagnosticsModel::Component::FpgaFifo).state ==
           obsidia::DiagnosticsModel::State::Untested);
}

} // namespace

int main() {
    testCompleteInventoryAndBounds();
    testCoprocessorMappingDoesNotInventPasses();
    std::cout << "DiagnosticsModel tests passed\n";
    return 0;
}
