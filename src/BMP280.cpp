#include "BMP280.h"

BMP280::BMP280::BMP280(spi_inst_t* spi, uint cs){
    _spiInst = spi;
    _cs = cs;
    gpio_init(_cs);
    gpio_set_dir(_cs, true);
    gpio_put(_cs, true);
    this->getTrimmingParameters();
    _powerMode = PowerMode::Sleep;  //Default config
}

uint8_t BMP280::BMP280::readForChipID(){
    uint8_t result = this->readRegister(ID);
    _chipID = result;
    return result;
}

uint8_t BMP280::BMP280::getChipID() const{
    return _chipID;
}

int32_t BMP280::BMP280::getData(uint8_t reg, bool burst){
    uint8_t buffer[3] = {0x00, 0x00, 0x00};
    int32_t result = 0;
    uint8_t readRegister = reg |= 0b1000000; // AND with 0b1000000 for read operation in SPI comunication.
    if(burst == true){
        //Read 3 register one after another
        gpio_put(_cs, false);
        spi_write_blocking(_spiInst, &readRegister, 1);
        spi_read_blocking(_spiInst, 0, buffer, 3);
        gpio_put(_cs, true);
        //printf("Buffer[0]: %#x\n", buffer[0]);
        //printf("Buffer[1]: %#x\n", buffer[1]);
        //printf("Buffer[2]: %#x\n", buffer[2]);

        //buffer[0] is MSB... last 4 bits of buffer[2] are not taken into account, thats why its bitshifted.
        result = (((uint32_t)buffer[0]<<12) | ((uint32_t)buffer[1]<<4) | ((uint32_t)buffer[2]>>4));
    } 
    else
    {
        //Read only one register
        gpio_put(_cs, false);
        spi_write_blocking(_spiInst, &readRegister, 1);
        spi_read_blocking(_spiInst, 0, &buffer[0], 1);
        gpio_put(_cs, true);
        result = buffer[0];
    }
    return result;
}

// ---------------- Register ----------------

/// @brief Set value of specified register.
/// @param reg Type of register.
/// @param config Value to put into register.
/// @return true if action succeded, otherwise false.
bool BMP280::BMP280::setRegister(uint8_t reg, uint8_t config){
    uint8_t registr = reg;
    uint8_t data[2] = {(reg &= 0b01111111), config}; //OR with reg because R='0' bit in SPI write operation
    gpio_put(_cs, false);
    spi_write_blocking(_spiInst, data, 2);
    gpio_put(_cs, true);
    uint8_t value = this->readRegister(registr);
    if(value != config){
        return false;
    }
    return true;
}

/// @brief Read value of specified register via SPI command.
/// @param reg Type of register.
/// @return Value of specified register.
uint8_t BMP280::BMP280::readRegister(uint8_t reg){
    uint8_t data = (reg | 0b10000000);
    uint8_t buffer = 0;
    gpio_put(_cs, false);
    spi_write_blocking(_spiInst, &data, 1);
    spi_read_blocking(_spiInst, 0, &buffer, 1);
    gpio_put(_cs, true);
    return buffer;
}

// ---------------- Power mode ----------------

/// @brief Set power mode of BMP280 sensor.
/// @param mode PowerMode enum to set.
/// @return true if action succeded, otherwise false.
bool BMP280::BMP280::setPowerMode(PowerMode mode){
    uint8_t config = this->readRegister(CTRL_MEAS);
    switch (mode){
    case PowerMode::Sleep:
        config = ((config & 0b11111100) | 0b00000000);
        break;
    case PowerMode::Normal:
        config = ((config & 0b11111100) | 0b00000011);
        break;
    case PowerMode::Forced:
        config = ((config & 0b11111100) | 0b00000010);
        break;
    }
    bool value = this->setRegister(CTRL_MEAS, config);
    if(value == true){
        _powerMode = mode;
    }
    return value;
}

/// @brief Get current power mode via sending SPI command.
/// @return PowerMode enum.
BMP280::PowerMode BMP280::BMP280::readPowerMode(){
    uint8_t data = this->readRegister(CTRL_MEAS);
    data &= 0b00000011;
    PowerMode mode;
    switch (data)
    {
    case 0b00000000:
        mode = PowerMode::Sleep;
        break;
    case 0b00000011:
        mode = PowerMode::Normal;
        break;
    case 0b00000010:
        mode = PowerMode::Forced;
        break;
    case 0b00000001:
        mode = PowerMode::Forced;
        break;
    }
    _powerMode = mode;
    return mode;
}

/// @brief Get PowerMode value of object variable.
/// @return PowerMode enum.
BMP280::PowerMode BMP280::BMP280::getPowerMode() const{
    return _powerMode;
}

// ---------------- Temperature ----------------

/// @brief Read current temperature in raw format via SPI command.
/// @return Temperature in raw format.
int32_t BMP280::BMP280::readRawTemperature(){
    int32_t adc_T = this->getData(TEMP_MSB, true);
    int32_t var1, var2, temp;
    var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    temp = (((var1 + var2)) * 5 + 128) >> 8;
    _rawTemperature = temp;
    return temp;
}

/// @brief Get temperature of object variable.
/// @return Temeperature in raw format.
int32_t BMP280::BMP280::getRawTemperature() const{
    return _rawTemperature;
}

/// @brief Read temperature in celsius deg double format via SPI command.
/// @return Temperature in celsius deg double.
double BMP280::BMP280::readTemperature(){
    double temp = this->readRawTemperature()/100.0;
    _temperature = temp;
    return (this->readRawTemperature()/100.0);
}

/// @brief Get temperature of object variable.
/// @return Temperature in celsius deg double.
double BMP280::BMP280::getTemperature() const{
    return _temperature;
}

// ---------------- Temperature ----------------

uint32_t BMP280::BMP280::readPresure(){
    uint32_t presure;
    return presure;
}

uint32_t BMP280::BMP280::getPresure() const{
    return _presure;
}

// ---------------- Utilities ----------------

void BMP280::BMP280::getTrimmingParameters(){
    uint8_t reg = 0x88 | 0x80;
    uint8_t buffer[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    gpio_put(_cs, false);
    spi_write_blocking(_spiInst, &reg, 1);
    spi_read_blocking(_spiInst, 0, buffer, 24);
    gpio_put(_cs, true);
    dig_T1 = (buffer[1]<<8) | buffer[0];
    dig_T2 = (buffer[3]<<8) | buffer[2];
    dig_T3 = (buffer[5]<<8) | buffer[4];
    dig_P1 = (buffer[7]<<8) | buffer[6];
    dig_P2 = (buffer[9]<<8) | buffer[8];
    dig_P3 = (buffer[11]<<8) | buffer[10];
    dig_P4 = (buffer[13]<<8) | buffer[12];
    dig_P5 = (buffer[15]<<8) | buffer[14];
    dig_P6 = (buffer[17]<<8) | buffer[16];
    dig_P7 = (buffer[19]<<8) | buffer[18];
    dig_P8 = (buffer[21]<<8) | buffer[20];
    dig_P9 = (buffer[23]<<8) | buffer[22];
}