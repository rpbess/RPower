#ifndef __MICRONET_H
#define __MICRONET_H

#include <stdint.h>

typedef struct{
		uint32_t low    : 32;
		uint32_t middle : 32;
		uint32_t high   : 32;
}micronet_address_t;  

typedef struct
{
		micronet_address_t TargetAddress;
	  micronet_address_t SenderAddress;
	 	uint8_t  					 Cmd;
		uint8_t  					 Data[256];
		uint16_t 					 DataLength;
} MicronetFrameTypeDef;


extern MicronetFrameTypeDef micronet_response;
extern MicronetFrameTypeDef micronet_request;

extern void micronet_send_byte(uint8_t databyte);
extern void micronet_append_byte(uint8_t databyte);
extern void micronet_set_descriptor(char descriptor[]);
extern void micronet_send_frame(MicronetFrameTypeDef frame);
extern void micronet_send_response(MicronetFrameTypeDef response);
extern uint16_t micronet_find_delay;
void micronet_handler(void);
#endif
