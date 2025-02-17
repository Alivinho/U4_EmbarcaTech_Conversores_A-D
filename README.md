# Controle de LEDs e Display OLED com Joystick no Raspberry Pi Pico W

## Descrição do Projeto

Este projeto implementa um sistema de controle de LEDs RGB e um display OLED SSD1306 utilizando um joystick analógico no Raspberry Pi Pico W. O joystick é usado para ajustar a intensidade dos LEDs e movimentar um quadrado na tela do display. Além disso, o sistema inclui um botão para alternar o funcionamento do PWM dos LEDs e um sistema de debounce para garantir a estabilidade da leitura dos botões.

## Componentes Utilizados

- **Microcontrolador:** Raspberry Pi Pico W
- **Display:** OLED SSD1306
- **Joystick:** Analógico com botão integrado
- **LEDs:** RGB (vermelho, azul e verde)
- **Botão:** Para controle do PWM dos LEDs
- **Resistores e fios de conexão**

## Estrutura do Código

O projeto está estruturado da seguinte forma:

### Definição de Pinos e Constantes
- **GPIOs** dos LEDs, joystick e botões são definidos.
- **Canais ADC** do joystick para leitura dos eixos X e Y.
- **Parâmetros de zona morta** para evitar movimentos indesejados do joystick.
- **Tempo de debounce** para garantir leituras confiáveis dos botões.

### Funções Principais

- **`setupLeds_Button()`**: Configura os LEDs e os botões.
- **`setupJoystick()`**: Inicializa o ADC para leitura do joystick.
- **`Joystick_Read(uint16_t *eixo_X, uint16_t *eixo_Y)`**: Lê os valores dos eixos do joystick.
- **`setup_pwm(uint pin)`**: Inicializa o PWM para controle dos LEDs.
- **`map_joystick_value(uint16_t value)`**: Converte os valores do joystick para uma escala adequada ao PWM.
- **`On_GreenLed()`**: Alterna o estado do LED verde.
- **`update_position(int *pos_x, int *pos_y, uint16_t eixo_X, uint16_t eixo_Y, int step, bool *moved)`**: Atualiza a posição do quadrado na tela OLED.
- **`draw_square(int pos_x, int pos_y)`**: Renderiza o quadrado no display OLED.
- **`button_isr(uint gpio, uint32_t events)`**: Rotina de interrupção para os botões.
- **`debounce_timer_callback(struct repeating_timer *t)`**: Temporizador para evitar leituras falsas de botões.

### Loop Principal
1. Lê os valores do joystick.
2. Atualiza a posição do quadrado no display.
3. Ajusta os LEDs de acordo com os valores do joystick.
4. Processa eventos de botão para alternar o PWM e ativar a borda no display.
5. Aguarda um curto intervalo para estabilidade do sistema.

## Como Compilar e Executar

1. Certifique-se de ter o **Raspberry Pi Pico SDK** instalado.
2. Clone ou copie o código-fonte para seu ambiente de desenvolvimento.
3. Compile o código utilizando o CMake:
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```
4. Envie o arquivo `.uf2` gerado para o Raspberry Pi Pico W.

