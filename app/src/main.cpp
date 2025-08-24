#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "math.h"

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	const struct device *const dev = DEVICE_DT_GET_ONE(mt6701);

	if (!device_is_ready(dev)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	printf("device is %p, name is %s\n", dev, dev->name);
static float value;
	while (1) {
		// gpio_pin_toggle_dt(&led);

		ret = sensor_sample_fetch(dev);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return 0;
		}
		
		struct sensor_value angle;
		sensor_channel_get(dev, SENSOR_CHAN_ROTATION, &angle);
		value = sensor_value_to_float(&angle);
		printk("Angle %d.%d", angle.val1, angle.val2);

	// 	start_ticks = SysTick->VAL;
	// 	cordic_sincos(angle, &sin, &cos);
	// 	stop_ticks = SysTick->VAL;
	// 	elapsed_ticks = start_ticks-stop_ticks;


	// 	start_ticks2 = SysTick->VAL;
	// 	sin2 = sinf(angle);
	// 	cos2 = cosf(angle);
	// 	stop_ticks2 = SysTick->VAL;
	// 	elapsed_ticks2 = start_ticks2-stop_ticks2;

	// 	printf("CORDIC: %d vs STDLIB: %d\n\r", elapsed_ticks, elapsed_ticks2);
		// k_msleep(100);
	}
	return 0;
}
