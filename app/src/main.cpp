#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "math.h"
#include "cordic.hpp"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define M_PI		3.14159265358979323846

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


int main(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	cordic_init();

	
  uint32_t start_ticks, stop_ticks, elapsed_ticks;
  uint32_t start_ticks2, stop_ticks2, elapsed_ticks2;

  float sin, sin2;
  float cos, cos2;
  volatile float angle = 45 * M_PI / 180.0;

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;

		start_ticks = SysTick->VAL;
		cordic_sincos(angle, &sin, &cos);
		stop_ticks = SysTick->VAL;
		elapsed_ticks = start_ticks-stop_ticks;


		start_ticks2 = SysTick->VAL;
		sin2 = sinf(angle);
		cos2 = cosf(angle);
		stop_ticks2 = SysTick->VAL;
		elapsed_ticks2 = start_ticks2-stop_ticks2;

		printf("CORDIC: %d vs STDLIB: %d\n\r", elapsed_ticks, elapsed_ticks2);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
