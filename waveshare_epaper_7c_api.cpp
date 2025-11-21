#include "waveshare_epaper_7c_api.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace waveshare_epaper_7c_api {

static const char *const TAG = "waveshare_epaper_7c_api";

namespace cmddata_5P65InF {
	// WaveshareEPaper5P65InF commands
	// https://www.waveshare.com/wiki/5.65inch_e-Paper_Module_(F)

	// R61H (TRES) Resolution Setting
	// 0x258 = 600
	// 0x1C0 = 448
	static const uint8_t R61_CMD_TRES[] = {0x61, 0x02, 0x58, 0x01, 0xC0};
	
	// R10H (DTM1): Data Start Transmission 1
	static const uint8_t R10_CMD_DTM1[] = {0x10};
	
	// R04H (PON): Power ON Command
	static const uint8_t R04_CMD_PON[] = {0x04};
	
	// R12H (DRF): Display Refresh
	static const uint8_t R12_CMD_DRF[] = {0x12};

	// R02H (POF): Power OFF Command
	static const uint8_t R02_CMD_POF[] = {0x02};

	// R07H (DSLP): Deep sleep#
	// Note Documentation @  Waveshare shows cmd code as 0x10 in table, but
	// 0x10 is DTM1.
	static const uint8_t R07_CMD_DSLP[] = {0x07, 0xA5};
}  // namespace cmddata_5P65InF

size_t WaveshareEPaper5P65InFApi::push_data(int data) {
    this->data( (data >> 0) & 0xFF );
    this->data( (data >> 8) & 0xFF );
    this->data( (data >> 16) & 0xFF );
    this->data( (data >> 24) & 0xFF );
    this->total_bytes_received_ += sizeof(data);
    return sizeof(data);
}

size_t WaveshareEPaper5P65InFApi::push_data(uint8_t data) {
    this->data( (data >> 0) & 0xFF );
    this->total_bytes_received_ += sizeof(data);
    return sizeof(data);
}

void WaveshareEPaper5P65InFApi::pre_push_data_chunk() {
	using namespace cmddata_5P65InF;
	
	if(this->is_streaming_ == false) {
		this->is_streaming_ = true;
  	this->total_bytes_received_ = 0;
  	
		// INITIALIZATION
		ESP_LOGD(TAG, "Initialise the display");
		this->initialize();

		// COMMAND DATA START TRANSMISSION
		ESP_LOGD(TAG, "Sending data to the display");
		this->cmd_data(R61_CMD_TRES, sizeof(R61_CMD_TRES));
		this->cmd_data(R10_CMD_DTM1, sizeof(R10_CMD_DTM1));

  	this->display_state_ = DisplayState::RECEIVING_DATA;
	}
}

void WaveshareEPaper5P65InFApi::post_push_data_chunk() {
	  size_t expected_size = this->get_width_internal() * this->get_height_internal() /2;
	  
	  ESP_LOGD("waveshare_epaper_7c_api", "%d / %d", this->total_bytes_received_ , expected_size);
		if(this->total_bytes_received_ >= expected_size ) {
			this->is_streaming_ = false;
			this->total_bytes_received_ = -1;
			this->display_state_ = DisplayState::POWERING_ON;
		}
}

bool WaveshareEPaper5P65InFApi::is_(WaitForState busy_state) {
  return busy_state == this->busy_pin_->digital_read();
} 

void WaveshareEPaper5P65InFApi::loop() {
	using namespace cmddata_5P65InF;
  switch (this->display_state_) {
    case DisplayState::POWERING_ON:
	    ESP_LOGD(TAG, "Powering ON");
	    this->display_state_ = DisplayState::REFRESHING;
	    this->cmd_data(R04_CMD_PON, sizeof(R04_CMD_PON));
      break;
      
    case DisplayState::REFRESHING:
    	if(this->is_(IDLE)) {
		    ESP_LOGD(TAG, "Refreshing");
		    this->display_state_ = DisplayState::POWERING_OFF;
		    this->cmd_data(R12_CMD_DRF, sizeof(R12_CMD_DRF));
	    }
      break;
      
    case DisplayState::POWERING_OFF:
    	if(this->is_(IDLE)) {
      	ESP_LOGD(TAG, "Powering OFF");
		    this->display_state_ = DisplayState::DEEP_SLEEPING;
		    this->cmd_data(R02_CMD_POF, sizeof(R02_CMD_POF));
	    }
      break;
      
    case DisplayState::DEEP_SLEEPING:
	    if(this->is_(BUSY)) {
	    	ESP_LOGD(TAG, "🎯 Affichage terminé");
	    	if (this->deep_sleep_between_updates_) {
    			ESP_LOGD(TAG, "💤 Deep sleep activé");
		      this->cmd_data(R07_CMD_DSLP, sizeof(R07_CMD_DSLP));
		    }
      	this->display_state_ = DisplayState::IDLE;
    	}
      break;
      
    case DisplayState::IDLE:
    case DisplayState::RECEIVING_DATA:
      // Rien à faire
      break;
  }
}

}  // namespace waveshare_epaper_7c_api
}  // namespace esphome
