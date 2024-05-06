/*
  MIT License

  Copyright (c) 2020 nyannkov

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "usb_midi_app.h"
#include "midi.h"
//#include "mode4_ymf825.h"
//#include "music_box_ymf825.h"
//#include "single_ymz294.h"


#include <gd32vf103_rcu.h>

#define MAX_MIDI_HANDLE_LIST_COUNT      2
#define MIDI_HANDLE_FREE                0 
#define MIDI_HANDLE_OCCUPIED            1

#define USB_MIDI_APP_ASSERT(cond)

typedef struct 
{
	uint32_t status;
	MIDI_Handle_t hMIDI;
} midi_handle_list_t;

typedef struct _usb_midi_event_packet
{
	uint8_t header;
	uint8_t midi[3];
} usb_midi_event_packet_t;

typedef struct
{
	MIDI_Handle_t* (*midi_init)(void);
	void (*midi_deinit)(MIDI_Handle_t *phMIDI);
} sound_driver_api_t;

static  midi_handle_list_t hmidi_list[MAX_MIDI_HANDLE_LIST_COUNT];


int32_t usb_midi_proc(const uint8_t *mid_msg,  size_t len)
{
	gpio_bit_set(GPIOA, GPIO_PIN_1);
	if(gpio_input_bit_get(GPIOA, GPIO_PIN_2)){
		gpio_bit_reset(GPIOA, GPIO_PIN_2);
	} else{
		gpio_bit_set(GPIOA, GPIO_PIN_2);
	}
	return 0;
}


MIDI_Handle_t *MIDI_Alloc(void)
{
	uint32_t i = 0;
	for ( i = 0; i < MAX_MIDI_HANDLE_LIST_COUNT; i++ )
	{
		if ( hmidi_list[i].status == MIDI_HANDLE_FREE )
		{
			hmidi_list[i].status = MIDI_HANDLE_OCCUPIED;
			return &hmidi_list[i].hMIDI;
		}
	}
	return (MIDI_Handle_t * )0;
}

void MIDI_Free(MIDI_Handle_t *phMIDI)
{
	uint32_t i = 0;
	for ( i = 0; i < MAX_MIDI_HANDLE_LIST_COUNT; i++ )
	{
		if ( &hmidi_list[i].hMIDI == phMIDI )
		{
			hmidi_list[i].status = MIDI_HANDLE_FREE;
			break;
		}
	}
}

