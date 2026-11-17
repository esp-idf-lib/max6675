# MAX6675 example

Reads the thermocouple temperature once per second and prints it.

## Wiring

| MAX6675 | ESP32                 |
|---------|-----------------------|
| 1 GND   | GND                   |
| 2 T-    | thermocouple -, GND   |
| 3 T+    | thermocouple +        |
| 4 VCC   | 3.3 V                 |
| 5 SCK   | `EXAMPLE_SCLK_GPIO`   |
| 6 CS    | `EXAMPLE_CS_GPIO`     |
| 7 SO    | `EXAMPLE_MISO_GPIO`   |
| 8 N.C.  | -                     |

`T-` must be grounded for the open thermocouple detector to work, and the
ground must be as close to the GND pin as possible.

## Configuration

The pins are set in `idf.py menuconfig` -> `Example configuration`:

| Option                | Default                        |
|-----------------------|--------------------------------|
| `EXAMPLE_MISO_GPIO`   | 19 (5 on the RISC-V targets)   |
| `EXAMPLE_SCLK_GPIO`   | 18 (6 on the RISC-V targets)   |
| `EXAMPLE_CS_GPIO`     | 5 (7 on the RISC-V targets)    |

## Build and run

```sh
idf.py set-target esp32
idf.py build flash monitor
```

## Expected output

```
I (312) example: Temperature: 24.75 °C
I (1312) example: Temperature: 24.75 °C
```

When the thermocouple is open, the driver reports `ESP_ERR_INVALID_RESPONSE`
and prints the error instead of a temperature:

```
E (312) example: Could not read the temperature: ESP_ERR_INVALID_RESPONSE
```

## Notes

* the chip converts in 170 ms typical and 220 ms maximum, the example waits
  a second between the reads;
* a type K thermocouple is required: the chip compensates for type K only;
* the cold junction, the board itself, must stay between -20 and +85 °C.
