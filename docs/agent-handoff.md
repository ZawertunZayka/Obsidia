# Obsidia agent handoff

Продолжай работу автономно в `/home/zawe/Obsidia`. Сначала полностью прочитай
`/home/zawe/Obsidia/AGENTS.md` и соблюдай его. Пользователь не должен писать код,
править конфигурацию или разруливать Git вручную.

## Текущий статус проекта (2026-09-20)

Все критические физические и программные этапы выравнивания завершены:

1. **microSD и TFT (Пройдено на железе)**:
   - microSD переведена на выделенную шину HSPI (CLK=18, MOSI=3, MISO=46, CS=17).
   - Физический диагностический тест `PASS_SHARED_SPI_100_400KHZ` прошел 100/100 циклов.
   - Экранные SPI-операции больше не вызывают сбоев SD.
   - Карта определена (15800 MiB, FAT32), корень и `/bruce.conf` монтируются штатно.
   - Сохранённый старый `irTx=17` автоматически мигрирует в `-1`, исключая конфликты с `SDCARD_CS`.

2. **Темы оформления**:
   - Папка `/ModernWatchDogs` скопирована на microSD (`/run/media/zawe/FLIPPER/ModernWatchDogs`).
   - Все 10 PNG + JSON проверены по контрольным суммам SHA-256.

3. **Полное удаление DOOM (Выполнено и протестировано)**:
   - Библиотека `lib/ObsidiaDoom`, код `src/obsidia/doom`, пункты меню `GamesMenu` удалены.
   - Раздел `doomwad` вырезан из `custom_16Mb.csv`, основной раздел `factory` расширен до `0x8F0000`.
   - Удалены встраивание WAD из `build.py` и спецрежим загрузки из `src/main.cpp`.
   - Регрессионный TDD-тест `tests/firmware/test_no_doom.py` пройден.

4. **Поддержка RDM6300 (125 kHz RFID)**:
   - В `OBSIDIA_V1.ini` заданы дефолтные `SERIAL_RX=4` и `SERIAL_TX=5`.
   - Исходный модуль Bruce `RFID` -> `Read 125kHz` задействует `Rdm6300Parser` и `HardwareSerial`.
   - Распиновка подключения (VCC=5V, GND=GND, TX=GPIO4) зафиксирована.

5. **NRF24L01+ на радиосопроцессоре**:
   - Диагностика на HW-394 прошла физически: `STATUS=0E`, `CONFIG=08`,
     `RF_CH=02`, `RF_SETUP=0F`.
   - Основная прошивка `obsidia-radio` запускает эту диагностику при старте и
     выставляет NRF24 ready bit в `GET_STATUS` только после успешного теста.

## Подтверждённое железо и GPIO (`hardware/pinout/pinmap.md`)

- **ESP32-S3 N16R8**: `/dev/ttyACM0`
- **ST7735 TFT**: SCK=12, MOSI=11, MISO=13, CS=10, DC=9, RST=14, BL=21 (Inversion ON / `INVOFF`)
- **microSD (HSPI)**: SCK=18, MOSI=3, MISO=46, CS=17
- **CardKB**: SDA=40, SCL=41 (I2C `0x5F`)
- **UART RDM6300**: RX=GPIO4, TX=GPIO5 (9600 8N1)
- **Radio UART (ObsidiaLink)**: RX=GPIO15, TX=GPIO16 (115200 8N1)

## Процедура сборки и проверки

1. **Регрессионные тесты**:
   ```bash
   python tests/pinmap/test_obsidia_sd_pins.py
   python tests/firmware/test_no_doom.py
   cd tests/build && make && ctest --output-on-failure
   ```

2. **Сборка и прошивка Bruce**:
   ```bash
   cd /home/zawe/Obsidia/firmware/main-s3
   /home/zawe/.platformio/penv/bin/pio run -e OBSIDIA_V1
   /home/zawe/.platformio/penv/bin/python \
     /home/zawe/.platformio/packages/tool-esptoolpy/esptool.py \
     --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write_flash 0x10000 \
     .pio/build/OBSIDIA_V1/firmware.bin
   ```

3. **Проверка логов**:
   ```bash
   picocom -b 115200 /dev/ttyACM0
   ```

Перед проведением новых фиксов или изменений обязательно изучи `AGENTS.md` и запускай тестовый суит для предотвращения регрессий.
