#include "mcu_api.h"
#include "mcu_errno.h"
#include <string.h>

#define INPUT 1
#define OUTPUT 0

#define LOW 0
#define HIGH 1

#include "ir.h"

unsigned char shots;
unsigned char scnt[4];
unsigned char sig[4][IR_MAX];

unsigned char serialNo;

struct IR IRinstance;

void print(IR_t* ir) {
	unsigned char i, j;

	debug_print(DBG_INFO, "Format=");
	if (ir->aeha)
		debug_print(DBG_INFO, "Kaden seihin kyokai");
	else
		debug_print(DBG_INFO, "NEC");

	debug_print(DBG_INFO, "Leader ON=%d, OFF=%d\n", ir->ldrh, ir->ldrl);
	debug_print(DBG_INFO, "Tern = %d(%d-%d)\n", ir->turn, ir->tl, ir->ts);
	debug_print(DBG_INFO, "Signal HIGH=%d(%d-%d)\n", ir->sigh, ir->hl, ir->hs);
	debug_print(DBG_INFO, "Signal LOW=%d(%d-%d)\n", ir->sigl, ir->ll, ir->ls);

	if (shots > 1) {
		debug_print(DBG_INFO, "\nInterval=%d000\n", ir->interval);
	}

	for (j = 0; j < shots; j++) {
		debug_print(DBG_INFO, "\nSignal counts=%d\n", scnt[j]);

		for (i = 0; i < scnt[j]; i++) {
			debug_print(DBG_INFO, "Data No.%d = %x\n", i, sig[j][i]);
		}
	}
}

void record(IR_t* instance) {
	int i;

	debug_print(DBG_INFO, "IR shot ready.\n");
	IR_analyze(instance);
	scnt[0] = instance->scnt;
	for (i = 0; i < instance->scnt; i++)
		sig[0][i] = instance->sig[i];

	for (shots = 1; shots < 4; shots++) {
		if (IR_analyze2(instance)) {
			scnt[shots] = instance->scnt;
			for (i = 0; i < instance->scnt; i++)
				sig[shots][i] = instance->sig[i];
		} else {
			break;
		}
	}
	print(instance);

}

void shot(IR_t* ir) {
	unsigned char i, j;
	for (j = 0; j < shots; j++) {
		ir->scnt = scnt[j];
		for (i = 0; i < ir->scnt; i++)
			ir->sig[i] = sig[j][i];
		IR_control(ir);
		busy_wait(ir->interval * 1000);
	}
}

void mcu_main() {

	int led_port_num = 131; // GPIO 1
	int ir_sensor_port_num = 128; // GPIO 2
	int on_board_led = 40;

	char buf[512];
	int len;

	gpio_setup(on_board_led, OUTPUT);
	gpio_setup(led_port_num, OUTPUT);
	gpio_setup(ir_sensor_port_num, INPUT);

	IR_initialize(&IRinstance, led_port_num, ir_sensor_port_num);

	while (1) {
		do {
			len = host_receive((unsigned char *) buf, 512);
			mcu_sleep(10);
		} while (len <= 0);

		if (strncmp(buf, "led_on", 6) == 0) {
			debug_print(DBG_INFO, "received led_on command!\n");
			gpio_write(on_board_led, 1);
		} else if (strncmp(buf, "led_off", 7) == 0) {
			debug_print(DBG_INFO, "received led_off command!\n");
			gpio_write(on_board_led, 0);
		} else if (strncmp(buf, "record", 6) == 0) {
			debug_print(DBG_INFO, "received record command!\n");
			record(&IRinstance);
		} else if (strncmp(buf, "shot", 4) == 0) {
			debug_print(DBG_INFO, "received shot command!\n");
			shot(&IRinstance);
		} else if (strncmp(buf, "ir_on", 5) == 0) {
			debug_print(DBG_INFO, "received ir_on command!\n");
			IR_ledon(&IRinstance);
		} else if (strncmp(buf, "ir_off", 6) == 0) {
			debug_print(DBG_INFO, "received ir_off command!\n");
			IR_ledoff(&IRinstance);
		}
	}

}

