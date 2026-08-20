# esphome-yeelight-xiaomi-miio

ESPHome configurations for the Xiaomi / Yeelight lamps in my home, built on top
of the upstream projects rather than copies of them.

## How it fits together

```
device.yaml            one file per physical device, named after the device
   ├── base            hardware definition, usually from a submodule
   └── common          packages/common.yaml — settings shared by every device
```

Upstream projects are referenced as git submodules under `vendor/`, pointing at
my own forks so nothing is lost if an upstream disappears. Each submodule is
pinned to an exact commit, so builds are reproducible and upstream cannot shift
underneath a device. Nothing upstream is modified here; fixes go to the fork,
and from there upstream as a pull request.

Referencing rather than vendoring also keeps the licences intact: the projects
below are variously Apache-2.0, GPLv3 and unlicensed, and copying them into one
repository would not be possible.

| Device | Base config | Upstream | State |
| --- | --- | --- | --- |
| `mi-smart-led-desk-lamp-pro` | `vendor/xiaomi-smart-led-desk-lamp-pro-esphome/lamp.yaml` | [lucasreiners](https://github.com/lucasreiners/xiaomi-smart-led-desk-lamp-pro-esphome) | flashed |
| `yeelight-led-pro` | `vendor/esphome-yeelight-ceiling-light/yeelight_light_lamp9_pro.yaml` | [syssi](https://github.com/syssi/esphome-yeelight-ceiling-light) | flashed |
| `mi-bedside-lamp-2` | `vendor/esphome-xiaomi_bslamp2/packages/core.yaml` | [mmakaay](https://github.com/mmakaay/esphome-xiaomi_bslamp2) | flashed |
| `yeelight-meteorite-ceiling-light` | `vendor/esphome-yeelight-ceiling-light/yeelight_light_ceiling10.yaml` | [syssi](https://github.com/syssi/esphome-yeelight-ceiling-light) | not opened yet |
| `yeelight-galaxy-ceiling-light-480` | none — no upstream supports YLXD17YL | — | scaffold, pin map unknown |
| `smart-desk-lamp-1s-{gaestezimmer,gaestekueche,schlafzimmer}` | `packages/hardware/desk-lamp-1s.yaml` | GPIO map from the [Tasmota template](https://templates.blakadder.com/xiaomi_MJTD01SYL.html) | not flashed yet |

The Bedside Lamp 2 is driven by
[mmakaay/esphome-xiaomi_bslamp2](https://github.com/mmakaay/esphome-xiaomi_bslamp2)
from the submodule: its HAL comes with the behaviour that acts on the front
panel events, which is the half that matters in daily use.

## What is my own

`components/yeelight_front_panel` drives the I2C touch panel on the Staria LED
Floor Lamp (YLLD01YL), which no upstream project supports. Its `bslamp2` model
is kept as a verified reference — decoding was confirmed against the real panel,
and the floor lamp's protocol is the closest analogue. See
[docs/yeelight-front-panel.md](docs/yeelight-front-panel.md).

`packages/common.yaml` carries what every device here needs: a fallback access
point with captive portal, an OTA password and API encryption. Values live in
`secrets.yaml`, which is per machine and never committed.

## Writing a device config

```yaml
substitutions:
  name: my-lamp

packages:
  base: !include vendor/<project>/<config>.yaml
  common: !include packages/common.yaml
```

Keep `common` last. Packages are merged in order and the later one wins, which
is what lets `common.yaml` override an upstream's hardcoded OTA password.

## Setup

```
git clone --recurse-submodules https://github.com/mrumpf/esphome-yeelight-xiaomi-miio.git
cp secrets.yaml.example secrets.yaml   # then fill it in
esphome run <device>.yaml
```

To pull upstream changes into a submodule deliberately:

```
git -C vendor/<project> pull --ff-only
git add vendor/<project> && git commit -m "Bump <project>"
```
