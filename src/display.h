#include <drivers/sensor.h>
#include <display/cfb.h>

#define DEVICE_NAME "Zephyr Gateway"

const uint8_t NAME_STARTING_INDEX = 40;
const uint8_t RIGHT_SIDE_STARTING_INDEX = 120;
const uint8_t CHAR_LENGTH = 15;
const uint8_t LINE_HEIGHT = 24;

/*
* Because sensor value decimals are very long on display screen,
* I am only print the first two values
*/

uint32_t get_first_two_digits(uint32_t value) {
    uint32_t remainder = value;

    while (remainder >= 100) {
        remainder /= 10;
    }
    return remainder;
}

void write_name_to_screen(const struct device *dev, char *name) {
    printk("writing name \"%s\" to display\n", name);
    cfb_print(dev, name, NAME_STARTING_INDEX, 0);
}

void write_temp_to_screen(const struct device *dev, struct sensor_value *temp) {
    char temperature[CHAR_LENGTH];
    sprintf(temperature, "Temp: %d.%d", temp->val1, get_first_two_digits(temp->val2));
    printk("writing temp %d.%d to display\n", temp->val1, temp->val2);
    cfb_print(dev, temperature, RIGHT_SIDE_STARTING_INDEX, LINE_HEIGHT);
}

void write_humidity_to_screen(const struct device *dev, struct sensor_value *hum) {
    char humidity[CHAR_LENGTH];
    sprintf(humidity, "Hum : %d.%d", hum->val1, get_first_two_digits(hum->val2));
    printk("writing humidity %d.%d to display\n", hum->val1, hum->val2);
    cfb_print(dev, humidity, RIGHT_SIDE_STARTING_INDEX, LINE_HEIGHT + 16);
}

char * dump_to_json(struct sensor_value *temp, struct sensor_value *hum, struct sensor_value coord[3]) {
	char *buf = k_malloc(100);
	char *template = "{\"Temp\": %d.%d, \"Hum\": %d.%d, \"X\": %d.%d, \"Y\": %d.%d, \"Z\": %d.%d, \"device\": \"%s\"}";
    sprintf(buf, template, 
            temp->val1, get_first_two_digits(temp->val2), 
            hum->val1, get_first_two_digits(hum->val2),
            coord[0].val1, get_first_two_digits(coord[0].val2),
            coord[1].val1, get_first_two_digits(coord[1].val2),
            coord[2].val1, get_first_two_digits(coord[2].val2),
            DEVICE_NAME
    );
    return buf;
}

void write_coordinates_to_screen(const struct device *dev, struct sensor_value coord[3]) {
    char x[CHAR_LENGTH], y[CHAR_LENGTH], z[CHAR_LENGTH];
    sprintf(x, "AX  : %02d.%d", coord[0].val1, get_first_two_digits(coord[0].val2));
    sprintf(y, "AY  : %02d.%d", coord[1].val1, get_first_two_digits(coord[1].val2));
    sprintf(z, "AZ  : %02d.%d", coord[2].val1, get_first_two_digits(coord[2].val2));
    printk("AX = %d.%d, AY = %d.%d, AZ = %d.%d\n", 
            coord[0].val1, coord[0].val2,
            coord[1].val1, coord[1].val2,
            coord[2].val1, coord[2].val2);
    cfb_print(dev, x, RIGHT_SIDE_STARTING_INDEX, LINE_HEIGHT + 16*2);
    cfb_print(dev, y, RIGHT_SIDE_STARTING_INDEX, LINE_HEIGHT + 16*3);
    cfb_print(dev, z, RIGHT_SIDE_STARTING_INDEX, LINE_HEIGHT + 16*4);
}

void write_separator(const struct device *dev) {
    uint8_t starting_index = RIGHT_SIDE_STARTING_INDEX-16;
    uint8_t max_rows = cfb_get_display_parameter(dev, CFB_DISPLAY_ROWS);
    for(uint8_t i = 0; i < max_rows; i++) {
        cfb_print(dev, "|", starting_index, LINE_HEIGHT + (i*16));
    }
    printk("separating marks have been written to display\n");
}
