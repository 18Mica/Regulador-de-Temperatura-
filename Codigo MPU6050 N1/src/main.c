#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "mpu6050.h"
#include "hardware/pwm.h"

#define SDA_GPIO  4
#define SCL_GPIO  5 
#define LED_PIN_X 14 // Led rojo
#define LED_PIN_Y 15 // Led verde
#define FAN_PIN 13   // Ventilador 
#define TEMP_LIMIT 32 // Límite de temperatura

void setup_pwm(uint pin) {
     gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 255); // Rango de 0-255 para ciclo de trabajo
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), 0);
    pwm_set_enabled(slice_num, true);
}

void set_fan_speed(uint pin, float temperature) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint level;

    if (temperature >= TEMP_LIMIT) {
        level = 255; // Velocidad máxima
    } else if (temperature > (TEMP_LIMIT - 5)) {
        // Escalar velocidad gradualmente entre TEMP_LIMIT - 5 y TEMP_LIMIT
        level = (uint)((temperature - (TEMP_LIMIT - 5)) * 51); // 51 = 255/5
    } else {
        level = 0; // Apagar ventilador
    }

    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), level);
}

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
    gpio_set_dir(LED_PIN_X, GPIO_OUT);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    // Configurar PWM para el ventilador
    setup_pwm(FAN_PIN);

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

         // Ajustar velocidad del ventilador según la temperatura
        set_fan_speed(FAN_PIN, temperature);

        // Pausa de 100 ms entre lecturas
        sleep_ms(1000);
    }
}