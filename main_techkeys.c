#include "platforms.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "usb_keyboard.h"
#include "io.h"
#include "hid.h"
#include "timer.h"
#include "auxiliary.h"
#include "display.h"
#include "gfx.h"
#include "time.h"


volatile bool should_scan = false;

void frame(int row, int frame, int bright)
{
	if (frame == 0)
	{
		GFX_fill((struct rect){12, row, 6, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame ==1) {
		GFX_fill((struct rect){11, row, 6, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 2) {
		GFX_fill((struct rect){10, row, 6, 1}, bright);
		GFX_fill((struct rect){17, row, 3, 1}, bright);
	} else if (frame == 3) {
		GFX_fill((struct rect){9, row, 6, 1}, bright);
		GFX_fill((struct rect){16, row, 2, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 4) {
		GFX_fill((struct rect){8, row, 6, 1}, bright);
		GFX_fill((struct rect){15, row, 2, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 5) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 2, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 6) {
		GFX_putpixel(14, row, bright);
		GFX_fill((struct rect){17, row, 2, 1}, bright);
	} else if (frame == 7) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_putpixel(14, row, bright);
		GFX_fill((struct rect){16, row, 2, 1}, bright);
	} else if (frame == 8) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 3, 1}, bright);
	} else if (frame == 9) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_fill((struct rect){14, row, 2, 1}, bright);
	} else if (frame == 10) {
		GFX_fill((struct rect){7, row, 6, 1}, bright);
		GFX_putpixel(14, row, bright);
	} else if (frame == 11) {
		GFX_putpixel(16, row, bright);
	} else if (frame == 12) {
		GFX_putpixel(8, row, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 4, 1}, bright);
	} else if (frame == 13) {
		GFX_fill((struct rect){7, row, 2, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 5, 1}, bright);
	} else if (frame == 14 || frame == 15) {
		GFX_fill((struct rect){6, row, 3, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_fill((struct rect){14, row, 6, 1}, bright);
	} else if (frame == 16) {
		GFX_fill((struct rect){5, row, 5, 1}, bright);
		GFX_fill((struct rect){13, row, 6, 1}, bright);
	} else if (frame == 17) {
		GFX_fill((struct rect){5, row, 13, 1}, bright);
		GFX_putpixel(19, row, bright);
	} else if (frame == 18) {
		GFX_fill((struct rect){6, row, 11, 1}, bright);
		GFX_fill((struct rect){18, row, 2, 1}, bright);
	} else if (frame == 19) {
		GFX_putpixel(5, row, bright);
		GFX_fill((struct rect){7, row, 9, 1}, bright);
		GFX_fill((struct rect){17, row, 3, 1}, bright);
	} else if (frame == 20) {
		GFX_fill((struct rect){5, row, 2, 1}, bright);
		GFX_fill((struct rect){9, row, 6, 1}, bright);
		GFX_fill((struct rect){16, row, 4, 1}, bright);
	} else if (frame == 21) {
		GFX_fill((struct rect){5, row, 4, 1}, bright);
		GFX_fill((struct rect){11, row, 2, 1}, bright);
		GFX_fill((struct rect){15, row, 5, 1}, bright);
	} else if (frame == 22) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){13, row, 7, 1}, bright);
	} else if (frame == 23 || frame == 24 || frame == 25) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 8, 1}, bright);
	} else if (frame == 26) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 7, 1}, bright);
	} else if (frame == 27) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 4, 1}, bright);
	} else if (frame == 28) {
		GFX_fill((struct rect){5, row, 6, 1}, bright);
		GFX_fill((struct rect){12, row, 2, 1}, bright);
	} else if (frame == 29) {
		GFX_fill((struct rect){5, row, 4, 1}, bright);
	} else if (frame == 30) {
		GFX_fill((struct rect){5, row, 2, 1}, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	} else if (frame >= 31 && frame <= 35) {
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	} else if (frame == 36) {
		GFX_putpixel(8, row, bright);
		GFX_fill((struct rect){10, row, 3, 1}, bright);
		GFX_putpixel(14, row, bright);
	} else if (frame == 37) {
		GFX_fill((struct rect){8, row, 2, 1}, bright);
		GFX_fill((struct rect){13, row, 2, 1}, bright);
	} else if (frame == 38) {
		GFX_fill((struct rect){8, row, 7, 1}, bright);
	} else if (frame == 39) {
		GFX_fill((struct rect){9, row, 5, 1}, bright);
	} else if (frame == 40) {
		GFX_fill((struct rect){10, row, 3, 1}, bright);
	}
}

int main(void)
{
	clock_prescale_set(clock_div_1);

	sei();
	USB_init();
	HID_commit_state();

	for (int i = 0; i < 4; ++i) {
		IO_set(i, true);
		IO_config(i, OUTPUT);
	}
	IO_config(0, INPUT);  //
	IO_config(1, INPUT);
	IO_config(2, INPUT);
	IO_config(3, INPUT);
	IO_config(4, INPUT);
	IO_set(0, true);
	IO_set(1, true);
	IO_set(2, true);
	IO_set(3, true);
	IO_set(4, true);

	//TIMER_init();

	DISPLAY_init();
	GFX_swap();
	char btnUp = "";
	char *btnLeft = "";
	char *btnDown = "";
	char *btnRight = "";
	char *temp_string = "";
	char *temp_letter = "a";
	int prog_mode = 0;
	int prog_mode_select = 0;

	uint8_t bright = 4;

	TIME_delay_ms(200);
	for (int i =0 ;i<=35 ;++i)
	{
		TIME_delay_ms(20);
		frame(0,i,bright);
		frame(1,i+1,bright);
		frame(2,i+2,bright);
		frame(3,i+3,bright);
		frame(4,i+4,bright);
		frame(5,i+5,bright);
		GFX_swap();
	}
	TIME_delay_ms(150);
	
	unsigned int blink_timer = 0;
	while (true) {
		++blink_timer;
		//Display home text
		if (prog_mode == 0) {
			GFX_put_text((struct rect){0, 0, 24, 7}, 0, 0, "v1.8", bright, 0);
			GFX_swap();
			TIME_delay_ms(5);
		} else {
			//Commented out attempt at Blinking Cursor (Delay screws up key polling)
			if (strlen(temp_string)<=3) //Displays first 3 entry letters prior to scrolling
			{
				GFX_put_text((struct rect){0, 0, 24, 7}, 0, 0, temp_string, bright, 0);
				if (blink_timer % 30 < 15)
				{
					GFX_put_text((struct rect){0, 0, 24, 7}, 0, 0, temp_string, bright, 0);
					GFX_put_text((struct rect){strlen(temp_string)*6, 0, 24, 7}, 0, 0, temp_letter, bright, 0);
					TIME_delay_ms(14);
				}
				GFX_swap();
				TIME_delay_ms(5);
			} else {
				//Display text past 3 characters that is scrolled 
				int position = (strlen(temp_string) - 3) * -6;
				GFX_put_text((struct rect){0, 0, 24, 7}, position, 0, temp_string, bright, 0);
				if (blink_timer % 30 < 15)
				{
					GFX_put_text((struct rect){0, 0, 24, 7}, position, 0, temp_string, bright, 0);
					GFX_put_text((struct rect){18, 0, 24, 7}, 0, 0, temp_letter, bright, 0);
					TIME_delay_ms(14);
				}
				GFX_swap();
				TIME_delay_ms(5);
			}
		}

		//Poll Keys
		int k = 0;
		for (int i = 0; i < 5; ++i) {
			if (!IO_get(i)) {
				k = i+1;
			}
		}
		if (!k)
			continue;
		else if (k == 1) {	
			//UP ARROW
			if (prog_mode > 0) {
				//THERE HAS TO BE A BETTER WAY...
				if (temp_letter == "c") {
					temp_letter = "b";
					TIME_delay_ms(150);
				} else if (temp_letter == "b") {
					temp_letter = "a";
					TIME_delay_ms(150);
				}
			} else {
				//TODO OUTPUT UP ARROW TEXT
			}
		} else if (k == 2) {
			//LEFT ARROW
			if (prog_mode > 0) {
				//TODO SHORTEN TEMP_STRING BY 1
				int string_length = strlen(temp_string);
				temp_string[string_length - 1] = 0;
				TIME_delay_ms(300);
			} else {
				//TODO OUTPUT LEFT ARROW TEXT
			}
		} else if (k == 3) {
			//DOWN ARROW
			if (prog_mode > 0) {
				if (temp_letter == "a") {
					temp_letter = "b";
					TIME_delay_ms(150);
				} else if (temp_letter == "b")
				{
					temp_letter = "c";
					TIME_delay_ms(150);
				}
			} else {
				//TODO OUTPUT DOWN ARROW TEXT
			}
		} else if (k == 4) {
			//RIGHT ARROW
			if (prog_mode > 0) {
				//ADD LETTER TO TEMP_STRING
				temp_string = strncat(temp_string, temp_letter, 30);
				temp_letter = "a";
				TIME_delay_ms(150);
			} else {
				//TODO OUTPUT RIGHT ARROW TEXT
			}
			TIME_delay_ms(20);
		} else if (k == 5) {
			//PROG BUTTON
			if (prog_mode > 0) {
				//TODO SAVE TEMP_STRING TO BUTTON
				prog_mode = 0;
				memset(temp_string, 0, strlen(temp_string));
				if (prog_mode == 1)
					btnUp = temp_string;
				if (prog_mode == 2)
					btnLeft = temp_string;
				if (prog_mode == 3)
					btnDown = temp_string;
				if (prog_mode == 4)
					btnRight = temp_string;
				TIME_delay_ms(300);
				int i = 0;
				int d = 11;
			} else {
				prog_mode_select = 1;
				//Delay to avoid immediate escape from prog mode
				TIME_delay_ms(300);
				int i = 0;
				int d = 11;
				while (prog_mode_select == 1)
				{
					//Scroll Select Button Text
					GFX_fill((struct rect){4, 0, 2, 1}, bright);
					GFX_fill((struct rect){18, 0, 2, 1}, bright);
					GFX_putpixel(3, 1, bright);
					GFX_putpixel(6, 1, bright);
					GFX_fill((struct rect){11, 1, 3, 1}, bright);
					GFX_putpixel(17, 1, bright);
					GFX_putpixel(20, 1, bright);
					GFX_putpixel(6, 2, bright);
					GFX_putpixel(11, 2, bright);
					GFX_putpixel(13, 2, bright);
					GFX_putpixel(20, 2, bright);
					GFX_fill((struct rect){4, 3, 2, 1}, bright);
					GFX_fill((struct rect){9, 3, 7, 1}, bright);
					GFX_fill((struct rect){18, 3, 2, 1}, bright);
					GFX_putpixel(4, 4, bright);
					GFX_putpixel(9, 4, bright);
					GFX_putpixel(11, 4, bright);
					GFX_putpixel(13, 4, bright);
					GFX_putpixel(15, 4, bright);
					GFX_putpixel(18, 4, bright);
					GFX_fill((struct rect){9, 5, 7, 1}, bright);
					GFX_putpixel(4, 6, bright);
					GFX_putpixel(18, 6, bright);
					GFX_swap();
					
					//Poll Keys
					int m = 0;
					for (int i = 0; i < 5; ++i) {
						if (!IO_get(i)) {
							m = i+1;
						}
					}
					if (!m)
						continue;
					else if (m == 1) {	
						//UP ARROW
						prog_mode = 1;
						prog_mode_select = 0;
						TIME_delay_ms(300);
					} else if (m == 2) {
						//LEFT ARROW
						prog_mode = 2;
						prog_mode_select = 0;
						TIME_delay_ms(300);
					} else if (m == 3) {
						//DOWN ARROW
						prog_mode = 3;
						prog_mode_select = 0;
						TIME_delay_ms(300);
					} else if (m == 4) {
						//RIGHT ARROW
						prog_mode = 3;
						prog_mode_select = 0;
						TIME_delay_ms(300);
					} else if (m == 5) {
						//PROG BUTTON
						prog_mode = 0;
						prog_mode_select = 0;
						TIME_delay_ms(300);
					}
				}
			}
		}
			
	}


			/*
			HID_set_scancode_state(KD, true);
			HID_commit_state();
			TIME_delay_ms(2);
			TIME_delay_ms(18);
			HID_set_scancode_state(KD, false);
			HID_commit_state();
			TIME_delay_ms(20);
			*/
	while (true) {
	}

}


void MAIN_timer_handler()
{
}

void MAIN_handle_sof()
{
	TIME_update_1ms();
}
