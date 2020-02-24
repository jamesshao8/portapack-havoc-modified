/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
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

#include "proc_am_tv.hpp"

#include "portapack_shared_memory.hpp"
//#include "audio_output.hpp"
#include "dsp_fft.hpp"
#include "event_m4.hpp"

#include <cstdint>

void WidebandFMAudio::execute(const buffer_c8_t& buffer) {
	if( !configured ) {
		return;
	}
	
	const auto decim_0_out = decim_0.execute(buffer, dst_buffer);
	const auto channel = decim_1.execute(decim_0_out, dst_buffer);

	// TODO: Feed channel_stats post-decimation data?
	feed_channel_stats(channel);

	spectrum_samples += channel.count;
	if( spectrum_samples >= spectrum_interval_samples ) {
		spectrum_samples -= spectrum_interval_samples;
		channel_spectrum.feed(channel, channel_filter_pass_f, channel_filter_stop_f);
	}
/*
	spectrum_samples += channel.count;
	if( spectrum_samples >= spectrum_interval_samples ) {
		spectrum_samples -= spectrum_interval_samples;
		channel_spectrum.feed(channel, channel_filter_pass_f, channel_filter_stop_f);
	}*/

/*
	const void* src_p = channel.p;
	const auto src_end = &channel.p[channel.count];
	void* dst_p = work_audio_buffer.p;
	while(src_p < src_end) 
        {

		const uint32_t s0 = *__SIMD32(src_p)++;
		const uint32_t s1 = *__SIMD32(src_p)++;

                const uint32_t t0 = __SMUAD(s0, s0);
		const uint32_t t1 = __SMUAD(s1, s1);

		*__SIMD32(dst_p)++ = __builtin_sqrtf(t0) + __builtin_sqrtf(t1);
	}
*/

        int8_t re, im;
	int8_t mag;

        for (size_t i = 0; i < 128; i++) 
	{
		re = buffer.p[i].real();
		im = buffer.p[i].imag();
		mag = __builtin_sqrtf((re * re) + (im * im)) ;

		//const unsigned int v =  (float)(mag) + 127.0f; //am demodulated
		const unsigned int v =  re + 127.0f; //timescope

		spectrum.db[i] = std::max(0U, std::min(255U, v));
	}


	AudioSpectrumMessage message { &spectrum };
	shared_memory.application_queue.push(message);
	
	
}

void WidebandFMAudio::on_message(const Message* const message) {
	switch(message->id) {
	case Message::ID::UpdateSpectrum:
	case Message::ID::SpectrumStreamingConfig:
		channel_spectrum.on_message(message);
		break;

	case Message::ID::WFMConfigure:
		configure(*reinterpret_cast<const WFMConfigureMessage*>(message));
		break;
		
	default:
		break;
	}
}

void WidebandFMAudio::configure(const WFMConfigureMessage& message) {
	constexpr size_t decim_0_input_fs = baseband_fs;
	constexpr size_t decim_0_output_fs = decim_0_input_fs / decim_0.decimation_factor;

	constexpr size_t decim_1_input_fs = decim_0_output_fs;
	constexpr size_t decim_1_output_fs = decim_1_input_fs / decim_1.decimation_factor;

	constexpr size_t demod_input_fs = decim_1_output_fs;

	//spectrum_interval_samples = decim_1_output_fs / spectrum_rate_hz;
	spectrum_interval_samples = 80000;
	spectrum_samples = 0;

	decim_0.configure(message.decim_0_filter.taps, 33554432);
	decim_1.configure(message.decim_1_filter.taps, 131072);
	channel_filter_pass_f = message.decim_1_filter.pass_frequency_normalized * decim_1_input_fs; //*10 spectrum will be good
	channel_filter_stop_f = message.decim_1_filter.stop_frequency_normalized * decim_1_input_fs; //timescope will be wrong, *1 vice versa
	channel_spectrum.set_decimation_factor(1);

	configured = true;
}


int main() {
	EventDispatcher event_dispatcher { std::make_unique<WidebandFMAudio>() };
	event_dispatcher.run();
	return 0;
}
