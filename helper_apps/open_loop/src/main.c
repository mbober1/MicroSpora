#include <arm_math.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <spinner/drivers/currsmp.h>
#include <spinner/drivers/svpwm.h>

#define VOLTAGE_AMPLITUDE 0.3f
#define ANGLE_STEP_DEGREES 1.0f
#define STEP_PERIOD_MS 1

const struct device *currsmp = DEVICE_DT_GET(DT_NODELABEL(currsmp));
struct currsmp_curr curr;


static void noop_regulation_callback(void *ctx)
{
	currsmp_get_currents(currsmp, &curr);
    (void)ctx;
}

int main(void)
{
    const struct device *svpwm = DEVICE_DT_GET(DT_NODELABEL(svpwm));
    float angle = 0.0f;

    if (!device_is_ready(currsmp) || !device_is_ready(svpwm)) {
        return 0;
    }

    currsmp_configure(currsmp, noop_regulation_callback, NULL);
    currsmp_start(currsmp);
    svpwm_start(svpwm);

    while (1) {
        float sine;
        float cosine;

        arm_sin_cos_f32(angle, &sine, &cosine);
        svpwm_set_phase_voltages(svpwm,
                                 VOLTAGE_AMPLITUDE * cosine,
                                 VOLTAGE_AMPLITUDE * sine);

        angle += ANGLE_STEP_DEGREES;
        if (angle >= 360.0f) {
            angle -= 360.0f;
        }

        k_msleep(STEP_PERIOD_MS);
    }

    return 0;
}