/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
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

#include "proc_btlerx.hpp"
#include "portapack_shared_memory.hpp"

#include "event_m4.hpp"

void BTLERxProcessor::execute(const buffer_c8_t& buffer) {
	// This is called at 3072000 / 2048 = 1500Hz

	if (!configured) return;
	
	// FM demodulation
	/*const auto decim_0_out = decim_0.execute(buffer, dst_buffer);				// 2048 / 8 = 256 (512 I/Q samples)
	const auto decim_1_out = decim_1.execute(decim_0_out, dst_buffer);			// 256 / 8 = 32 (64 I/Q samples)
	const auto channel_out = channel_filter.execute(decim_1_out, dst_buffer);	// 32 / 2 = 16 (32 I/Q samples)*/

	const auto decim_0_out = decim_0.execute(buffer, dst_buffer);
	const auto channel = decim_1.execute(decim_0_out, dst_buffer);

	feed_channel_stats(channel);
	
	auto audio_oversampled = demod.execute(channel, work_audio_buffer);
	
	//audio_output.write(audio_oversampled);
	// Audio signal processing
	for (size_t c = 0; c < audio_oversampled.count; c++) {
		const int32_t sample_int = audio_oversampled.p[c] * 32768.0f;
		int32_t current_sample = __SSAT(sample_int, 16);
		int result;
		current_sample /= 128;
                
                rb_head++;
	        rb_head=(rb_head)%RB_SIZE;

                rb_buf[rb_head] = current_sample;

		skipSamples = skipSamples - 1;


		if (skipSamples<1)
		{
				

			int32_t threshold_tmp=0;
			for (int c=0;c<8;c++)
			{
				threshold_tmp = threshold_tmp + (int32_t)rb_buf[(rb_head+c)%RB_SIZE];
			}
			g_threshold = (int32_t)threshold_tmp/8;

				
			int transitions=0;
			if (rb_buf[(rb_head+9)%RB_SIZE] > g_threshold)
			{
				for (int c=0;c<8;c++)
				{
					if (rb_buf[(rb_head + c)%RB_SIZE] > rb_buf[(rb_head + c + 1)%RB_SIZE])
					transitions = transitions + 1;
				}
			}
			else
			{
				for (int c=0;c<8;c++)
				{
					if (rb_buf[(rb_head + c)%RB_SIZE] < rb_buf[(rb_head + c + 1)%RB_SIZE])
					transitions = transitions + 1;
				}
			}
			
			bool packet_detected=false;
			if ( transitions==4 && abs(g_threshold)<15500)
			{

				
				uint8_t packet_data[500];
				int packet_length;
				uint32_t packet_crc;
				uint32_t calced_crc;
				uint64_t packet_addr_l;
				uint32_t result;
				uint8_t crc[3];
				uint8_t packet_header_arr[2];

				packet_addr_l=0;
				for (int i=0;i<4;i++) 
				{                   
				    bool current_bit;
				    uint8_t byte=0;
				    for (int c=0;c<8;c++)
				    {
				        if (rb_buf[(rb_head + (i+1)*8 + c)%RB_SIZE] > g_threshold)
				            current_bit = true;
				        else
				            current_bit = false;
				        byte |= current_bit << (7-c);
				    }
				    uint8_t byte_temp = (uint8_t) (((byte * 0x0802LU & 0x22110LU) | (byte * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
				    packet_addr_l|=((uint64_t)byte_temp)<<(8*i);
				}

				channel_number = 38;

				
				for (int t=0;t<2;t++)
				{
				    bool current_bit;
				    uint8_t byte=0;
				    for (int c=0;c<8;c++)
				    {
				        if (rb_buf[(rb_head + 5*8+t*8 + c)%RB_SIZE] > g_threshold)
				            current_bit = true;
				        else
				            current_bit = false;
				        byte |= current_bit << (7-c);
				    }

				    packet_header_arr[t] = byte;
				}

				

				uint8_t byte_temp2 = (uint8_t) (((channel_number * 0x0802LU & 0x22110LU) | (channel_number * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
				uint8_t lfsr_1 = byte_temp2 | 2;
				int header_length = 2;
				int header_counter = 0;
				while(header_length--)
				{
				    for(uint8_t i = 0x80; i; i >>= 1)
				    {
				        if(lfsr_1 & 0x80)
				        {
				            lfsr_1 ^= 0x11;
				            (packet_header_arr[header_counter]) ^= i;
				        }
				        lfsr_1 <<= 1;
				    }
				    header_counter = header_counter + 1;
				}

				

				if (packet_addr_l==0x8E89BED6)
				{  

				    uint8_t byte_temp3 = (uint8_t) (((packet_header_arr[1] * 0x0802LU & 0x22110LU) | (packet_header_arr[1] * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
				    packet_length=byte_temp3&0x3F;


				    /*data_message.is_data = true;
				    data_message.value = packet_length;
				    shared_memory.application_queue.push(data_message);*/
				
				} 
				else 
				{
				    packet_length=0;
				}

				/*phase += 1;
				if (phase >= 1000) 
				{
						phase = 0;		
						result = packet_addr_l;
						data_message.is_data = true;
						data_message.value = result >> 24;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = result >> 16;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = result >> 8;
						shared_memory.application_queue.push(data_message);	
						data_message.is_data = true;
						data_message.value = result;
						shared_memory.application_queue.push(data_message);	
				}*/

				for (int t=0;t<packet_length+2+3;t++)
				{
				    bool current_bit;
				    uint8_t byte=0;
				    for (int c=0;c<8;c++)
				    {
				        if (rb_buf[(rb_head + 5*8+t*8 + c)%RB_SIZE] > g_threshold)
				            current_bit = true;
				        else
				            current_bit = false;
				        byte |= current_bit << (7-c);
				    }

				    packet_data[t] = byte;
				}

				

				uint8_t byte_temp4 = (uint8_t) (((channel_number * 0x0802LU & 0x22110LU) | (channel_number * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
				uint8_t lfsr_2 = byte_temp4 | 2;
				int pdu_crc_length = packet_length+2+3;
				int pdu_crc_counter = 0;
				while(pdu_crc_length--)
				{
				    for(uint8_t i = 0x80; i; i >>= 1)
				    {
				        if(lfsr_2 & 0x80)
				        {
				            lfsr_2 ^= 0x11;
				            (packet_data[pdu_crc_counter]) ^= i;
				        }
				        lfsr_2 <<= 1;
				    }
				    pdu_crc_counter = pdu_crc_counter + 1;
				}

				

				if (packet_addr_l==0x8E89BED6)
				{  
				    crc[0]=crc[1]=crc[2]=0x55;
				}
				else 
				{
				    crc[0]=crc[1]=crc[2]=0;
				}

				uint8_t v, t, d, crc_length;
				uint32_t crc_result=0;
				crc_length = packet_length + 2;
				int counter = 0;
				while(crc_length--)
				{
				    uint8_t byte_temp5 = (uint8_t) (((packet_data[counter] * 0x0802LU & 0x22110LU) | (packet_data[counter] * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
				    d = byte_temp5;
				    for(v = 0; v < 8; v++, d >>= 1)
				    {
				        t = crc[0] >> 7;
				        crc[0] <<= 1;
				        if(crc[1] & 0x80) crc[0] |= 1;
				        crc[1] <<= 1;
				        if(crc[2] & 0x80) crc[1] |= 1;
				        crc[2] <<= 1;
				        if(t != (d & 1))
				        {
				            crc[2] ^= 0x5B;
				            crc[1] ^= 0x06;
				        }
				    }
				    counter = counter + 1;
				}
				for (v=0;v<3;v++) crc_result=(crc_result<<8)|crc[v];
				calced_crc = crc_result;

				packet_crc=0;
				for (int c=0;c<3;c++) packet_crc=(packet_crc<<8)|packet_data[packet_length+2+c];

				
				/*if (packet_addr_l==0x8E89BED6)
				{  
						data_message.is_data = true;
						data_message.value = calced_crc>>24;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = calced_crc>>16;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = calced_crc>>8;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = calced_crc;
						shared_memory.application_queue.push(data_message);

						data_message.is_data = true;
						data_message.value = packet_crc>>24;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = packet_crc>>16;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = packet_crc>>8;
						shared_memory.application_queue.push(data_message);
						data_message.is_data = true;
						data_message.value = packet_crc;
						shared_memory.application_queue.push(data_message);

				}*/
				/*if (packet_crc==calced_crc)
				{
				    uint8_t mac_data[6];
				    int counter = 0;
				    for (int i = 7; i >= 2; i--) 
				    {
				        uint8_t byte_temp6 = (uint8_t) (((packet_data[i] * 0x0802LU & 0x22110LU) | (packet_data[i] * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
					//result = byte_temp6;
					mac_data[counter] = byte_temp6;
					counter = counter + 1;
					
				    }
				    

				    packet_detected = true;
				} */

				if (packet_addr_l==0x8E89BED6)
				{
				    uint8_t mac_data[6];
				    int counter = 0;
				    for (int i = 7; i >= 2; i--) 
				    {
				        uint8_t byte_temp6 = (uint8_t) (((packet_data[i] * 0x0802LU & 0x22110LU) | (packet_data[i] * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
					//result = byte_temp6;
					mac_data[counter] = byte_temp6;
					counter = counter + 1;
				    }

				    data_message.is_data = false;
			            data_message.value = 'A';
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[0];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[1];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[2];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[3];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[4];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = true;
			            data_message.value = mac_data[5];
				    shared_memory.application_queue.push(data_message);

				    data_message.is_data = false;
			            data_message.value = 'B';
				    shared_memory.application_queue.push(data_message);




				    packet_detected = true;
				}
				else
				    packet_detected = false;
			}

			if (packet_detected) 
			{
				skipSamples=20;
			}
		}
		/*sample_bits <<= 1;
		sample_bits = (current_sample) ? 1 : 0;*/
		
		
/*
		phase += 1;

		if (phase >= 1000) 
		{
			phase = 0;
			//word_bits <<= 1;
			//word_bits |= (sample_bits & 1);
					
			data_message.is_data = true;
			data_message.value = channel_number;
			shared_memory.application_queue.push(data_message);	
		}*/
	}
}

void BTLERxProcessor::on_message(const Message* const message) {
	if (message->id == Message::ID::BTLERxConfigure)
		configure(*reinterpret_cast<const BTLERxConfigureMessage*>(message));
}

void BTLERxProcessor::configure(const BTLERxConfigureMessage& message) {	
	decim_0.configure(taps_200k_wfm_decim_0.taps, 33554432);
	decim_1.configure(taps_200k_wfm_decim_1.taps, 131072);
	demod.configure(audio_fs, 5000);

	audio_output.configure(audio_24k_hpf_300hz_config, audio_24k_deemph_300_6_config, 0);
	
	phase = 0;

	triggered = false;
	state = WAIT_START;
	
	configured = true;
}

int main() {
	EventDispatcher event_dispatcher { std::make_unique<BTLERxProcessor>() };
	event_dispatcher.run();
	return 0;
}
