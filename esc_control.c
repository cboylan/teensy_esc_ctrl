/* 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

static const uint16_t icr_val = 40000;
static const uint16_t min_ocr = 2000;
static const uint8_t min_ocrl = 0xD0;
static const uint8_t min_ocrh = 0x07;
static const uint16_t max_ocr = 4000;
static const uint8_t max_ocrl = 0xA0;
static const uint8_t max_ocrh = 0x0F;

void set_default_duty_cycles(void)
{
	OCR1AH = 0;
	OCR1AL = 0;

	OCR1BH = 0;
	OCR1BL = 0;

	OCR1CH = 0;
	OCR1CL = 0;

	OCR3AH = 0;
	OCR3AL = 0;
}

void setup_pwms(void)
{
    /* 16Mhz / 8 / 40000 = 50hz */
	ICR1 = icr_val;
	ICR3 = icr_val;

    /* Set OC1[A|B|C] to low on match, set high at TOP also set part of mode 14. */
	TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<COM1C1) | (1<<WGM11);
    /* Prescale factor of 8 and complete setting of PWM mode 14. */
	TCCR1B = (1<<CS11) | (1<<WGM12) | (1<<WGM13);

    /* Set OC3A to low on match, set high at TOP also set part of mode 14. */
	TCCR3A = (1<<COM3A1) | (1<<WGM31);
    /* Prescale factor of 8 and complete setting of PWM mode 14. */
	TCCR3B = (1<<CS31) | (1<<WGM32) | (1<<WGM33);

    /* Set initial PWM duties to 0% */
	OCR1AH = 0;
	OCR1AL = 0;

	OCR1BH = 0;
	OCR1BL = 0;

	OCR1CH = 0;
	OCR1CL = 0;

	OCR3AH = 0;
	OCR3AL = 0;

    /* Set initial counters to 0. Interrupts should be off. */
	TCNT1 = 0;
	TCNT3 = 0;

	// Timer 1 and Timer 3
	// Set ports to output
	DDRB |= (1 << DDB5) | (1 << DDB6) | (1 << DDB7);
    DDRC |= (1 << DDC6);
}

/**
 * @breif Handle a set pwm command
 */
void handle_set_pwm_command(uint8_t port, uint8_t vall, uint8_t valh)
{
    /* TODO: verify new duty cycle is within valid range. */
    if( (valh < min_ocrh && vall < min_ocrl) || (valh > max_ocrh && vall > max_ocrl) ){
        return;
    }

	switch(port)
	{
		case 0:
			OCR1AH = valh;
			OCR1AL = vall;
			break;
		case 1:
			OCR1BH = valh;
			OCR1BL = vall;
			break;
		case 2:
			OCR1CH = valh;
			OCR1CL = vall;
			break;
		case 3:
			OCR3AH = valh;
			OCR3AL = vall;
			break;
	}
}

void handle_arm_esc_command(uint8_t port)
{
    switch(port)
    {
        case 0:
            OCR1A = min_ocr;
	        _delay_ms(10000);
            break;
        case 1:
            OCR1B = min_ocr;
	        _delay_ms(10000);
            break;
        case 2:
            OCR1C = min_ocr;
	        _delay_ms(10000);
            break;
        case 3:
            OCR3A = min_ocr;
	        _delay_ms(10000);
            break;
        case 4:
            OCR1A = min_ocr;
            OCR1B = min_ocr;
            OCR1C = min_ocr;
            OCR3A = min_ocr;
	        _delay_ms(10000);
            break;
    }
}

int main(void)
{
	char buf[32];
	uint8_t n;
    uint16_t on_time;

	CPU_PRESCALE(0);
	setup_pwms();

    handle_arm_esc_command(0);

	while (1){
        for(on_time = min_ocr; on_time <= max_ocr; on_time += 10 ){
            handle_set_pwm_command(0, on_time, on_time >> 8);
            _delay_ms(100);
        }
        for(on_time = max_ocr; on_time >= min_ocr; on_time -= 10 ){
            handle_set_pwm_command(0, on_time, on_time >> 8);
            _delay_ms(100);
        }
	}
}
