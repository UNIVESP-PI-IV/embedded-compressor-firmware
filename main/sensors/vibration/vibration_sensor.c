#include "sensors/vibration/vibration_sensor.h"
#include "cJSON.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define MPU6050_ADDR                0x68
#define MPU6050_ACCEL_FACTOR        16384.0f

#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_FREQ_HZ          400000


static const char *TAG = "VIBRATION_MPU6050";

static i2c_master_dev_handle_t mpu_dev = NULL;

esp_err_t vibration_sensor_init(void) {
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    i2c_new_master_bus(&bus_cfg, &bus_handle);

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &mpu_dev);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar o barramento I2C Master");
        return ret;
    }
    
    // Acorda o MPU-6050 escrevendo 0x00 no PWR_MGMT_1 (0x6B)
    uint8_t pwr_cmd[2] = {0x6B, 0x00};
    ret = i2c_master_transmit(mpu_dev, pwr_cmd, sizeof(pwr_cmd), 100);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "MPU-6050 inicializado com sucesso nos GPIOs %d (SDA) e %d (SCL)", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    } else {
        ESP_LOGE(TAG, "Falha ao acordar MPU-6050");
    }

    return ret;
}

esp_err_t vibration_sensor_read(vibration_sensor_data_t *data) {
    if (!data || !mpu_dev) {
        ESP_LOGE(TAG, "Parametros invalidos ou MPU nao inicializado");
        return ESP_FAIL;
    }

    uint8_t reg = 0x3B;
    uint8_t raw[6];
    if (i2c_master_transmit_receive(mpu_dev, &reg, 1, raw, 6, 100) != ESP_OK) {
        ESP_LOGE(TAG, "Falha na comunicacao I2C durante a leitura");
        return ESP_FAIL;
    }

    data->accel_x = (float)((int16_t)(raw[0] << 8 | raw[1])) / MPU6050_ACCEL_FACTOR;
    data->accel_y = (float)((int16_t)(raw[2] << 8 | raw[3])) / MPU6050_ACCEL_FACTOR;
    data->accel_z = ((float)((int16_t)(raw[4] << 8 | raw[5])) / MPU6050_ACCEL_FACTOR) - 1.0f; // Subtrai a aceleração gravitacional padrão (1 g)

    ESP_LOGI(TAG, "Aceleracao (g) -> X: %.2f | Y: %.2f | Z: %.2f", data->accel_x, data->accel_y, data->accel_z);
    return ESP_OK;
}
