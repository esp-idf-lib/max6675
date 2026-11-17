/**
 * @file max6675.c
 *
 * ESP-IDF driver for the MAX6675 cold-junction compensated
 * thermocouple-to-digital converter.
 *
 * Copyright (c) 2026 Ruslan V. Uss <unclerus@gmail.com>
 *
 * MIT Licensed as described in the file LICENSE
 */
#include <esp_log.h>
#include <esp_idf_version.h>

#include "max6675.h"

static const char *TAG = "max6675";

#define BIT_DUMMY_SIGN  (1u << 15)
#define BIT_OPEN_TC     (1u << 2)
#define TEMPERATURE_SHIFT 3

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

esp_err_t max6675_init_desc(max6675_t *dev, spi_host_device_t host, gpio_num_t cs_pin, int clock_speed_hz)
{
    CHECK_ARG(dev);
    CHECK_ARG(host >= SPI1_HOST && host < SPI_HOST_MAX);
    CHECK_ARG(clock_speed_hz > 0 && clock_speed_hz <= MAX6675_MAX_CLOCK_SPEED_HZ);
    CHECK_ARG(GPIO_IS_VALID_OUTPUT_GPIO(cs_pin));

    spi_device_interface_config_t cfg =
    {
        // the chip samples the data on the falling edge of SCK, which is
        // SPI mode 0
        .mode = 0,
        .clock_speed_hz = clock_speed_hz,
        .spics_io_num = cs_pin,
        .queue_size = 1,
        // the chip needs 100 ns between the CS edge and the first clock
        .cs_ena_pretrans = 1
    };

    return spi_bus_add_device(host, &cfg, &dev->spi_dev);
}

esp_err_t max6675_free_desc(max6675_t *dev)
{
    CHECK_ARG(dev && dev->spi_dev);

    CHECK(spi_bus_remove_device(dev->spi_dev));
    dev->spi_dev = NULL;

    return ESP_OK;
}

esp_err_t max6675_read_temperature(max6675_t *dev, float *temperature, bool *open)
{
    CHECK_ARG(dev && dev->spi_dev);
    CHECK_ARG(temperature && open);

    // the frame comes out MSB first, and the SPI driver stores the bytes in
    // the order they arrive, so the two bytes have to be combined by hand
    uint8_t frame[2];
    spi_transaction_t t =
    {
        .length = 16,
        .rxlength = 16,
        .rx_buffer = frame
    };

    CHECK(spi_device_transmit(dev->spi_dev, &t));

    uint16_t raw = (uint16_t)frame[0] << 8 | frame[1];

    *open = (raw & BIT_OPEN_TC) != 0;

    if (raw & BIT_DUMMY_SIGN)
    {
        ESP_LOGE(TAG, "The frame is not valid: the dummy bit is set (0x%04x)", raw);
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (*open)
    {
        ESP_LOGE(TAG, "The thermocouple is open");
        return ESP_ERR_INVALID_RESPONSE;
    }

    *temperature = (float)(raw >> TEMPERATURE_SHIFT) * MAX6675_RESOLUTION;

    return ESP_OK;
}
