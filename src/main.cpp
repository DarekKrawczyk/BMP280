#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "BMP280.h"

#define MOSI 19
#define SCK 18
#define CS 17
#define MISO 16

#define chip_id 0xD0

int main() {
    stdio_init_all();

    //Baudrate 0.5Mhz - couldn't find in datasheet
    spi_init(spi0, 500000);

    //SPI acceptable 00 and 11 configuration
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, (spi_order_t)0);

    //Mapping GPIO
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);

    // gpio_init(CS);
    // gpio_set_dir(CS, true);
    // gpio_put(CS, true);

    BMP280::BMP280 bmp280 = BMP280::BMP280(spi0, CS);

    uint8_t data = chip_id;
    uint8_t chipID = 0;
    spi_write_blocking(spi0, &data, 1);
    spi_read_blocking(spi0, 0, &chipID, 1);

    //Temperature oversampling x1, presure oversampling x1 0x27
    bmp280.setRegister(0xF4, 0b11100011);
    int counter = 0;
    while(true){
        printf("---------- DEBUG ----------\n");
        //chipID = bmp280.getData(chip_id, false);
        //printf("Chip ID: %#x\n", chipID);
        //printf("Temperature: %i\n", bmp280.readRawTemperature());
        //printf("Temperature: %f[C]\n", bmp280.readTemperature());
        //printf("Register: %#x\n", bmp280.getData(0xF4, false));
        if(counter==0){
            bmp280.setPowerMode(BMP280::PowerMode::Sleep);
            // uint8_t valueToSend = ((bmp280.readRegister(CTRL_MEAS) & 0b11111100) | 0b00000000);
            // bmp280.setRegister(CTRL_MEAS, valueToSend);
        }
        if(counter==10){
           bmp280.setPowerMode(BMP280::PowerMode::Forced); 
            // uint8_t valueToSend = ((bmp280.readRegister(CTRL_MEAS) & 0b11111100) | 0b00000010);
            // bmp280.setRegister(CTRL_MEAS, valueToSend);
        }
        if(counter==20){
            bmp280.setPowerMode(BMP280::PowerMode::Normal);
            // uint8_t valueToSend = ((bmp280.readRegister(CTRL_MEAS) & 0b11111100) | 0b00000011);
            // bmp280.setRegister(CTRL_MEAS, valueToSend);
        }
        counter++;
        if(counter==30){
            counter=0;
        }
        printf("Counter: %i\n", counter);
        printf("Register: %i\n", bmp280.readPowerMode());
        printf("ChipID: %#x\n", bmp280.readForChipID());
        printf("---------- END ----------\n\n\n");
        sleep_ms(1000);
    }
}

/*  TODO:
    - PowerMode - done
    - Temperature - done
    - Presure - ...
    - other functionalities
*/