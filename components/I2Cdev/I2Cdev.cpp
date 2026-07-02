// I2Cdev library collection - Main I2C device class
// Abstracts bit and byte I2C R/W functions into a convenient class
// EFM32 stub port by Nicolas Baldeck <nicolas@pioupiou.fr>
// Based on Arduino's I2Cdev by Jeff Rowberg <jeff@rowberg.net>
//
// Changelog:
//      2015-01-02 - Initial release


/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2015 Jeff Rowberg, Nicolas Baldeck

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include <esp_log.h>
#include <esp_err.h>

#include "I2Cdev.h"

#define I2C_NUM I2C_NUM_0

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); } } while(0);

namespace {

static i2c_master_bus_handle_t s_bus_handle = nullptr;

static esp_err_t ensure_bus()
{
	if (s_bus_handle != nullptr) {
		return ESP_OK;
	}

	esp_err_t err = i2c_master_get_bus_handle(I2C_NUM, &s_bus_handle);
	if (err == ESP_OK) {
		return ESP_OK;
	}

	i2c_master_bus_config_t bus_config = {};
	bus_config.i2c_port = I2C_NUM;
	bus_config.sda_io_num = (gpio_num_t)I2C_SDA_PIN;
	bus_config.scl_io_num = (gpio_num_t)I2C_SCL_PIN;
	bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
	bus_config.glitch_ignore_cnt = 7;
	bus_config.intr_priority = 0;
	bus_config.trans_queue_depth = 1;
	bus_config.flags.enable_internal_pullup = 1;

	return i2c_new_master_bus(&bus_config, &s_bus_handle);
}

static esp_err_t add_device(uint8_t devAddr, i2c_master_dev_handle_t *dev_handle)
{
	esp_err_t err = ensure_bus();
	if (err != ESP_OK) {
		return err;
	}

	i2c_device_config_t dev_config = {};
	dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	dev_config.device_address = devAddr;
	dev_config.scl_speed_hz = 100000;
	dev_config.scl_wait_us = 0;
	dev_config.flags.disable_ack_check = 0;

	return i2c_master_bus_add_device(s_bus_handle, &dev_config, dev_handle);
}

static void remove_device(i2c_master_dev_handle_t dev_handle)
{
	if (dev_handle != nullptr) {
		i2c_master_bus_rm_device(dev_handle);
	}
}

}

/** Default constructor.
 */
I2Cdev::I2Cdev() {
}

/** Initialize I2C0
 */
void I2Cdev::initialize() {
	(void)ensure_bus();
}

/** Enable or disable I2C
 * @param isEnabled true = enable, false = disable
 */
void I2Cdev::enable(bool isEnabled) {
	if (isEnabled) {
		initialize();
	}
}

/** Default timeout value for read operations.
 */
uint16_t I2Cdev::readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;
/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout) {


	uint8_t b;
    uint8_t count = readByte(devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = readByte(devAddr, regAddr, &b, timeout)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return Status of read operation (true = success)
 */
int8_t I2Cdev::readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout) {
    return readBytes(devAddr, regAddr, 1, data, timeout);
}

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev::readTimeout)
 * @return I2C_TransferReturn_TypeDef http://downloads.energymicro.com/documentation/doxygen/group__I2C.html
 */
int8_t I2Cdev::readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout) {
	if (length == 0 || data == nullptr) {
		return 0;
	}

	i2c_master_dev_handle_t dev_handle = nullptr;
	if (add_device(devAddr, &dev_handle) != ESP_OK) {
		return 0;
	}

	uint8_t reg = regAddr;
	esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg, 1, data, length, timeout ? timeout : readTimeout);
	remove_device(dev_handle);

	return (err == ESP_OK) ? length : 0;
}

bool I2Cdev::writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data){

	uint8_t data1[] = {(uint8_t)(data>>8), (uint8_t)(data & 0xff)};
	return writeBytes(devAddr, regAddr, 2, data1);
}

void I2Cdev::SelectRegister(uint8_t dev, uint8_t reg){
	i2c_master_dev_handle_t dev_handle = nullptr;
	if (add_device(dev, &dev_handle) != ESP_OK) {
		return;
	}

	uint8_t reg_byte = reg;
	ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, &reg_byte, 1, I2Cdev::readTimeout));
	remove_device(dev_handle);
}

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    readByte(devAddr, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return writeByte(devAddr, regAddr, b);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b = 0;
    if (readByte(devAddr, regAddr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return writeByte(devAddr, regAddr, b);
    } else {
        return false;
    }
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
	uint8_t payload[2] = { regAddr, data };
	i2c_master_dev_handle_t dev_handle = nullptr;
	if (add_device(devAddr, &dev_handle) != ESP_OK) {
		return false;
	}

	esp_err_t err = i2c_master_transmit(dev_handle, payload, sizeof(payload), I2Cdev::readTimeout);
	remove_device(dev_handle);

	return err == ESP_OK;
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param length Number of bytes to write
 * @param data Array of bytes to write
 * @return Status of operation (true = success)
 */
bool I2Cdev::writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data){
	if (length == 0 || data == nullptr) {
		return false;
	}

	i2c_master_transmit_multi_buffer_info_t buffer_info[2];
	uint8_t reg = regAddr;
	buffer_info[0].write_buffer = &reg;
	buffer_info[0].buffer_size = 1;
	buffer_info[1].write_buffer = data;
	buffer_info[1].buffer_size = length;

	i2c_master_dev_handle_t dev_handle = nullptr;
	if (add_device(devAddr, &dev_handle) != ESP_OK) {
		return false;
	}

	esp_err_t err = i2c_master_multi_buffer_transmit(dev_handle, buffer_info, 2, I2Cdev::readTimeout);
	remove_device(dev_handle);
	return err == ESP_OK;
}


/**
 * read word
 * @param devAddr
 * @param regAddr
 * @param data
 * @param timeout
 * @return
 */
int8_t I2Cdev::readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout){
	uint8_t msb[2] = {0,0};
	uint8_t count = readBytes(devAddr, regAddr, 2, msb, timeout);
	if (count != 2) {
		return 0;
	}
	*data = (int16_t)((msb[0] << 8) | msb[1]);
	return count;
}
