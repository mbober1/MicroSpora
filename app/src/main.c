#include <spinner/control/cloop.h>
#include <spinner/drivers/feedback.h>

#include <arm_math.h>
#include <math.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define SPEED_TARGET_RPM 10.0f

int main(void)
{
	const struct device *feedback = DEVICE_DT_GET(DT_NODELABEL(feedback));
	arm_pid_instance_f32 speed_pid = {0};
	static float measured_rpm;
	float current_limit;

	if (!device_is_ready(feedback)) {
		LOG_ERR("Speed control stopped: feedback device is not ready");
		return 0;
	}

	measured_rpm = feedback_get_speed(feedback);
	if (!isfinite(measured_rpm)) {
		LOG_ERR("Speed control stopped: initial speed feedback is invalid");
		return 0;
	}

	speed_pid.Kp = (float)CONFIG_APP_SPEED_PID_KP_UA_PER_RPM / 1000000.0f;
	speed_pid.Ki = (float)CONFIG_APP_SPEED_PID_KI_UA_PER_RPM / 1000000.0f;
	speed_pid.Kd = (float)CONFIG_APP_SPEED_PID_KD_UA_PER_RPM / 1000000.0f;
	arm_pid_init_f32(&speed_pid, 1);
	current_limit = (float)CONFIG_APP_SPEED_CURRENT_LIMIT_MA / 1000.0f;

	cloop_start();
	cloop_set_ref(0.0f, 0.0f);
	LOG_INF("Speed control active: target %.2f RPM, measured %.2f RPM",
		(double)SPEED_TARGET_RPM, (double)measured_rpm);

	while (true) {
		float iq_reference;

		measured_rpm = feedback_get_speed(feedback);
		if (!isfinite(measured_rpm)) {
			cloop_set_ref(0.0f, 0.0f);
			cloop_stop();
			LOG_ERR("Speed control stopped: speed feedback is invalid");
			return 0;
		}

		iq_reference = arm_pid_f32(&speed_pid, SPEED_TARGET_RPM - measured_rpm);
		if (iq_reference > current_limit) {
			iq_reference = current_limit;
			speed_pid.state[2] = iq_reference;
		} else if (iq_reference < -current_limit) {
			iq_reference = -current_limit;
			speed_pid.state[2] = iq_reference;
		}

		cloop_set_ref(0.0f, iq_reference);
		k_sleep(K_MSEC(CONFIG_APP_SPEED_LOOP_PERIOD_MS));
	}

	return 0;
}
