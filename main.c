#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Display Oled - SSD1306
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
ssd1306_t ssd;

// Definição das GPIOS e Canais ADC do Joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON_PIN 22
#define ADC_CHANNEL_X0 0
#define ADC_CHANNEL_Y1 1

// Definição das GPIOS dos LEDs
#define LED_RED_PIN 13
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11

// Definição da GPIO do Botão A
#define BUTTON_A_PIN 5

// Definição de variáveis para controle da rotação dos eixos do joystick 
#define JOYSTICK_CENTER 2048
#define JOYSTICK_MAX 4095
#define JOYSTICK_TOLERANCE 205

// Variável para controle do tratamento do Debouncing 
#define DEBOUNCE_TIME_MS 20

// Protótipos das funções utilizadas 
void setupLeds_Button();
void setupJoystick();
void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y);
void setup_pwm(uint pin);
uint16_t map_joystick_value(uint16_t value);
void On_GreenLed();
void button_isr(uint gpio, uint32_t events);
bool debounce_timer_callback(struct repeating_timer *t);
void update_position(int *pos_x, int *pos_y, uint16_t eixo_X, uint16_t eixo_Y, int step, bool *moved);
void draw_square(int pos_x, int pos_y);

// Variáveis para controle dos botões 
volatile bool button_a_pressed = false;
volatile bool joystick_button_pressed = false;
volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_joystick_button_time = 0;
struct repeating_timer debounce_timer;
volatile bool border_visible = false;

// Variável para controle do Display Oled
bool cor = true;

int main() {

    stdio_init_all();
    setupLeds_Button();
    setupJoystick();

    // Configuração (Inicialuzação) do Display Oled
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL); 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd);

    // Variáveis para armazenamentos dos valores dos eixos do Joystick 
    uint16_t valor_X, valor_Y;

    // PWM dos LEDs 
    bool pwm_enabled = true;
    setup_pwm(LED_RED_PIN);
    setup_pwm(LED_BLUE_PIN);
    pwm_set_gpio_level(LED_RED_PIN, 0);
    pwm_set_gpio_level(LED_BLUE_PIN, 0);
    gpio_put(LED_GREEN_PIN, 0);

    // Rotinas de Interrupção 
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    add_repeating_timer_ms(DEBOUNCE_TIME_MS, debounce_timer_callback, NULL, &debounce_timer);

    // Variáveis para desenhodo do quadrado 8x8
    int pos_x = 32;
    int pos_y = 64;
    int step = 2;
    bool moved = true;

    while (true) {
        Joystick_Read(&valor_X, &valor_Y);
        printf("X: %d, Y: %d\n", valor_X, valor_Y);

        update_position(&pos_x, &pos_y, valor_X, valor_Y, step, &moved);
        if (moved) {
            draw_square(pos_x, pos_y);
        }

        uint16_t red_pwm = map_joystick_value(valor_Y);
        uint16_t blue_pwm = map_joystick_value(valor_X);

        if (pwm_enabled) {
            pwm_set_gpio_level(LED_RED_PIN, red_pwm);
            pwm_set_gpio_level(LED_BLUE_PIN, blue_pwm);
        } else {
            pwm_set_gpio_level(LED_RED_PIN, 0);
            pwm_set_gpio_level(LED_BLUE_PIN, 0);
        }

        if (button_a_pressed) {
            pwm_enabled = !pwm_enabled;
            button_a_pressed = false;
        }

        if (joystick_button_pressed) {
            On_GreenLed();
            border_visible = !border_visible;
            ssd1306_fill(&ssd, false);
            if (border_visible) {
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
                ssd1306_rect(&ssd, 4, 4, 122, 58, cor, !cor);
                ssd1306_rect(&ssd, 5, 5, 122, 58, cor, !cor);
            }
            ssd1306_send_data(&ssd);
            joystick_button_pressed = false;
        }

        sleep_ms(10);
    }
}

// Função de inicialização dos LEDs e do Botão 
void setupLeds_Button() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(JOYSTICK_BUTTON_PIN);
    gpio_set_dir(JOYSTICK_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON_PIN);
}

// Função de Inicialização do Joystick 
void setupJoystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

// Função para leitura de valores dos eixos X e Y do Joystick 
void Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y) {
    adc_select_input(ADC_CHANNEL_X0);
    sleep_us(2);
    *eixo_X = adc_read();

    adc_select_input(ADC_CHANNEL_Y1);
    sleep_us(2);
    *eixo_Y = adc_read();
}

// Função de inicialização do PWM
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, JOYSTICK_MAX);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

// Função de configuração da "Zona Morta" do Joystick 
uint16_t map_joystick_value(uint16_t value) {
    if (abs(value - JOYSTICK_CENTER) <= JOYSTICK_TOLERANCE) {
        return 0;
    } else if (value < JOYSTICK_CENTER) {
        return ((JOYSTICK_CENTER - value) * 2);
    } else {
        return ((value - JOYSTICK_CENTER) * 2);
    }
}

// Função para ligar o Led Verde 
void On_GreenLed() {
    static bool led_state = false;
    led_state = !led_state;
    gpio_put(LED_GREEN_PIN, led_state);
}

// Função para movimentação do Quadrado 8x8 
void update_position(int *pos_x, int *pos_y, uint16_t eixo_X, uint16_t eixo_Y, int step, bool *moved) {
    if (eixo_X > JOYSTICK_CENTER + JOYSTICK_TOLERANCE) {
        *pos_x += step;
    } else if (eixo_X < JOYSTICK_CENTER - JOYSTICK_TOLERANCE) {
        *pos_x -= step;
    }
    if (eixo_Y > JOYSTICK_CENTER + JOYSTICK_TOLERANCE) {
        *pos_y += step;
    } else if (eixo_Y < JOYSTICK_CENTER - JOYSTICK_TOLERANCE) {
        *pos_y -= step;
    }
    if (*pos_x < 0) *pos_x = 0;
    if (*pos_x > WIDTH - 8) *pos_x = WIDTH - 8;
    if (*pos_y < 0) *pos_y = 0;
    if (*pos_y > HEIGHT - 8) *pos_y = HEIGHT - 8;

}

// Função para desenho do Quadrado 8x8
void draw_square(int pos_x, int pos_y) {
    
    ssd1306_fill(&ssd, false);  

    // Redesenha a borda se estiver ativa
    if (border_visible) {
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
        ssd1306_rect(&ssd, 4, 4, 122, 58, cor, !cor);
        ssd1306_rect(&ssd, 5, 5, 122, 58, cor, !cor);
    }

    // Desenha o quadrado
    ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, true, true);
    ssd1306_send_data(&ssd);
}

// Função para rotina de interrupção dos Botões 
void button_isr(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && (current_time - last_button_a_time) > DEBOUNCE_TIME_MS) {
        last_button_a_time = current_time;
        button_a_pressed = true;
    }
    
    if (gpio == JOYSTICK_BUTTON_PIN && (current_time - last_joystick_button_time) > DEBOUNCE_TIME_MS) {
        last_joystick_button_time = current_time;
        joystick_button_pressed = true; 
       
    }
}

// Temporizador 
bool debounce_timer_callback(struct repeating_timer *t) {
    if (gpio_get(BUTTON_A_PIN) == 1) {
        button_a_pressed = false;
    }
    if (gpio_get(JOYSTICK_BUTTON_PIN) == 1) {
        joystick_button_pressed = false;
    }
    return true;
}
