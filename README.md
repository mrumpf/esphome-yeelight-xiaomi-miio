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

| Device | Base config | Upstream |
| --- | --- | --- |
| `mi-smart-led-desk-lamp-pro` | `vendor/xiaomi-smart-led-desk-lamp-pro-esphome/lamp.yaml` | [lucasreiners](https://github.com/lucasreiners/xiaomi-smart-led-desk-lamp-pro-esphome) |
| `yeelight-led-pro` | `vendor/esphome-yeelight-ceiling-light/yeelight_light_lamp9.yaml` | [syssi](https://github.com/syssi/esphome-yeelight-ceiling-light) |
| `yeelight-light-ceiling10` | `vendor/esphome-yeelight-ceiling-light/yeelight_light_ceiling10.yaml` | [syssi](https://github.com/syssi/esphome-yeelight-ceiling-light) |
| `mi-bedside-lamp-2` | `packages/hardware/bslamp2.yaml` | own, see below |

[mmakaay/esphome-xiaomi_bslamp2](https://github.com/mmakaay/esphome-xiaomi_bslamp2)
is vendored for reference. Its front panel protocol informed the component here,
but no code was taken from it — that project is GPLv3 and this one is not.

## What is my own

`components/yeelight_front_panel` drives the I2C touch panel found on the
Bedside Lamp 2 and the Staria Floor Lamp. Those lamps come from two different
upstream projects, so the component belongs to neither. See
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
