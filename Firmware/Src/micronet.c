#include "micronet.h"

MicronetFrameTypeDef micronet_response;
MicronetFrameTypeDef micronet_request;
micronet_address_t   micronet_address;
uint8_t              device_descriptor[30];
uint8_t							 device_descriptor_length = 0;
uint16_t 						 micronet_find_delay;
const uint8_t        micronet_preamble[4] = {0x55, 0xAA, 0x78, 0x92};

uint16_t micronet_crc = 0xFFFF;

void random_delay()
{
	  for(unsigned int i=0;i<micronet_find_delay;i++)
		for(unsigned long d=0;d<10000;d++);
}

void crc16_update(uint8_t a)
{
    int i;
    micronet_crc ^= a;   
    for (i = 0; i < 8; ++i){
        if (micronet_crc & 1) micronet_crc = (micronet_crc >> 1) ^ 0xA001;
        else micronet_crc = (micronet_crc >> 1);
    }
}

void micronet_send(uint8_t b)
{
		crc16_update(b);
		micronet_send_byte(b);
}

void micronet_set_descriptor(char descriptor[])
{
	 device_descriptor_length = 0;
	 for(int i=0;i<30;i++){
			if(descriptor[i]>31 && descriptor[i]<127){
					device_descriptor[i] = descriptor[i];
				  device_descriptor_length = i+1;
			}else{
				i = 30;
			}
	 }
}

void micronet_send_address(micronet_address_t address)
{
	micronet_send(address.high >> 24);
	micronet_send(address.high >> 16);
	micronet_send(address.high >> 8 );
	micronet_send(address.high & 0xFF);	
	micronet_send(address.middle >> 24);
	micronet_send(address.middle >> 16);
	micronet_send(address.middle >> 8 );
	micronet_send(address.middle & 0xFF);
	micronet_send(address.low >> 24);
	micronet_send(address.low >> 16);
	micronet_send(address.low >> 8 );
	micronet_send(address.low & 0xFF);
}

void micronet_send_response(MicronetFrameTypeDef response)
{
		response.TargetAddress.low = micronet_request.SenderAddress.low;
		response.TargetAddress.middle = micronet_request.SenderAddress.middle;
		response.TargetAddress.high = micronet_request.SenderAddress.high;
	  micronet_send_frame(response);
}

void micronet_send_frame(MicronetFrameTypeDef frame)
{
	 for(char i=0;i<sizeof(micronet_preamble);i++)micronet_send_byte(micronet_preamble[i]);
	 micronet_crc = 0xFFFF;
	 micronet_send_address(micronet_address);
	 micronet_send_address(frame.TargetAddress);
	 micronet_send(frame.Cmd);
	 micronet_send(frame.DataLength >> 8);
	 micronet_send(frame.DataLength & 0xFF);
	 for(uint16_t i=0;i<frame.DataLength;i++)micronet_send(frame.Data[i]);
	 micronet_send_byte(micronet_crc>>8);
	 micronet_send_byte(micronet_crc&0xFF);
	 micronet_send_byte(0);
	 micronet_send_byte(0);
	 micronet_send_byte(0);
}

uint8_t preamble_buffer[4];
uint8_t header_buffer[27];
uint16_t data_cursor = 0;
uint16_t received_frame_crc = 0;

void micronet_frame_handler()
{
			if((micronet_request.TargetAddress.low    == micronet_address.low && \
				  micronet_request.TargetAddress.middle == micronet_address.middle && \
			    micronet_request.TargetAddress.high   == micronet_address.high) || \
			   (micronet_request.TargetAddress.low    == 0 && \
				  micronet_request.TargetAddress.middle == 0 && \
			    micronet_request.TargetAddress.high   == 0))
			{
					switch(micronet_request.Cmd){
						case 255:
							 random_delay();
						   micronet_response.Cmd = 255;
							 micronet_response.DataLength = device_descriptor_length;
						   for(int i=0;i<=device_descriptor_length;i++)micronet_response.Data[i] = device_descriptor[i];
							 micronet_send_response(micronet_response);
						break;
						
						case 254:
							 micronet_response.DataLength = device_descriptor_length;
						   for(int i=0;i<=device_descriptor_length;i++)micronet_response.Data[i] = device_descriptor[i];
							 micronet_send_response(micronet_response);
						break;
						
						default:
								micronet_handler();
					}
					
			}
}

void micronet_append_byte(uint8_t databyte)
{
		preamble_buffer[0] = preamble_buffer[1];
		preamble_buffer[1] = preamble_buffer[2];
		preamble_buffer[2] = preamble_buffer[3];
		preamble_buffer[3] = databyte;
	  
		if(preamble_buffer[0] == micronet_preamble[0] && preamble_buffer[1] == micronet_preamble[1] \
		&& preamble_buffer[2] == micronet_preamble[2] && preamble_buffer[3] == micronet_preamble[3]){
				data_cursor  = 0;
			  micronet_crc = 0xFFFF;
			  return;
		}
		if(data_cursor>26){
			if((uint16_t)(sizeof(micronet_request.Data)+27) > data_cursor)
			{
					micronet_request.Data[data_cursor-27] = databyte;
				  data_cursor++;
			}
			if((micronet_request.DataLength+28)==data_cursor){
				received_frame_crc = micronet_crc;
			}
			if((micronet_request.DataLength+29)==data_cursor){
				if(received_frame_crc==((uint16_t)(micronet_request.Data[data_cursor-29]<<8)|micronet_request.Data[data_cursor-28])){
					micronet_frame_handler();
				}
			}
		}else{
			header_buffer[data_cursor++] = databyte;
		}	
    crc16_update(databyte);		
		if(data_cursor==27){
			micronet_request.SenderAddress.high   = ((uint32_t)header_buffer[0]<<24 |(uint32_t)header_buffer[1]<<16 |(uint32_t)header_buffer[2]<<8 |(uint32_t)header_buffer[3]);
			micronet_request.SenderAddress.middle = ((uint32_t)header_buffer[4]<<24 |(uint32_t)header_buffer[5]<<16 |(uint32_t)header_buffer[6]<<8 |(uint32_t)header_buffer[7]);
			micronet_request.SenderAddress.low    = ((uint32_t)header_buffer[8]<<24 |(uint32_t)header_buffer[9]<<16 |(uint32_t)header_buffer[10]<<8|(uint32_t)header_buffer[11]);			
			micronet_request.TargetAddress.high   = ((uint32_t)header_buffer[12]<<24|(uint32_t)header_buffer[13]<<16|(uint32_t)header_buffer[14]<<8|(uint32_t)header_buffer[15]);
			micronet_request.TargetAddress.middle = ((uint32_t)header_buffer[16]<<24|(uint32_t)header_buffer[17]<<16|(uint32_t)header_buffer[18]<<8|(uint32_t)header_buffer[19]);
			micronet_request.TargetAddress.low    = ((uint32_t)header_buffer[20]<<24|(uint32_t)header_buffer[21]<<16|(uint32_t)header_buffer[22]<<8|(uint32_t)header_buffer[23]);		
			micronet_request.Cmd 									=   header_buffer[24];			
			micronet_request.DataLength					  = ((uint16_t)header_buffer[25]<<8 | (uint16_t)header_buffer[26]);		
		}
}
