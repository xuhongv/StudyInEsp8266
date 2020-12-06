#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sht20.h"

static const char *TAG = "sht20_drive.c";

static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size)
{
    if (size == 0)
    {
        return ESP_OK;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);

    if (size > 1)
    {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }

    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);

    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;

    // int ret, i2c_num = 0;
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, 0x78, ACK_CHECK_EN); //Slave address,SA0=0
    // i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN); //Slave address,SA0=0
    // i2c_master_write_byte(cmd, OLED_Byte, ACK_CHECK_EN);
    // i2c_master_stop(cmd);
    // ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    // i2c_cmd_link_delete(cmd);
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    // int i2c_master_port = DEV_SHT20_I2C_NUM;
    // i2c_config_t conf;
    // conf.mode = I2C_MODE_MASTER;
    // conf.sda_io_num = DEV_SHT20_SDA_NUM;
    // conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    // conf.scl_io_num = DEV_SHT20_SCL_NUM;
    // conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    // conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    // i2c_param_config(i2c_master_port, &conf);

    int i2c_master_port = 0;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = DEV_SHT20_SDA_NUM;
    conf.sda_pullup_en = 0;
    conf.scl_io_num = DEV_SHT20_SCL_NUM;
    conf.scl_pullup_en = 0;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    return 0;

    // return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void init_sht20(void)
{
    static bool tmp = true;
    if (tmp)
    {
        tmp = false;
        ESP_ERROR_CHECK(i2c_master_init());
    }
}

float get_sht20_Temperature(void)
{
    uint8_t data_rx[8] = {0};
    uint8_t data_wr[8] = {0};
    unsigned int dat = 0;
    float temp = 0;

    //懒加载
    init_sht20();

    data_wr[0] = HOLD_AT_START;
    i2c_master_write_slave(DEV_SHT20_I2C_NUM, data_wr, strlen((char *)data_wr));
    vTaskDelay(20 / portTICK_RATE_MS);

    i2c_master_read_slave(DEV_SHT20_I2C_NUM, data_rx, 3);

    if (!data_rx[0] && !data_rx[1])
    {
        return -1;
    }

    data_rx[1] &= 0xfc;
    dat = (data_rx[0] << 8) | data_rx[1];

    temp = ((float)dat * 175.72) / 65536.0 - 46.85; // ℃

    ESP_LOGD(TAG, "temp=%.3f℃", temp);

    return temp;
}

float get_sht20_Humidity(void)
{

    uint8_t data_rx[8] = {0};
    uint8_t data_wr[8] = {0};
    unsigned int dat = 0;
    float temp = 0;

    //懒加载
    init_sht20();

    data_wr[0] = HOLD_AH_START;
    i2c_master_write_slave(DEV_SHT20_I2C_NUM, data_wr, strlen((char *)data_wr));
    vTaskDelay(20 / portTICK_RATE_MS);

    i2c_master_read_slave(DEV_SHT20_I2C_NUM, data_rx, 3);

    if (!data_rx[0] && !data_rx[1])
    {
        return -1;
    }

    data_rx[1] &= 0xfc;
    dat = (data_rx[0] << 8) | data_rx[1];

    temp = (float)((dat * 125.0) / 65536.0 - 6); //%RH

    ESP_LOGD(TAG, "hum=%.2f", temp);

    return temp;
}
