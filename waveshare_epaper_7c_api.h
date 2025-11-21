#pragma once

#include <esphome/components/waveshare_epaper/waveshare_epaper.h>

// Utiliser une déclaration forward pour éviter les problèmes d'inclusion
namespace esphome {
namespace waveshare_epaper {
    class WaveshareEPaper5P65InF;
}  // namespace waveshare_epaper
}  // namespace esphome

namespace esphome {
namespace waveshare_epaper_7c_api {

class WaveshareEPaper5P65InFApi : public waveshare_epaper::WaveshareEPaper5P65InF {
 public:

	void loop() override;

  // Méthode pour les services API
  template <typename T> size_t push_data_chunk(std::vector<T> &data) {
  	if(
  		display_state_ != DisplayState::IDLE &&
  	  display_state_ != DisplayState::RECEIVING_DATA &&
  	  display_state_ != DisplayState::DEEP_SLEEPING) {
  	  
      	ESP_LOGD("waveshare_epaper_7c_api", "Display current state %d != idle, receiving data or deepsleep", display_state_);
  			return -1;
  	}

  	size_t write = 0;
  	this->pre_push_data_chunk();
		for (size_t i = 0; i < data.size(); i++) {
			write += push_data(data[i]);
		}
		this->post_push_data_chunk();
  	return write;
  };
  
	size_t push_data_chunk(uint8_t *data, size_t size) {
  	if(
  		display_state_ != DisplayState::IDLE &&
  	  display_state_ != DisplayState::RECEIVING_DATA &&
  	  display_state_ != DisplayState::DEEP_SLEEPING) {
  	  
      	ESP_LOGD("waveshare_epaper_7c_api", "Display current state %d != idle, receiving data or deepsleep", display_state_);
  			return -1;
  	}

  	size_t write = 0;
  	this->pre_push_data_chunk();
		for (size_t i = 0; i < size; i++) {
			write += push_data(data[i]);
		}
		this->post_push_data_chunk();
  	return write;
  };

 protected:
  bool is_streaming_{false};
  size_t total_bytes_received_{0};
 	size_t push_data(long int data);
 	size_t push_data(int data);
	size_t push_data(uint8_t data);
	void pre_push_data_chunk();
	void post_push_data_chunk();
	bool is_(WaitForState busy_state);
	
  enum class DisplayState {
    IDLE,
    RECEIVING_DATA,
    POWERING_ON,
    REFRESHING, 
    POWERING_OFF,
    DEEP_SLEEPING
  };
  
  DisplayState display_state_ = DisplayState::IDLE;
};

}  // namespace waveshare_epaper_7c_api
}  // namespace esphome
