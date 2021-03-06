#include <stdio.h>
#include <linker/sections.h>
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(galileo, LOG_LEVEL_INF);

#include "cfb_image.h"
#include "display.h"
#include "http_util.h"
#include "dhcp_config.h"

#define TEMP_DEVICE_NAME "HDC1010"
#define ACCE_DEVICE_NAME "MMA8652FC"
#define DISP_DEVICE_NAME "SSD16XX"

const struct device *temp_humidity_device;
const struct device *accelerometer_device;
const struct device *display_device;

char* convert_to_string(struct sensor_value *temp, struct sensor_value *hum){
	char *buf = k_malloc(100);
	char *values = "\"0x0001,%d.%d,%d.%d,1,0x0002:11\"";
	sprintf(buf, values, temp->val1, temp->val2, hum->val1, hum->val2);
	return buf;
}

void post_data(struct sensor_value *temp, struct sensor_value *hum, struct sensor_value coord[3]) {
	// char *json_data = dump_to_json(temp, hum, coord);
	char *json_data = convert_to_string(temp, hum);
	LOG_INF("sensor values posted: %s", json_data);
	int response;
	response = post_sensor_data(json_data);
	LOG_INF("---------- data posted ---------");
	k_free(json_data);
}

void main(void)
{
	LOG_INF("Initializing DHCP to obtain IP address");
	initialize_dhcp();

	temp_humidity_device = device_get_binding(TEMP_DEVICE_NAME);
	if (temp_humidity_device == NULL) {
		printk("Failed to initialize Temperature & Humidity device... aborting!\n");
		return;
	} else {
		printk("Temperature and humidity device has been initialized\n");
	}

	accelerometer_device = device_get_binding(ACCE_DEVICE_NAME);
	if(accelerometer_device == NULL){
		printk("Failed to initialize accelerometer device\n");
		return;
	} else {
		printk("Accelerometer has been initialized to %s\n", ACCE_DEVICE_NAME);
	}

	display_device = device_get_binding(DISP_DEVICE_NAME);
	if (display_device == NULL) {
		printk("Failed to initialize display device");
		return;
	} else {
		printk("Display device has been initialized... %s\n", DISP_DEVICE_NAME);
		if (cfb_framebuffer_init(display_device)) {
			printk("Framebuffer initialization failed\n");
			return;
		}
		display_blanking_off(display_device);
	}

	bool server_status = ping_http_server() >= 0;
	
	struct sensor_value temp;
	struct sensor_value humidity;
	struct sensor_value coord[3];
	char name[] = "Boss Bibentyo";

	while(1) {
		printk("Fetching temperature & humidity...\n");
		sensor_sample_fetch(temp_humidity_device);
		sensor_channel_get(temp_humidity_device, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(temp_humidity_device, SENSOR_CHAN_HUMIDITY, &humidity);

		printk("Fetching accelerometer data...\n");
		sensor_sample_fetch(accelerometer_device);
		sensor_channel_get(accelerometer_device, SENSOR_CHAN_ACCEL_XYZ, coord);

		cfb_framebuffer_clear(display_device, true);

		cfb_framebuffer_set_font(display_device, 0);
		cfb_print(display_device, "1", 0, LINE_HEIGHT);
	
		cfb_framebuffer_set_font(display_device, 2);
		write_name_to_screen(display_device, name);
		cfb_framebuffer_set_font(display_device, 1);
		write_separator(display_device);
		write_temp_to_screen(display_device, &temp);
		write_humidity_to_screen(display_device, &humidity);
		write_coordinates_to_screen(display_device, coord);	
		cfb_framebuffer_finalize(display_device);

		post_data(&temp, &humidity, coord);
		
		k_sleep(K_SECONDS(10));
	}
}
