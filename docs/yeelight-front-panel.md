# Yeelight I²C front panel

Several Yeelight lamps put their power button, mode button and brightness
slider on a small microcontroller of their own rather than wiring them to the
ESP32 directly. That controller is an I²C slave, and it also drives the LEDs
that illuminate the panel. Because there is no stock ESPHome component that can
speak to it, these lamps need the `yeelight_front_panel` component.

Lamps whose front panel is a plain GPIO button (`lamp9`, most ceiling lights)
do **not** need this component — a `binary_sensor: gpio` is enough.

## How the panel works

The exchange is the same on every model found so far:

1. The panel pulls a GPIO line low when an event is waiting.
2. The ESP32 writes a fixed request message (on models that need one).
3. The ESP32 reads a message back and decodes it.

LED updates go the other way: the ESP32 writes a message containing a 16 bit
bitmask of the LEDs that should be lit.

The component keeps the interrupt handler to a single counter increment and
does all I²C work from the main loop, so a slow bus can never block the ISR.

## Supported models

### `bslamp2` — Xiaomi Mijia Bedside Lamp 2

Address `0x2C`, 7 byte messages, 10 slider LEDs, 22 slider levels.

Event request: `01:00:00:00:00:00:01`

| Byte | Meaning |
|---|---|
| 0-3 | fixed prefix `04:04:01:00` |
| 4 | part — `01` power button, `02` color button, `03` slider touch, `04` slider release |
| 5 | buttons: `01` touch, `02` release. Slider: position, counting down from `0x16` |
| 6 | checksum: `message[5] + message[4] + 1` |

LED message: `02:03:<state_hi>:<state_lo>:64:00:00`, where bits 10 and 11 of the
state must always be set, bit 14 is the power button LED, bit 12 the color
button LED, and bits 9 down to 0 are slider LEDs 1 to 10.

This protocol was originally worked out by the
[esphome-xiaomi_bslamp2](https://github.com/mmakaay/esphome-xiaomi_bslamp2)
project. The implementation here is independent of theirs.

### `lamp10` — Yeelight Staria Floor Lamp

Address `0x50`, 3 byte messages.

Established by static analysis of the original firmware: the front panel driver
writes three bytes to slave `0x50` on I²C port 1, and caches a 16 bit LED state
into the last two bytes immediately before the write — structurally the same
LED update the Bedside Lamp 2 performs. So the LED message is
`<command>:<state_hi>:<state_lo>`.

**The event message layout is not known yet.** `parse_event()` returns false
for this model, which makes the component log every message it receives:

```
[W][yeelight_front_panel]: Unrecognised message: 0A.00.02 (3)
```

Operate the panel and collect those lines to establish the format. The slider
LED count and level count in `models.h` are placeholders until then, as is the
LED command byte.

## Adding a model

Everything model specific lives behind `FrontPanelModel` in `models.h`. To add
a lamp:

1. Add a class implementing `message_length()`, `parse_event()` and
   `encode_leds()`, plus the slider LED and level counts.
2. Add it to the `Model` enum and to `get_model()`.
3. Add the name and default address to `MODELS` in `__init__.py`.

Nothing above that interface changes — the binary sensors and the slider sensor
work on the decoded `FrontPanelEvent`, never on raw bytes.

### Known but not yet implemented

The **Yeelight Staria Table Lamp** (`lamp9`) also has an I²C front panel, on a
different address again, using 7 byte messages with a `0a` prefix and 22 slider
levels. Note that `yeelight_light_lamp9.yaml` in this repository drives that
lamp's plain GPIO button instead and leaves the panel unused.
