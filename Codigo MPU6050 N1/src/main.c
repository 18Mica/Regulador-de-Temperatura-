#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "mpu6050.h"

#define SDA_GPIO  4
#define SCL_GPIO  5 
#define LED_PIN_X 14 // Led rojo
#define LED_PIN_Y 15 // Led verde
#define FAN_PIN 13   // Ventilador 
#define TEMP_LIMIT 29 // Límite de temperatura

int main(void)
{  
    stdio_init_all();
    sleep_ms(1000);

    printf("Inicializando I2C...\n");

    // Inicialización de I2C
    i2c_init(i2c0, 400000);
    gpio_set_function(SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(SCL_GPIO, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_GPIO);
    gpio_pull_up(SCL_GPIO);

    printf("Inicializando MPU6050...\n");

    // Inicialización del MPU6050
    mpu6050_init(i2c0, 0x68);
    sleep_ms(2000);

    printf("Who am I = 0x%2x\n", mpu6050_who_am_i());

    // Configuración de los pines de salida
    gpio_init(LED_PIN_X);
    gpio_init(LED_PIN_Y);
    gpio_init(FAN_PIN);
    gpio_set_dir(LED_PIN_X, GPIO_OUT);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_set_dir(FAN_PIN, GPIO_OUT);

    while (true) {
        // Estructura para datos del sensor
        mpu_accel_t accel = {0};
        mpu_gyro_t gyro = {0};
        int16_t temp = 0;

        // Leer datos del MPU6050
        mpu6050_read_accel(&accel);
        mpu6050_read_gyro(&gyro);
        mpu6050_read_temp(&temp);

        // Convertir temperatura a un valor real (en decimales)
        float temperature = temp / 340.0 + 36.53;

        printf("Temperatura = %.2f\n", temperature);

        // Verificar si la temperatura supera el límite y activar LEDs
        bool led_x_on = temperature > TEMP_LIMIT;
        bool led_y_on = temperature > TEMP_LIMIT;

        gpio_put(LED_PIN_X, led_x_on);
        gpio_put(LED_PIN_Y, led_y_on);

        // Activar ventilador si cualquiera de los LEDs está encendido
        if (led_x_on || led_y_on) {
            gpio_put(FAN_PIN, 0); // Encender ventilador
        } else {
            gpio_put(FAN_PIN, 1); // Apagar ventilador
        }

        // Pausa de 100 ms entre lecturas
        sleep_ms(100);
    }
}