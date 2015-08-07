/*
 IR Libraly
 For IR remocon Shield with Arduino UNO
 GNU General Public License V3
 */

#include "ir.h"
#include "mcu_api.h"
#include "mcu_errno.h"

//SYMBOLs
#define IR_DETECT 2000
#define IR_STOP 3360
#define IR_THRESH 805
#define IR_FORM 6500
#define IR_ADJUST 105
#define IR_TIMEOUT 100//mS

#define INPUT 1
#define OUTPUT 0

#define LOW 0
#define HIGH 1

unsigned long lap;

#define LAP_START() (lap = time_us())
#define LAP_RESTART(t) (lap += (t))
#define LAP_TIME (time_us() - lap)

void busy_wait(signed long wait_time) {
	signed long start_time = time_us();
	while (time_us() - start_time < wait_time) {
	}
}

void IR_ledon(IR_t* ir) {
	debug_print(DBG_INFO, "gpio_write 1 to %d\n", ir->ir_led_port);
	gpio_write(ir->ir_led_port, HIGH);

}

void IR_ledoff(IR_t* ir) {
	debug_print(DBG_INFO, "gpio_write 0 to %d\n", ir->ir_led_port);
	gpio_write(ir->ir_led_port, LOW);

}

bool IR_on(IR_t* ir) {
	return gpio_read(ir->ir_sensor_port) == LOW;
}

bool IR_off(IR_t* ir) {
	return gpio_read(ir->ir_sensor_port) == HIGH;
}
void IR_initialize(IR_t* ir, int led_port, int sensor_port) {
	ir->ir_led_port = led_port;
	ir->ir_sensor_port = sensor_port;
	debug_print(DBG_INFO, "setup led_port: %d\n", ir->ir_led_port);
	debug_print(DBG_INFO, "setup lr_sensor_port: %d\n", ir->ir_sensor_port);

	gpio_setup(ir->ir_led_port, OUTPUT);
	gpio_setup(ir->ir_sensor_port, INPUT);
}

void IR_analyze(IR_t* ir) {
	unsigned char i, b;
	unsigned int sp;

	ir->tl = ir->hl = ir->ll = 0;
	ir->ts = ir->hs = ir->ls = 65535;

	LAP_START();
	while (LAP_TIME < IR_DETECT)
		if (IR_off(ir))
			LAP_START();
	while (IR_on(ir))
		;
	ir->ldrh = LAP_TIME - IR_ADJUST;

	while (IR_off(ir))
		;
	ir->ldrl = LAP_TIME - ir->ldrh;

	LAP_RESTART(ir->ldrh + ir->ldrl);
	for (i = 0; i < IR_MAX; i++)
		for (b = 0; b < 8; b++) {

			while (IR_on(ir))
				;
			ir->turn = LAP_TIME - IR_ADJUST;
			if (ir->turn > ir->tl)
				ir->tl = ir->turn;
			else if (ir->turn < ir->ts)
				ir->ts = ir->turn;

			while (IR_off(ir)) {
				sp = LAP_TIME - ir->turn;
				if (sp > IR_STOP)
					goto EXIT;
			}
			LAP_RESTART(ir->turn + sp);
			ir->sig[i] <<= 1;

			if (sp > IR_THRESH) {
				ir->sig[i] |= 1;
				if (sp > ir->hl)
					ir->hl = sp;
				else if (sp < ir->hs)
					ir->hs = sp;
			} else {
				ir->sig[i] &= 0xfe;
				if (sp > ir->ll)
					ir->ll = sp;
				else if (sp < ir->ls)
					ir->ls = sp;
			}
		}
	EXIT: ir->tail = time_us();
	ir->scnt = i;
	ir->turn = (ir->tl + ir->ts) / 2;
	ir->sigh = (ir->hl + ir->hs) / 2;
	ir->sigl = (ir->ll + ir->ls) / 2;

	ir->aeha = (ir->ldrh < IR_FORM);
}

bool IR_analyze2(IR_t* ir) {
	unsigned char i, b;
	unsigned int sp;

	LAP_START();
	while (LAP_TIME < IR_DETECT) {
		if (IR_off(ir))
			LAP_START();
		if (time_us() - ir->tail > IR_TIMEOUT)
			return false;
	}
	ir->interval = (time_us() - ir->tail);
	ir->interval -= IR_DETECT / 1000;

	while (IR_on(ir))
		;
	while (IR_off(ir))
		;

	for (i = 0; i < IR_MAX; i++) {
		for (b = 0; b < 8; b++) {
			while (IR_on(ir))
				;
			LAP_START();
			while (IR_off(ir)) {
				sp = LAP_TIME;
				if (sp > IR_STOP)
					goto EXIT;
			}
			ir->sig[i] <<= 1;
			ir->sig[i] |= (sp > IR_THRESH);
		}
	}
	EXIT: ir->tail = time_us();
	ir->scnt = i;
	return true;
}

void IR_shot(IR_t* ir, unsigned long t) {
	LAP_START();
	while (LAP_TIME < t) {
		gpio_write(ir->ir_led_port, HIGH);
		if (ir->aeha) { //38.02kHz, 50.2%
			busy_wait(13);
		} else { //37.93kHz, 34.6%
			busy_wait(9);
		}
		gpio_write(ir->ir_led_port, LOW);
		if (ir->aeha) {
			busy_wait(13);
		} else {
			busy_wait(17);
		}
	}
}

void IR_control(IR_t* ir) {
	unsigned char i;
	unsigned char b;

	IR_shot(ir, ir->ldrh);
	busy_wait(ir->ldrl);

	for (i = 0; i < ir->scnt; i++) {
		for (b = 0; b < 8; b++) {
			IR_shot(ir, ir->turn);
			if ((ir->sig[i] & (0x80 >> b)))
				busy_wait(ir->sigh);
			else
				busy_wait(ir->sigl);
		}
	}
	IR_shot(ir, ir->turn);
	busy_wait(IR_STOP);
}

