/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "tv_collector.hpp"

#include "dsp_fft.hpp"

#include "utility.hpp"
#include "event_m4.hpp"
#include "portapack_shared_memory.hpp"

#include "event_m4.hpp"

#include <algorithm>

void TvCollector::on_message(const Message* const message) {
	switch(message->id) {
	case Message::ID::UpdateSpectrum:
		update();
		break;

	case Message::ID::SpectrumStreamingConfig:
		set_state(*reinterpret_cast<const SpectrumStreamingConfigMessage*>(message));
		break;

	default:
		break;
	}
}

void TvCollector::set_state(const SpectrumStreamingConfigMessage& message) {
	if( message.mode == SpectrumStreamingConfigMessage::Mode::Running ) {
		start();
	} else {
		stop();
	}
}

void TvCollector::start() {
	streaming = true;
	ChannelSpectrumConfigMessage message { &fifo };
	shared_memory.application_queue.push(message);
}

void TvCollector::stop() {
	streaming = false;
	fifo.reset_in();
}

void TvCollector::set_decimation_factor(
	const size_t decimation_factor
) {
	channel_spectrum_decimator.set_factor(decimation_factor);
}

/* TODO: Refactor to register task with idle thread?
 * It's sad that the idle thread has to call all the way back here just to
 * perform the deferred task on the buffer of data we prepared.
 */

void TvCollector::feed(
	const buffer_c16_t& channel,
	const uint32_t filter_pass_frequency,
	const uint32_t filter_stop_frequency
) {
	// Called from baseband processing thread.
	channel_filter_pass_frequency = filter_pass_frequency;
	channel_filter_stop_frequency = filter_stop_frequency;

	channel_spectrum_decimator.feed(channel,[this](const buffer_c16_t& data) {this->post_message(data);});
}

void TvCollector::post_message(const buffer_c16_t& data) {
	// Called from baseband processing thread.
        float re, im;
	float mag;
        float max;
	if( streaming && !channel_spectrum_request_update ) {
 		/*for(size_t i=0; i<128; i++) 
		{
			const auto s = data.p[i];
                        re = (float)(data.p[i].real());
			im = (float)(data.p[i].imag());
			mag = __builtin_sqrtf((re * re) + (im * im)) ;
                        channel_spectrum[i] = {mag, mag};
		}
                for(size_t i=128; i<256; i++) 
		{
			mag = 0 ;
                        channel_spectrum[i] = {mag, mag};
		}*/

                for(size_t i=0; i<256; i++) 
		{
			const auto s = data.p[i];
                        re = (float)(data.p[i].real());
			im = (float)(data.p[i].imag());
			mag = __builtin_sqrtf((re * re) + (im * im)) ;
			/*if (mag > max)
			{
				max = mag;
			}
			double result = (255 - 255 *(mag/ blackLevel) );
			channel_spectrum[i] = {result, result};*/
			channel_spectrum[i] = {mag, mag};
		}
		//maxSignalLevel = maxSignalLevel * 0.9 + max * 0.1;
		//blackLevel = maxSignalLevel * 0.4;
                channel_spectrum_sampling_rate = data.sampling_rate;
		channel_spectrum_request_update = true;
		EventDispatcher::events_flag(EVT_MASK_SPECTRUM);
	}
}


void TvCollector::update() {
	// Called from idle thread (after EVT_MASK_SPECTRUM is flagged)
	if( streaming && channel_spectrum_request_update ) {
		ChannelSpectrum spectrum;
		spectrum.sampling_rate = channel_spectrum_sampling_rate;
		spectrum.channel_filter_pass_frequency = channel_filter_pass_frequency;
		spectrum.channel_filter_stop_frequency = channel_filter_stop_frequency;
		for(size_t i=0; i<spectrum.db.size(); i++) {
                        const auto corrected_sample = channel_spectrum[i].real();
			//const auto mag2 = magnitude_squared(corrected_sample * (1.0f / 32768.0f));
			//const float db = mag2_to_dbv_norm(mag2);
			//constexpr float mag_scale = 5.0f;
			const unsigned int v = corrected_sample + 127.0f;
			//const unsigned int v = 255.0f - (corrected_sample + 127.0f); //maxSignalLevel 255.0f
                        
                        //const unsigned int v = corrected_sample ;
			spectrum.db[i] = std::max(0U, std::min(255U, v));
		}
		fifo.in(spectrum);
	}

	channel_spectrum_request_update = false;
}
