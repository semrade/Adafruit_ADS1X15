#ifndef __I2C_H__
#define __I2C_H__
#include <iostream>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

class I2CDevice {
public:
    I2CDevice(const std::string &devicePath, int address) : devicePath(devicePath), address(address), file(-1) {
        open();
    }

    ~I2CDevice() {
        close();
    }

    void write(uint8_t reg, uint8_t data) {
        uint8_t buffer[2] = {reg, data};
        if (::write(file, buffer, 2) != 2) {
            throw std::runtime_error("Failed to write to the I2C bus");
        }
    }

    void write(uint8_t *buffer, uint8_t len) {
        if (::write(file, buffer, len) != len) {
            throw std::runtime_error("Failed to write to the I2C bus");
            }
    }

    uint8_t read(uint8_t reg) {
        if (::write(file, &reg, 1) != 1) {
            throw std::runtime_error("Failed to write to the I2C bus");
        }
        uint8_t data;
        if (::read(file, &data, 1) != 1) {
            throw std::runtime_error("Failed to read from the I2C bus");
        }
        return data;
    }

    void read(uint8_t *buffer, uint8_t len) {
        if (::write(file, &buffer[0], 1) != 1) {
            throw std::runtime_error("Failed to write to the I2C bus");
        }
        if (::read(file, buffer, len) != len) {
            throw std::runtime_error("Failed to read from the I2C bus");
        }

    }

    void close() {
        if (file != -1) {
            ::close(file);
            file = -1;
        }
    }

private:
    std::string devicePath;
    int address;
    int file;

    void open() {
        file = ::open(devicePath.c_str(), O_RDWR);
        if (file < 0) {
            throw std::runtime_error("Failed to open the I2C bus");
        }

        if (ioctl(file, I2C_SLAVE, address) < 0) {
            ::close(file);
            throw std::runtime_error("Failed to set I2C address");
        }
    }
};

#endif