#ifndef IR_H_
#define IR_H_


#define IR_MAX 32
#include <unistd.h>
#include <stdbool.h>

typedef struct IR {
	unsigned long tail;
	int ir_led_port;
	int ir_sensor_port;
	bool aeha;
	unsigned int ldrh, ldrl;
	unsigned int turn;
	unsigned int sigh, sigl;
	unsigned int tl, hl, ll;
	unsigned int ts, hs, ls;
	unsigned char scnt;
	unsigned char sig[IR_MAX];
	unsigned int interval;
} IR_t;

bool IR_on(IR_t*);
bool IR_off(IR_t*);
void IR_ledon(IR_t*);
void IR_ledoff(IR_t*);

void IR_initialize(IR_t*, int, int);

void IR_shot(IR_t*, unsigned long);

void IR_analyze(IR_t*);
void IR_control(IR_t*);
bool IR_analyze2(IR_t*);

extern IR_t IRinstance;
void busy_wait(signed long wait_time);

#endif
