#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/bldc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	const struct device *drv8316 = DEVICE_DT_GET(DT_NODELABEL(drv8316));
	int ret;

	LOG_INF("Starting DRV8316C SPI self-test");

	if (!device_is_ready(drv8316)) {
		LOG_ERR("DRV8316C initialization failed");
		return -ENODEV;
	}

	ret = bldc_drv_disable(drv8316);
	if (ret < 0) {
		LOG_ERR("DRV8316C disable/SPI test failed: %d", ret);
		return ret;
	}

	LOG_INF("DRV8316C SPI self-test passed; outputs remain disabled");
	return 0;
}