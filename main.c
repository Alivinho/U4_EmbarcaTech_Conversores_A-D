#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

// Definção das GPIOS e Canais ADC do Joystick 
#define JOYSTICK_X_PIN 26  
#define JOYSTICK_Y_PIN 27  
#define JOYSTICK_BUTTON_PIN 22 
#define ADC_CHANNEL_X0 0 // Canal ADC para eixo X
#define ADC_CHANNEL_Y1 1 // Canal ADC para eixo Y


// Definição das GPIOS do LED
#define LED_RED_PIN 13
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11

// Protótipos das Funções utilizadas
void setupLeds();
void setupJoystick();
void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y);


int main(){

    stdio_init_all();
    setupLeds();
    setupJoystick();

    uint16_t valor_X, valor_Y; // Armazenar os valores do Joystick (Eixo X e Y)


    while (true) {

        Joystick_Read(&valor_X, &valor_Y);
        printf("X: %d\n", valor_X);
        printf("Y: %d\n", valor_Y);
        
        sleep_ms(1000);
    }
}

void setupLeds(){
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
}

void setupJoystick(){
    adc_init();

    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Inicialização do Botão do Joystick 
    gpio_init(JOYSTICK_BUTTON_PIN);
    gpio_set_dir(JOYSTICK_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON_PIN);
}

void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y){
    adc_select_input(ADC_CHANNEL_X0);
    sleep_us(2);
    *eixo_X = adc_read();

    adc_select_input(ADC_CHANNEL_Y1);
    sleep_us(2);
    *eixo_Y = adc_read();


}
