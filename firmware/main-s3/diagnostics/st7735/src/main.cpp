#include <Arduino.h>
#include <SPI.h>

#include <cstddef>
#include <cstdint>

namespace {

constexpr int kPinBacklight = 21;
constexpr int kPinChipSelect = 10;
constexpr int kPinDataCommand = 9;
constexpr int kPinReset = 14;
constexpr int kPinMosi = 11;
constexpr int kPinClock = 12;
constexpr std::uint16_t kWidth = 128;
constexpr std::uint16_t kHeight = 160;
constexpr std::uint32_t kSpiHz = 10000000;

constexpr std::uint8_t kSwReset = 0x01;
constexpr std::uint8_t kSleepOut = 0x11;
constexpr std::uint8_t kNormalOn = 0x13;
constexpr std::uint8_t kInvertOff = 0x20;
constexpr std::uint8_t kDisplayOn = 0x29;
constexpr std::uint8_t kColumnAddress = 0x2A;
constexpr std::uint8_t kRowAddress = 0x2B;
constexpr std::uint8_t kMemoryWrite = 0x2C;
constexpr std::uint8_t kMemoryAccess = 0x36;
constexpr std::uint8_t kPixelFormat = 0x3A;

constexpr std::uint16_t rgb565(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    return static_cast<std::uint16_t>(((red & 0xF8U) << 8U) |
                                      ((green & 0xFCU) << 3U) |
                                      (blue >> 3U));
}

constexpr std::uint16_t kBlack = rgb565(0, 0, 0);
constexpr std::uint16_t kWhite = rgb565(255, 255, 255);
constexpr std::uint16_t kRed = rgb565(255, 0, 0);
constexpr std::uint16_t kGreen = rgb565(0, 255, 0);
constexpr std::uint16_t kBlue = rgb565(0, 0, 255);

SPISettings displaySpi(kSpiHz, MSBFIRST, SPI_MODE0);

void logLine(const char *message) {
    Serial0.println(message);
    Serial.println(message);
}

void selectDisplay() {
    SPI.beginTransaction(displaySpi);
    digitalWrite(kPinChipSelect, LOW);
}

void deselectDisplay() {
    digitalWrite(kPinChipSelect, HIGH);
    SPI.endTransaction();
}

void writeCommand(std::uint8_t command, const std::uint8_t *data = nullptr,
                  std::size_t length = 0) {
    selectDisplay();
    digitalWrite(kPinDataCommand, LOW);
    SPI.transfer(command);
    if (length != 0U) {
        digitalWrite(kPinDataCommand, HIGH);
        SPI.writeBytes(data, length);
    }
    deselectDisplay();
}

void setAddressWindow(std::uint16_t x, std::uint16_t y, std::uint16_t width,
                      std::uint16_t height) {
    const std::uint16_t xEnd = x + width - 1U;
    const std::uint16_t yEnd = y + height - 1U;
    const std::uint8_t columns[] = {
        static_cast<std::uint8_t>(x >> 8U), static_cast<std::uint8_t>(x),
        static_cast<std::uint8_t>(xEnd >> 8U), static_cast<std::uint8_t>(xEnd)};
    const std::uint8_t rows[] = {
        static_cast<std::uint8_t>(y >> 8U), static_cast<std::uint8_t>(y),
        static_cast<std::uint8_t>(yEnd >> 8U), static_cast<std::uint8_t>(yEnd)};
    writeCommand(kColumnAddress, columns, sizeof(columns));
    writeCommand(kRowAddress, rows, sizeof(rows));
}

void fillRectangle(std::uint16_t x, std::uint16_t y, std::uint16_t width,
                   std::uint16_t height, std::uint16_t color) {
    if (width == 0U || height == 0U || x >= kWidth || y >= kHeight) return;
    if (x + width > kWidth) width = kWidth - x;
    if (y + height > kHeight) height = kHeight - y;

    setAddressWindow(x, y, width, height);
    selectDisplay();
    digitalWrite(kPinDataCommand, LOW);
    SPI.transfer(kMemoryWrite);
    digitalWrite(kPinDataCommand, HIGH);

    std::uint8_t pixels[128];
    for (std::size_t index = 0; index < sizeof(pixels); index += 2U) {
        pixels[index] = static_cast<std::uint8_t>(color >> 8U);
        pixels[index + 1U] = static_cast<std::uint8_t>(color);
    }
    std::uint32_t bytesRemaining = static_cast<std::uint32_t>(width) * height * 2U;
    while (bytesRemaining != 0U) {
        const std::size_t count = bytesRemaining < sizeof(pixels) ? bytesRemaining : sizeof(pixels);
        SPI.writeBytes(pixels, count);
        bytesRemaining -= count;
    }
    deselectDisplay();
}

void fillScreen(std::uint16_t color) {
    fillRectangle(0, 0, kWidth, kHeight, color);
}

const std::uint8_t *glyph(char character) {
    static constexpr std::uint8_t space[7] = {0, 0, 0, 0, 0, 0, 0};
    static constexpr std::uint8_t a[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static constexpr std::uint8_t b[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    static constexpr std::uint8_t d[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    static constexpr std::uint8_t i[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F};
    static constexpr std::uint8_t k[7] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    static constexpr std::uint8_t l[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    static constexpr std::uint8_t o[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static constexpr std::uint8_t p[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static constexpr std::uint8_t s[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    static constexpr std::uint8_t y[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    switch (character) {
        case 'A': return a;
        case 'B': return b;
        case 'D': return d;
        case 'I': return i;
        case 'K': return k;
        case 'L': return l;
        case 'O': return o;
        case 'P': return p;
        case 'S': return s;
        case 'Y': return y;
        default: return space;
    }
}

void drawText(std::uint16_t x, std::uint16_t y, const char *text,
              std::uint8_t scale, std::uint16_t color) {
    for (; *text != '\0'; ++text, x += 6U * scale) {
        const std::uint8_t *rows = glyph(*text);
        for (std::uint8_t row = 0; row < 7U; ++row) {
            for (std::uint8_t column = 0; column < 5U; ++column) {
                if ((rows[row] & (0x10U >> column)) != 0U) {
                    fillRectangle(x + column * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

void initializeDisplay() {
    digitalWrite(kPinReset, HIGH);
    delay(10);
    digitalWrite(kPinReset, LOW);
    delay(20);
    digitalWrite(kPinReset, HIGH);
    delay(150);

    writeCommand(kSwReset);
    delay(150);
    writeCommand(kSleepOut);
    delay(150);

    const std::uint8_t frameRate[] = {0x01, 0x2C, 0x2D};
    const std::uint8_t frameRatePartial[] = {0x01, 0x2C, 0x2D};
    const std::uint8_t frameRateIdle[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
    const std::uint8_t inversion = 0x07;
    const std::uint8_t power1[] = {0xA2, 0x02, 0x84};
    const std::uint8_t power2 = 0xC5;
    const std::uint8_t power3[] = {0x0A, 0x00};
    const std::uint8_t power4[] = {0x8A, 0x2A};
    const std::uint8_t power5[] = {0x8A, 0xEE};
    const std::uint8_t vcom = 0x0E;
    const std::uint8_t memoryAccess = 0xC8;
    const std::uint8_t pixelFormat = 0x05;
    writeCommand(0xB1, frameRate, sizeof(frameRate));
    writeCommand(0xB2, frameRatePartial, sizeof(frameRatePartial));
    writeCommand(0xB3, frameRateIdle, sizeof(frameRateIdle));
    writeCommand(0xB4, &inversion, 1);
    writeCommand(0xC0, power1, sizeof(power1));
    writeCommand(0xC1, &power2, 1);
    writeCommand(0xC2, power3, sizeof(power3));
    writeCommand(0xC3, power4, sizeof(power4));
    writeCommand(0xC4, power5, sizeof(power5));
    writeCommand(0xC5, &vcom, 1);
    writeCommand(kInvertOff);
    writeCommand(kMemoryAccess, &memoryAccess, 1);
    writeCommand(kPixelFormat, &pixelFormat, 1);
    writeCommand(kNormalOn);
    delay(10);
    writeCommand(kDisplayOn);
    delay(100);
}

void blinkBacklight() {
    for (std::uint8_t count = 0; count < 3U; ++count) {
        digitalWrite(kPinBacklight, HIGH);
        delay(250);
        digitalWrite(kPinBacklight, LOW);
        delay(250);
    }
    digitalWrite(kPinBacklight, HIGH);
}

} // namespace

void setup() {
    Serial0.begin(115200);
    Serial.begin(115200);
    delay(300);
    logLine("OBSIDIA ST7735 RAW SPI DIAGNOSTIC");

    pinMode(kPinBacklight, OUTPUT);
    pinMode(kPinChipSelect, OUTPUT);
    pinMode(kPinDataCommand, OUTPUT);
    pinMode(kPinReset, OUTPUT);
    digitalWrite(kPinChipSelect, HIGH);
    digitalWrite(kPinDataCommand, HIGH);
    blinkBacklight();

    SPI.begin(kPinClock, -1, kPinMosi, kPinChipSelect);
    initializeDisplay();

    struct TestFrame {
        std::uint16_t color;
        const char *name;
    };
    constexpr TestFrame frames[] = {
        {kRed, "RED"}, {kGreen, "GREEN"}, {kBlue, "BLUE"},
        {kBlack, "BLACK"}, {kWhite, "WHITE"}};
    for (const auto &frame : frames) {
        logLine(frame.name);
        fillScreen(frame.color);
        delay(1200);
    }

    fillScreen(kBlack);
    drawText(22, 58, "OBSIDIA", 2, kWhite);
    drawText(31, 83, "DISPLAY OK", 1, kGreen);
    logLine("[WAIT] Physical confirmation required: OBSIDIA / DISPLAY OK");
}

void loop() {
    static std::uint32_t lastHeartbeat = 0;
    const std::uint32_t now = millis();
    if (static_cast<std::uint32_t>(now - lastHeartbeat) >= 2000U) {
        lastHeartbeat = now;
        logLine("[ALIVE] ST7735 raw diagnostic running");
    }
    delay(20);
}
