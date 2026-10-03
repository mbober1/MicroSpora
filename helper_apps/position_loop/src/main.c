/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <spinner/control/cloop.h>
#include <spinner/drivers/feedback.h>

#include <arm_math.h>
#include <math.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define POSITION_TARGET_SCALE 1000.0f
#define POSITION_TARGET_MAX_DEG 1000000.0f

static atomic_t position_target_millidegrees;

static int cmd_position(const struct shell *shell, size_t argc, char **argv)
{
	char *end;
	float target_degrees;

	if (argc != 2U) {
		shell_error(shell, "Usage: position <degrees>");
		return -EINVAL;
	}

	target_degrees = strtof(argv[1], &end);
	if (end == argv[1] || *end != '\0' || !isfinite(target_degrees) ||
	    fabsf(target_degrees) > POSITION_TARGET_MAX_DEG) {
		shell_error(shell, "Position must be a finite value within +/-1000000 degrees");
		return -EINVAL;
	}

	atomic_set(&position_target_millidegrees,
		   (atomic_val_t)lroundf(target_degrees * POSITION_TARGET_SCALE));
	shell_print(shell, "Position target: %.3f degrees", (double)target_degrees);

	return 0;
}

SHELL_CMD_ARG_REGISTER(position, NULL, "Set multi-turn mechanical position in degrees",
		       cmd_position, 2, 0);

int main(void)
{
	LOG_ERR("TEST");
	const struct device *feedback = DEVICE_DT_GET(DT_NODELABEL(feedback));
	arm_pid_instance_f32 position_pid = {0};
	volatile static float position_degrees;
	float current_limit;
	uint32_t last_status_ms = 0U;
	int ret;

	if (!device_is_ready(feedback)) {
		LOG_ERR("Position control stopped: feedback device is not ready");
		return 0;
	}

	position_degrees = feedback_get_position(feedback);

	atomic_set(&position_target_millidegrees,
		   (atomic_val_t)lroundf(position_degrees * POSITION_TARGET_SCALE));

	position_pid.Kp = (float)CONFIG_APP_POSITION_PID_KP_UA_PER_DEG / 1000000.0f;
	position_pid.Ki = (float)CONFIG_APP_POSITION_PID_KI_UA_PER_DEG / 1000000.0f;
	position_pid.Kd = (float)CONFIG_APP_POSITION_PID_KD_UA_PER_DEG / 1000000.0f;
	arm_pid_init_f32(&position_pid, 1);
	current_limit = (float)CONFIG_APP_POSITION_CURRENT_LIMIT_MA / 1000.0f;

	cloop_start();
	cloop_set_ref(0.0f, 0.0f);
	LOG_INF("Position control active at %.3f degrees", (double)position_degrees);

	while (true) {
		float target_degrees =
			(float)atomic_get(&position_target_millidegrees) / POSITION_TARGET_SCALE;
		float iq_reference;
		uint32_t now_ms = k_uptime_get_32();

		position_degrees = feedback_get_position(feedback);
		if ((uint32_t)(now_ms - last_status_ms) >= 1000U) {
			LOG_INF("Position %.3f deg, target %.3f deg",
				(double)position_degrees, (double)target_degrees);
			last_status_ms = now_ms;
		}

		iq_reference = arm_pid_f32(&position_pid, target_degrees - position_degrees);
		if (iq_reference > current_limit) {
			iq_reference = current_limit;
			position_pid.state[2] = iq_reference;
		} else if (iq_reference < -current_limit) {
			iq_reference = -current_limit;
			position_pid.state[2] = iq_reference;
		}

		cloop_set_ref(0.0f, iq_reference);
		k_sleep(K_MSEC(CONFIG_APP_POSITION_LOOP_PERIOD_MS));
	}

	return 0;
}
