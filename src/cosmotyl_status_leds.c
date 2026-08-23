#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/init.h>

#define LED_NODE DT_NODELABEL(cosmotyl_status_leds)

static const struct device *const strip = DEVICE_DT_GET(LED_NODE);

static int cosmotyl_status_leds_init(void)
{
    if (!device_is_ready(strip)) {
        return -ENODEV;
    }

    struct led_rgb pixels[2] = {
        { .r = 0, .g = 24, .b = 0 },   /* LED 1: green */
        { .r = 0, .g = 0,  .b = 24 },  /* LED 2: blue  */
    };

    return led_strip_update_rgb(strip, pixels, 2);
}

SYS_INIT(cosmotyl_status_leds_init, APPLICATION, 90);
