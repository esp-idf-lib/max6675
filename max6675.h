/**
 * @file max6675.h
 *
 * @defgroup max6675 max6675
 * @{
 *
 * ESP-IDF driver for the MAX6675 cold-junction compensated
 * thermocouple-to-digital converter.
 *
 * The chip converts the signal of a type K thermocouple into a 12-bit
 * value with a resolution of 0.25 &deg;C and reports it through a read-only
 * SPI-compatible interface. The interface needs three wires: SCK, SO and CS.
 * There is no register access, no configuration and no way to read the cold
 * junction separately: every frame contains only the compensated
 * thermocouple temperature.
 *
 * The conversion takes 170 ms typical and 220 ms maximum, and pulling CS
 * low stops the conversion that is in progress, so the chip must be left
 * alone between the reads. The driver reports the raw conversion, it does
 * not wait: a new read sooner than 220 ms after the previous one returns the
 * result of an older conversion, or the result of the conversion that was
 * interrupted.
 *
 * The temperature is measured by the thermocouple, so the range is 0 to
 * +1023.75 &deg;C, while the cold junction, the ambient temperature of the
 * board, must stay within -20 to +85 &deg;C.
 *
 * Copyright (c) 2026 Ruslan V. Uss <unclerus@gmail.com>
 *
 * MIT Licensed as described in the file LICENSE
 */
#ifndef __MAX6675_H__
#define __MAX6675_H__

#include <stdbool.h>
#include <stdint.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The highest clock frequency the chip supports, 4.3 MHz
 */
#define MAX6675_MAX_CLOCK_SPEED_HZ 4300000

/**
 * The maximum conversion time, in milliseconds
 */
#define MAX6675_CONVERSION_TIME_MS 220

/**
 * The temperature of one LSB of the frame, in degrees Celsius
 */
#define MAX6675_RESOLUTION 0.25f

/**
 * Device descriptor
 */
typedef struct
{
    spi_device_handle_t spi_dev; //!< SPI device handle
} max6675_t;

/**
 * @brief Initialize the device descriptor
 *
 * Connects the chip to the SPI bus. SCK and SO are configured by the SPI
 * driver, the CS pin is driven by the driver as well, so it must not be
 * shared with another device on the same bus.
 *
 * The CS pin is set up by this function, so the SPI bus must exist already.
 *
 * @param dev Device descriptor
 * @param host SPI host
 * @param cs_pin GPIO pin connected to CS
 * @param clock_speed_hz SPI clock frequency, 1 MHz is enough and
 *                       MAX6675_MAX_CLOCK_SPEED_HZ is the limit
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if any of the arguments is invalid
 *      - ESP_ERR_NO_MEM if the SPI device could not be allocated
 *      - another error code from the SPI driver
 */
esp_err_t max6675_init_desc(max6675_t *dev, spi_host_device_t host, gpio_num_t cs_pin, int clock_speed_hz);

/**
 * @brief Free the device descriptor
 *
 * Removes the device from the SPI bus. The descriptor is not freed, it
 * belongs to the caller.
 *
 * @param dev Device descriptor
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if the descriptor is NULL or the device is
 *        already removed
 *      - another error code from the SPI driver
 */
esp_err_t max6675_free_desc(max6675_t *dev);

/**
 * @brief Read the thermocouple temperature
 *
 * Reads one 16-bit frame and converts it. Pulling CS low interrupts the
 * conversion that may be in progress, so call this function no more often
 * than once per MAX6675_CONVERSION_TIME_MS.
 *
 * The open thermocouple flag is always reported, so that the caller can tell
 * a broken thermocouple from a broken bus. The temperature is not written
 * when the flag is set: the number in that frame is meaningless.
 *
 * @param dev Device descriptor
 * @param temperature Pointer to store the temperature, in degrees Celsius
 * @param open Pointer to store the open thermocouple flag
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if the descriptor is NULL or not initialized,
 *        or any of the pointers is NULL
 *      - ESP_ERR_INVALID_RESPONSE if the frame is not a valid one: the dummy
 *        sign bit is set or the thermocouple is open
 *      - another error code from the SPI driver
 */
esp_err_t max6675_read_temperature(max6675_t *dev, float *temperature, bool *open);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __MAX6675_H__ */
