# MAX6675

[![Build](https://github.com/esp-idf-lib/max6675/actions/workflows/build.yml/badge.svg)](https://github.com/esp-idf-lib/max6675/actions/workflows/build.yml)
[![Build docs](https://github.com/esp-idf-lib/max6675/actions/workflows/build-docs.yml/badge.svg)](https://github.com/esp-idf-lib/max6675/actions/workflows/build-docs.yml)
[![Validation](https://github.com/esp-idf-lib/max6675/actions/workflows/validate-component.yml/badge.svg)](https://github.com/esp-idf-lib/max6675/actions/workflows/validate-component.yml)
[![ESP Component Registry](https://img.shields.io/badge/component%20registry-max6675-blue)](https://components.espressif.com/components/esp-idf-lib/max6675)

* [Documentation](https://esp-idf-lib.github.io/max6675/)
* [Repository](https://github.com/esp-idf-lib/max6675)
* [Issues](https://github.com/esp-idf-lib/max6675/issues)
* [Discussions](https://github.com/esp-idf-lib/core/discussions)
* [Components Registry](https://components.espressif.com/components/esp-idf-lib/max6675)

## Features

ESP-IDF driver for the MAX6675 cold-junction compensated
thermocouple-to-digital converter with a type K thermocouple.

* 12-bit temperature, 0.25 °C per LSB, 0 to +1023.75 °C
* detects an open thermocouple and refuses the frame instead of returning a
  meaningless number
* validates the frame and the arguments
* uses the SPI master driver, so the chip can share the bus with other
  devices (CS must be exclusive)

## Wiring

| MAX6675 | ESP32          | Notes                                        |
|---------|----------------|----------------------------------------------|
| 1 GND   | GND            |                                              |
| 2 T-    | thermocouple - | must be grounded for the open detector       |
| 3 T+    | thermocouple + |                                              |
| 4 VCC   | 3.3 V          | 100 nF to GND close to the chip              |
| 5 SCK   | `EXAMPLE_SCLK_GPIO` | SPI clock                               |
| 6 CS    | `EXAMPLE_CS_GPIO`   | chip select                             |
| 7 SO    | `EXAMPLE_MISO_GPIO` | data out                               |
| 8 N.C.  | -              |                                              |

The chip is read-only for the master: there is no MOSI line, and the driver
configures the bus without one.

The `T-` lead must be connected to ground as close to the GND pin as
possible, otherwise the open thermocouple detector does not work.

## Interface

There are no registers. The master pulls CS low and clocks out 16 bits, MSB
first:

| Bit     | Meaning                                                      |
|---------|--------------------------------------------------------------|
| D15     | dummy sign bit, always 0                                     |
| D14-D3  | the compensated temperature, 0.25 °C per LSB, 0 = 0 °C, 4095 = +1023.75 °C |
| D2      | 1 when the thermocouple input is open                        |
| D1      | device ID, 0 for the MAX6675                                 |
| D0      | three-state, not used                                        |

The chip also compensates the cold junction, but it does not report the cold
junction temperature: only the compensated thermocouple temperature is
available. The cold junction itself must stay between -20 and +85 °C.

## Timing

A conversion takes 170 ms typical and 220 ms maximum. Pulling CS low stops
the conversion that is in progress: the frame that follows belongs to the
previous conversion, or to the interrupted one, which is why the datasheet
asks to wait.

The driver does not wait: read a value, then wait at least
`MAX6675_CONVERSION_TIME_MS` before the next read. Reading at a higher rate
returns stale values, not garbage, but they are not the current temperature.

The clock may run at up to 4.3 MHz; 1 MHz is more than enough for a chip that
produces one 16-bit frame per 220 ms.

## Faults

When the thermocouple is open, the frame reports it in D2. The number in the
same frame is meaningless, so `max6675_read_temperature()` reports the flag
through its `open` parameter, returns `ESP_ERR_INVALID_RESPONSE` and does not
touch the temperature variable. The same
happens when the dummy sign bit is set, which usually means the wiring is
wrong or the chip is not there.

## Usage

```c
#include <max6675.h>

spi_bus_config_t bus_cfg = {
    .miso_io_num = 19,
    .mosi_io_num = -1,
    .sclk_io_num = 18,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
};
ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

max6675_t dev;
ESP_ERROR_CHECK(max6675_init_desc(&dev, SPI2_HOST, GPIO_NUM_5, 1000000));

while (true)
{
    float temperature;
    bool open;
    esp_err_t err = max6675_read_temperature(&dev, &temperature, &open);
    if (err == ESP_OK)
        ESP_LOGI(TAG, "%.2f °C", temperature);
    else if (open)
        ESP_LOGE(TAG, "The thermocouple is open");
    else
        ESP_LOGE(TAG, "Could not read: %s", esp_err_to_name(err));

    vTaskDelay(pdMS_TO_TICKS(1000));
}
```

## API

```c
esp_err_t max6675_init_desc(max6675_t *dev, spi_host_device_t host, gpio_num_t cs_pin, int clock_speed_hz);
esp_err_t max6675_free_desc(max6675_t *dev);
esp_err_t max6675_read_temperature(max6675_t *dev, float *temperature, bool *open);
```

`max6675_init_desc()` validates the host, the clock speed (up to
`MAX6675_MAX_CLOCK_SPEED_HZ`) and the CS pin, and adds the device to the bus.
`max6675_free_desc()` removes it and clears the handle, so the second call
returns `ESP_ERR_INVALID_ARG`.

## Thread safety

The SPI driver serialises the transfers, so the reads may be called from
several tasks. `max6675_free_desc()` is not protected: do not remove the
device while another task may be reading it.

## Example

See [examples/default](examples/default).

## Installation

```sh
idf.py add-dependency "esp-idf-lib/max6675^1.0.0"
```

## Support

* [Issues](https://github.com/esp-idf-lib/max6675/issues)
* [Discussions](https://github.com/esp-idf-lib/core/discussions)

## Contributing

Please see [CONTRIBUTING.md](https://github.com/esp-idf-lib/core/blob/main/CONTRIBUTING.md).
