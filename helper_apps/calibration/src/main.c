/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arm_math.h>
#include <math.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <spinner/control/cloop.h>
#include <spinner/drivers/feedback.h>

#define CALIBRATION_CURRENT 0.25f
#define ALIGNMENT_REVOLUTIONS 3
#define ALIGNMENT_STEPS_PER_REVOLUTION 360
#define ALIGNMENT_STEP_DEGREES 1.0f
#define ALIGNMENT_STEP_PERIOD_MS 10
#define ALIGNMENT_TRIALS 3
#define CALIBRATION_SAMPLES 64
#define CALIBRATION_SAMPLE_PERIOD_MS 10
#define CALIBRATION_MIN_CONCENTRATION 0.995f
#define CALIBRATION_MAX_REPEAT_ERROR_DEGREES 10.0f
#define CALIBRATION_MAX_TRACKING_ERROR_DEGREES 45.0f
#define RAD_TO_DEG 57.2957795131f

static float wrap_degrees(float angle)
{
    angle = fmodf(angle, 360.0f);
    return angle < 0.0f ? angle + 360.0f : angle;
}

static float signed_angle_difference(float angle, float reference)
{
    return wrap_degrees(angle - reference + 180.0f) - 180.0f;
}

int main(void)
{
    const struct device *feedback = DEVICE_DT_GET(DT_NODELABEL(feedback));
    float trial_angles[ALIGNMENT_TRIALS];
    float trial_sine_sum = 0.0f;
    float trial_cosine_sum = 0.0f;

    if (!device_is_ready(feedback)) {
        printk("Calibration failed: feedback device is not ready\n");
        return 0;
    }

    printk("Calibrating with regulated current. Keep the shaft unloaded...\n");
    cloop_set_eangle_override(0.0f);
    cloop_set_ref(CALIBRATION_CURRENT, 0.0f);
    cloop_start();

    for (int trial = 0; trial < ALIGNMENT_TRIALS; trial++) {
        float command_angle = 0.0f;
        float previous_sensor_angle = feedback_get_eangle(feedback);
        float tracked_motion = 0.0f;
        float sine_sum = 0.0f;
        float cosine_sum = 0.0f;

        for (int step = 0;
             step < ALIGNMENT_REVOLUTIONS * ALIGNMENT_STEPS_PER_REVOLUTION;
             step++) {
            cloop_set_eangle_override(command_angle);
            command_angle = wrap_degrees(command_angle +
                                          ALIGNMENT_STEP_DEGREES);
            k_msleep(ALIGNMENT_STEP_PERIOD_MS);

            float sensor_angle = feedback_get_eangle(feedback);
            tracked_motion += signed_angle_difference(sensor_angle,
                                 previous_sensor_angle);
            previous_sensor_angle = sensor_angle;

            if ((step + 1) % ALIGNMENT_STEPS_PER_REVOLUTION == 0) {
                float expected_motion =
                    (float)(step + 1) * ALIGNMENT_STEP_DEGREES;
                float tracking_error = fabsf(tracked_motion - expected_motion);

                if (tracked_motion < 0.0f ||
                    tracking_error > CALIBRATION_MAX_TRACKING_ERROR_DEGREES) {
                    cloop_stop();
                    printk("Calibration rejected: encoder tracked %.1f of %.1f deg; check pole pairs, phase order, and current\n",
                           (double)tracked_motion,
                           (double)expected_motion);
                    return 0;
                }
            }
        }

        cloop_set_eangle_override(0.0f);
        k_sleep(K_SECONDS(1));

        for (int sample = 0; sample < CALIBRATION_SAMPLES; sample++) {
            float electrical_angle = feedback_get_eangle(feedback);
            float sine;
            float cosine;

            arm_sin_cos_f32(electrical_angle, &sine, &cosine);
            sine_sum += sine;
            cosine_sum += cosine;
            k_sleep(K_MSEC(CALIBRATION_SAMPLE_PERIOD_MS));
        }

        float concentration = hypotf(sine_sum, cosine_sum) /
                              CALIBRATION_SAMPLES;
        if (concentration < CALIBRATION_MIN_CONCENTRATION) {
            cloop_stop();
            printk("Calibration rejected: trial %d did not settle (%.3f)\n",
                   trial + 1, (double)concentration);
            return 0;
        }

        trial_angles[trial] = wrap_degrees(
            atan2f(sine_sum, cosine_sum) * RAD_TO_DEG);
        float measured_sine;
        float measured_cosine;

        arm_sin_cos_f32(trial_angles[trial], &measured_sine,
                        &measured_cosine);
        trial_sine_sum += measured_sine;
        trial_cosine_sum += measured_cosine;

        int measured_millidegrees =
            (int)lroundf(trial_angles[trial] * 1000.0f);
        printk("Trial %d: %d.%03d deg, concentration %.3f\n", trial + 1,
               measured_millidegrees / 1000,
               measured_millidegrees % 1000, (double)concentration);
    }

    cloop_stop();

    float measured_angle = wrap_degrees(
        atan2f(trial_sine_sum, trial_cosine_sum) * RAD_TO_DEG);
    float max_repeat_error = 0.0f;

    for (int trial = 0; trial < ALIGNMENT_TRIALS; trial++) {
        float difference = wrap_degrees(trial_angles[trial] - measured_angle +
                                        180.0f) - 180.0f;

        max_repeat_error = MAX(max_repeat_error, fabsf(difference));
    }

    if (max_repeat_error > CALIBRATION_MAX_REPEAT_ERROR_DEGREES) {
        printk("Calibration rejected: alignment is not repeatable (max error %.1f deg)\n",
               (double)max_repeat_error);
        return 0;
    }

    float current_offset = DT_PROP(DT_NODELABEL(feedback), phase_offset_degrees);
    float calibrated_offset = wrap_degrees(current_offset - measured_angle);
    int dts_offset = (int)lroundf(calibrated_offset);

    if (dts_offset >= 360) {
        dts_offset -= 360;
    }

        printk("Mean electrical angle: %.2f deg\n", (double)measured_angle);
        printk("Maximum repeat error: %.2f deg\n", (double)max_repeat_error);
    printk("Set phase-offset-degrees = <%d> in MicroSpora DTS\n", dts_offset);

    return 0;
}
