#include "yeelight_front_panel.h"
#include "models.h"

#include "esphome/core/log.h"

#include <algorithm>
#include <cmath>

namespace esphome {
namespace yeelight_front_panel {

static const char *const TAG = "yeelight_front_panel";

void IRAM_ATTR HOT FrontPanelTriggerStore::gpio_intr(FrontPanelTriggerStore *store) { store->event_count++; }

void YeelightFrontPanel::setup() {
  this->model_ = get_model(this->model_id_);

  this->trigger_pin_->setup();
  this->trigger_pin_->attach_interrupt(FrontPanelTriggerStore::gpio_intr, &this->store_,
                                       gpio::INTERRUPT_FALLING_EDGE);
}

void YeelightFrontPanel::dump_config() {
  ESP_LOGCONFIG(TAG, "Yeelight front panel:");
  ESP_LOGCONFIG(TAG, "  Model: %s", this->model_->name());
  ESP_LOGCONFIG(TAG, "  Message length: %u bytes", this->model_->message_length());
  ESP_LOGCONFIG(TAG, "  Slider: %u LEDs, %u levels", this->model_->slider_led_count(),
                this->model_->slider_level_count());
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Trigger pin: ", this->trigger_pin_);
}

void YeelightFrontPanel::loop() {
  const uint32_t event_count = this->store_.event_count;
  if (event_count != this->last_event_count_) {
    const uint32_t missed = event_count - this->last_event_count_ - 1;
    if (missed > 0) {
      ESP_LOGW(TAG, "Missed %u front panel event(s)", missed);
    }
    this->last_event_count_ = event_count;
    this->read_event_();
  }

  if (this->leds_dirty_) {
    this->flush_leds_();
  }
}

void YeelightFrontPanel::read_event_() {
  const uint8_t length = this->model_->message_length();

  const uint8_t *request = this->model_->event_request();
  if (request != nullptr && this->write(request, length) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Requesting the pending event failed");
    return;
  }

  uint8_t message[MAX_MESSAGE_LENGTH];
  if (this->read(message, length) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Reading the pending event failed");
    return;
  }

  if (this->debug_) {
    ESP_LOGI(TAG, "Message: %s", format_hex_pretty(message, length).c_str());
  }

  FrontPanelEvent event;
  if (!this->model_->parse_event(message, &event)) {
    ESP_LOGW(TAG, "Unrecognised message: %s", format_hex_pretty(message, length).c_str());
    return;
  }

  this->event_callback_.call(event);
}

void YeelightFrontPanel::flush_leds_() {
  uint8_t message[MAX_MESSAGE_LENGTH];
  this->model_->encode_leds(this->led_state_, message);
  if (this->write(message, this->model_->message_length()) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Updating the front panel LEDs failed");
    return;
  }
  this->leds_dirty_ = false;
}

void YeelightFrontPanel::set_slider_level(float level) {
  if (!(level > 0.0f)) {
    this->turn_off_leds(LED_ALL_SLIDER);
    return;
  }

  const uint8_t count = this->model_->slider_led_count();
  uint8_t lit = static_cast<uint8_t>(std::ceil(std::min(level, 1.0f) * count));
  if (lit < 1)
    lit = 1;

  uint16_t leds = 0;
  for (uint8_t i = 0; i < lit; i++)
    leds |= 1 << i;

  this->turn_off_leds(LED_ALL_SLIDER);
  this->turn_on_leds(leds);
}

uint8_t YeelightFrontPanel::slider_level_count() const {
  return this->model_ == nullptr ? 0 : this->model_->slider_level_count();
}

}  // namespace yeelight_front_panel
}  // namespace esphome
