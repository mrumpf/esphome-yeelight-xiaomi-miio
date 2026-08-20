#pragma once

#include "yeelight_front_panel.h"

namespace esphome {
namespace yeelight_front_panel {

/// Xiaomi Mijia Bedside Lamp 2 (yeelink.light.bslamp2).
///
/// I2C address 0x2C, 7 byte messages. An event is fetched by writing a fixed
/// request and reading the reply. Message layout:
///
///   [0..3] fixed prefix 04:04:01:00
///   [4]    part: 01 power button, 02 color button, 03 slider touch,
///          04 slider release
///   [5]    buttons: 01 touch, 02 release
///          slider:  position, counting down from 0x16 at slider LED 1
///   [6]    checksum
///
/// Protocol established by the esphome-xiaomi_bslamp2 project
/// (https://github.com/mmakaay/esphome-xiaomi_bslamp2). This is an independent
/// implementation of it.
class Bslamp2Model : public FrontPanelModel {
 public:
  static const uint8_t ADDRESS = 0x2C;
  static const uint8_t MESSAGE_LENGTH = 7;
  static const uint8_t SLIDER_LEDS = 10;
  static const uint8_t SLIDER_LEVELS = 22;

  const char *name() const override { return "bslamp2"; }
  uint8_t message_length() const override { return MESSAGE_LENGTH; }
  uint8_t slider_led_count() const override { return SLIDER_LEDS; }
  uint8_t slider_level_count() const override { return SLIDER_LEVELS; }
  const uint8_t *event_request() const override;
  bool parse_event(const uint8_t *message, FrontPanelEvent *event) const override;
  void encode_leds(uint16_t leds, uint8_t *message) const override;
};

/// Yeelight Staria Floor Lamp (yeelink.light.lamp10).
///
/// I2C address 0x50, 3 byte messages. Established by static analysis of the
/// original firmware: the front panel driver writes three bytes to slave 0x50
/// on I2C port 1, caching a 16 bit LED state into the last two bytes first --
/// structurally the same LED update the Bedside Lamp 2 performs.
///
/// The event message layout has NOT been captured yet, so parse_event() always
/// returns false and the hub logs the raw bytes instead. Run the lamp with
/// `debug: true` and operate the panel to collect them.
class Lamp10Model : public FrontPanelModel {
 public:
  static const uint8_t ADDRESS = 0x50;
  static const uint8_t MESSAGE_LENGTH = 3;
  /// Unverified: taken from the number of slider LEDs visible on the device.
  static const uint8_t SLIDER_LEDS = 10;
  /// Unverified: assumed identical to the other lamps in the family.
  static const uint8_t SLIDER_LEVELS = 22;
  /// Unverified: the command byte that precedes the 16 bit LED state.
  static const uint8_t LED_COMMAND = 0x02;

  const char *name() const override { return "lamp10"; }
  uint8_t message_length() const override { return MESSAGE_LENGTH; }
  uint8_t slider_led_count() const override { return SLIDER_LEDS; }
  uint8_t slider_level_count() const override { return SLIDER_LEVELS; }
  bool parse_event(const uint8_t *message, FrontPanelEvent *event) const override;
  void encode_leds(uint16_t leds, uint8_t *message) const override;
};

const FrontPanelModel *get_model(Model model);

}  // namespace yeelight_front_panel
}  // namespace esphome
