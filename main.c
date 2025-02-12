#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

// Definição das GPIOS e Canais ADC do Joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON_PIN 22
#define ADC_CHANNEL_X0 0 // Canal ADC para eixo X
#define ADC_CHANNEL_Y1 1 // Canal ADC para eixo Y

// Definição das GPIOS do LED
#define LED_RED_PIN 13
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11

// Valores do joystick
#define JOYSTICK_CENTER 2048
#define JOYSTICK_MAX 4095
#define JOYSTICK_TOLERANCE 150  // Zona morta ao redor do centro

// Protótipos das Funções utilizadas
void setupLeds();
void setupJoystick();
void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y);
void setup_pwm(uint pin);
uint16_t map_joystick_value(uint16_t value);
void On_GreenLed();

int main() {
    stdio_init_all();
    setupLeds();
    setupJoystick();

    uint16_t valor_X, valor_Y;
    bool button_pressed = false;

    // Configura os pinos dos LEDs como PWM
    setup_pwm(LED_RED_PIN);
    setup_pwm(LED_BLUE_PIN);

    // Garante que os LEDs iniciem desligados
    pwm_set_gpio_level(LED_RED_PIN, 0);
    pwm_set_gpio_level(LED_BLUE_PIN, 0);
    gpio_put(LED_GREEN_PIN, 0);


    while (true) {
        Joystick_Read(&valor_X, &valor_Y);
        printf("X: %d, Y: %d\n", valor_X, valor_Y);

        printf("Botão do Joystick: %s\n", gpio_get(JOYSTICK_BUTTON_PIN) == 0 ? "Pressionado" : "Solto");


        // Mapeia os valores do joystick para o PWM considerando a zona morta
        uint16_t red_pwm = map_joystick_value(valor_X);
        uint16_t blue_pwm = map_joystick_value(valor_Y);

        // Aplica os valores de PWM aos LEDs
        pwm_set_gpio_level(LED_RED_PIN, red_pwm);
        pwm_set_gpio_level(LED_BLUE_PIN, blue_pwm);

        // Verifica o botão do joystick para alternar o LED verde e desligar os outros
        if (gpio_get(JOYSTICK_BUTTON_PIN) == 0) { // Botão pressionado (ativo baixo)
            if (!button_pressed) {
                On_GreenLed();
                pwm_set_gpio_level(LED_RED_PIN, 0);
                pwm_set_gpio_level(LED_BLUE_PIN, 0);
                button_pressed = true;
            }
        } else {
            button_pressed = false;
        }

        sleep_ms(50); // Reduz o delay para melhorar a resposta
    }
}

void setupLeds() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
}

void setupJoystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Inicialização do Botão do Joystick
    gpio_init(JOYSTICK_BUTTON_PIN);
    gpio_set_dir(JOYSTICK_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON_PIN); 

}

void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y) {
    adc_select_input(ADC_CHANNEL_X0);
    sleep_us(2);
    *eixo_X = adc_read();

    adc_select_input(ADC_CHANNEL_Y1);
    sleep_us(2);
    *eixo_Y = adc_read();
}

// Função para configurar o PWM
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, JOYSTICK_MAX);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);  // Garante que o LED inicia apagado
}

uint16_t map_joystick_value(uint16_t value) {
    if (abs(value - JOYSTICK_CENTER) <= JOYSTICK_TOLERANCE) {
        return 0; // LED apagado dentro da zona morta
    } else if (value < JOYSTICK_CENTER) {
        return ((JOYSTICK_CENTER - value) * 2);
    } else {
        return ((value - JOYSTICK_CENTER) * 2);
    }
}

// Função para alternar o LED verde quando o botão do joystick for pressionado
void On_GreenLed() {
    static bool led_state = false;
    led_state = !led_state;
    gpio_put(LED_GREEN_PIN, led_state);
}