#include "nrf_spectrum.h"

#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/spectrum_plot.h"

#ifdef OBSIDIA_V1
#include "obsidia/services/ObsidiaRadioRuntime.hpp"
#endif

#define CHANNELS 80
#define NRF_FULL_SCALE 125

uint8_t channel[CHANNELS];

#ifndef OBSIDIA_V1
String scanChannels(bool web) {
    String result = "{";
    uint8_t rpdValues[CHANNELS] = {0};
    digitalWrite(bruceConfigPins.NRF24_bus.io0, LOW);

    for (int i = 0; i < CHANNELS; ++i) {
        NRFradio.setChannel(i);
        NRFradio.startListening();
        delayMicroseconds(128);
        NRFradio.stopListening();

        const int rpd = NRFradio.testRPD() ? 1 : 0;
        channel[i] = (channel[i] * 3U + rpd * NRF_FULL_SCALE) / 4U;
        rpdValues[i] = channel[i];
    }

    digitalWrite(bruceConfigPins.NRF24_bus.io0, HIGH);

    if (web) {
        for (int i = 0; i < CHANNELS; ++i) {
            if (i > 0) result += ",";
            result += String(rpdValues[i]);
        }
        result += "}";
    }
    return result;
}
#else
static bool scanRemoteChannels(String *webResult = nullptr) {
    String result = "{";
    for (int i = 0; i < CHANNELS; ++i) {
        std::uint8_t rpd = 0;
        if (!obsidia::remoteNrf24ReadRpd(static_cast<std::uint8_t>(i), rpd)) return false;
        channel[i] = static_cast<std::uint8_t>(
            (static_cast<unsigned>(channel[i]) * 3U + (rpd ? NRF_FULL_SCALE : 0U)) / 4U
        );
        if (webResult != nullptr) {
            if (i > 0) result += ",";
            result += String(channel[i]);
        }
    }
    if (webResult != nullptr) {
        result += "}";
        *webResult = result;
    }
    return true;
}

String scanChannels(bool web) {
    String result;
    if (!scanRemoteChannels(web ? &result : nullptr)) return "{}";
    return result;
}
#endif

static void nrfEnvelope(const uint8_t *levels, uint8_t *envelope, int plotWidth) {
    for (int x = 0; x < plotWidth; ++x) {
        const int32_t position = static_cast<int32_t>(x) * (CHANNELS - 1) * 256 / (plotWidth - 1);
        int channelIndex = position >> 8;
        int fraction = position & 0xff;
        if (channelIndex >= CHANNELS - 1) {
            channelIndex = CHANNELS - 2;
            fraction = 256;
        }
        const int level = levels[channelIndex] +
                          (levels[channelIndex + 1] - levels[channelIndex]) * fraction / 256;
        const int scaled = level * 100 / NRF_FULL_SCALE;
        envelope[x] = static_cast<uint8_t>(scaled < 0 ? 0 : (scaled > 100 ? 100 : scaled));
    }
}

void nrf_spectrum() {
#ifdef OBSIDIA_V1
    const auto radio = obsidia::radioRuntimeSnapshot();
    if (radio.health != obsidia::RadioService::Health::Ready ||
        (radio.readyMask & (1U << 2U)) == 0U) {
        displayError("Remote NRF24 not ready", true);
        return;
    }
#endif

    SpectrumPlot plot;
    if (!plot.begin("NRF Spectrum")) {
        displayError("Out of memory", true);
        return;
    }

    const int plotWidth = plot.width();
    uint8_t *envelope = static_cast<uint8_t *>(malloc(plotWidth));
    uint8_t *peakEnvelope = static_cast<uint8_t *>(malloc(plotWidth));
    uint8_t peak[CHANNELS] = {0};
    if (envelope == nullptr || peakEnvelope == nullptr) {
        free(envelope);
        free(peakEnvelope);
        plot.end();
        displayError("Out of memory", true);
        return;
    }

    constexpr int tickCount = 5;
    int columns[tickCount];
    String labels[tickCount];
    for (int i = 0; i < tickCount; ++i) {
        const int channelIndex = i * (CHANNELS - 1) / (tickCount - 1);
        columns[i] = channelIndex * (plotWidth - 1) / (CHANNELS - 1);
        labels[i] = String(2.400f + channelIndex * 0.001f, 2);
    }
    plot.ruler(columns, labels, tickCount);
    plot.status("scanning NRF24...");

#ifndef OBSIDIA_V1
    if (!nrf_start(NRF_MODE_SPI)) {
        Serial.println("Fail Starting radio");
        free(envelope);
        free(peakEnvelope);
        plot.end();
        displayError("NRF24 not found");
        delay(500);
        return;
    }

    NRFradio.setAutoAck(false);
    NRFradio.disableCRC();
    NRFradio.setAddressWidth(2);
    const uint8_t noiseAddress[][2] = {
        {0x55, 0x55}, {0xAA, 0xAA}, {0xA0, 0xAA},
        {0xAB, 0xAA}, {0xAC, 0xAA}, {0xAD, 0xAA}
    };
    for (uint8_t i = 0; i < 6; ++i) NRFradio.openReadingPipe(i, noiseAddress[i]);
    NRFradio.setDataRate(RF24_1MBPS);
#endif

    uint32_t lastFrame = 0;
    uint32_t lastRow = 0;
    bool remoteFailure = false;
    while (!check(EscPress)) {
#ifdef OBSIDIA_V1
        if (!scanRemoteChannels()) {
            remoteFailure = true;
            break;
        }
#else
        scanChannels(false);
#endif

        int busiestChannel = 0;
        for (int i = 0; i < CHANNELS; ++i) {
            if (channel[i] > peak[i]) peak[i] = channel[i];
            else if (peak[i] > 0) --peak[i];
            if (channel[i] > channel[busiestChannel]) busiestChannel = i;
        }

        if (millis() - lastFrame >= 40U) {
            lastFrame = millis();
            nrfEnvelope(channel, envelope, plotWidth);
            nrfEnvelope(peak, peakEnvelope, plotWidth);

            const int highlight = busiestChannel * (plotWidth - 1) / (CHANNELS - 1);
            const int span = (2 * (plotWidth - 1)) / (CHANNELS - 1);
            plot.trace(envelope, peakEnvelope, highlight - span, highlight + span);

            if (millis() - lastRow >= 120U) {
                lastRow = millis();
                plot.pushRow(envelope);
                plot.status(
                    "peak ch" + String(busiestChannel) + "  " +
                    String(2.400f + busiestChannel * 0.001f, 3) + "GHz  " +
                    String(envelope[highlight]) + "%"
                );
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

#ifdef OBSIDIA_V1
    obsidia::remoteNrf24Stop();
#else
    NRFradio.stopListening();
    NRFradio.powerDown();
#endif
    free(envelope);
    free(peakEnvelope);
    plot.end();
    if (remoteFailure) displayError("Remote NRF24 link error", true);
    delay(250);
}
