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

	const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(encoder0));

	if (!device_is_ready(dev)) {
		printk("sensor: device not ready.\n");
		return 0;
	}

	printf("device is %p, name is %s\n", dev, dev->name);
static float value_deg;
static float value_vel;

	while (1) {
		// gpio_pin_toggle_dt(&led);

		ret = sensor_sample_fetch(dev);
		if (ret) {
			printk("sensor_sample_fetch failed ret %d\n", ret);
			return 0;
		}

		struct sensor_value angle;
		struct sensor_value velocity;
		sensor_channel_get(dev, SENSOR_CHAN_ROTATION, &angle);
		sensor_channel_get(dev, SENSOR_CHAN_RPM, &velocity);
		value_deg = sensor_value_to_float(&angle);
		value_vel = sensor_value_to_float(&velocity);
		// k_msleep(100);
	}
	return 0;
}
