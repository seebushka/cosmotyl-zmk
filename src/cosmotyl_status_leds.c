#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/transport/central.h>

#define LED_NODE DT_NODELABEL(cosmotyl_status_leds)
#define PERIPHERAL_COUNT 2

/* Physical receiver layout:
 *   pixel 1 = LEFT indicator
 *   pixel 0 = RIGHT indicator
 *
 * Provisional ZMK source mapping:
 *   source 0 = LEFT
 *   source 1 = RIGHT
 */
static const uint8_t source_to_pixel[PERIPHERAL_COUNT] = {1, 0};

static const struct device *const strip = DEVICE_DT_GET(LED_NODE);

static uint8_t battery_level[PERIPHERAL_COUNT];
static bool battery_valid[PERIPHERAL_COUNT];
static bool blink_on;

/* ZMK's selected central transport. */
extern const struct zmk_split_transport_central *active_transport;

static struct k_work_delayable status_work;

static struct led_rgb connected_colour(uint8_t source)
{
    if (!battery_valid[source]) {
        /* Connected but waiting for first battery report. */
        return (struct led_rgb){ .r = 0, .g = 0, .b = 6 };
    }

    if (battery_level[source] <= 15) {
        return (struct led_rgb){ .r = 6, .g = 0, .b = 0 };
    }

    if (battery_level[source] <= 30) {
        return (struct led_rgb){ .r = 6, .g = 2, .b = 0 };
    }

    return (struct led_rgb){ .r = 0, .g = 6, .b = 0 };
}

static void update_status_leds(struct k_work *work)
{
    bool connected[PERIPHERAL_COUNT] = {false, false};
    uint8_t sources[PERIPHERAL_COUNT];

    if (active_transport &&
        active_transport->api &&
        active_transport->api->get_available_source_ids) {

        int count = active_transport->api->get_available_source_ids(sources);

        if (count > 0) {
            for (int i = 0; i < count && i < PERIPHERAL_COUNT; i++) {
                if (sources[i] < PERIPHERAL_COUNT) {
                    connected[sources[i]] = true;
                }
            }
        }
    }

    struct led_rgb pixels[2] = {
        { .r = 0, .g = 0, .b = 0 },
        { .r = 0, .g = 0, .b = 0 },
    };

    for (uint8_t source = 0; source < PERIPHERAL_COUNT; source++) {
        uint8_t pixel = source_to_pixel[source];

        if (connected[source]) {
            pixels[pixel] = connected_colour(source);
        } else if (blink_on) {
            pixels[pixel] = (struct led_rgb){ .r = 0, .g = 0, .b = 6 };
        }
    }

    led_strip_update_rgb(strip, pixels, 2);

    blink_on = !blink_on;
    k_work_reschedule(&status_work, K_MSEC(750));
}

static int battery_listener(const zmk_event_t *eh)
{
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);

    if (!ev) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->source < PERIPHERAL_COUNT) {
        battery_level[ev->source] = ev->state_of_charge;
        battery_valid[ev->source] = true;
        k_work_reschedule(&status_work, K_NO_WAIT);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(cosmotyl_status_battery, battery_listener);
ZMK_SUBSCRIPTION(cosmotyl_status_battery, zmk_peripheral_battery_state_changed);

static int cosmotyl_status_leds_init(void)
{
    if (!device_is_ready(strip)) {
        return -ENODEV;
    }

    k_work_init_delayable(&status_work, update_status_leds);
    k_work_schedule(&status_work, K_NO_WAIT);

    return 0;
}

SYS_INIT(cosmotyl_status_leds_init, APPLICATION, 90);
