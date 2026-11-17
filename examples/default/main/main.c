/*
 * The example reads the thermocouple temperature once per second and prints
 * it. The chip needs no configuration: the driver only has to be told which
 * pins the bus uses.
 *
 * MIT Licensed as described in the file LICENSE
 */
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>

#include <max6675.h>

#define PIN_SO    CONFIG_EXAMPLE_MISO_GPIO
#define PIN_SCK   CONFIG_EXAMPLE_SCLK_GPIO
#define PIN_CS    CONFIG_EXAMPLE_CS_GPIO

static const char *TAG = "example";

void app_main()
{
    // The chip is write-only for the master: there is no MOSI line, so the
    // bus is configured without one
    spi_bus_config_t bus_cfg =
    {
        .miso_io_num = PIN_SO,
        .mosi_io_num = -1,
        .sclk_io_num = PIN_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    max6675_t dev;
    ESP_ERROR_CHECK(max6675_init_desc(&dev, SPI2_HOST, PIN_CS, 1000000));

    while (true)
    {
        float temperature;
        bool open;
        esp_err_t err = max6675_read_temperature(&dev, &temperature, &open);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);
        else if (open)
            ESP_LOGE(TAG, "The thermocouple is open");
        else
            ESP_LOGE(TAG, "Could not read the temperature: %s", esp_err_to_name(err));

        // the chip needs 220 ms to convert, otherwise the next frame holds
        // the result of the interrupted conversion
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
