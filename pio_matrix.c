// Embarcatec
// Tarefa - Aula Sincrona - 03/02/2025 - reposição (04/02/2025)
// Discente: Alexsami Lopes

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "pico/bootrom.h"

// arquivo .pio
#include "pio_matrix.pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// pino de saída
#define OUT_PIN 7

#define FRAME_SIZE 25

#define PIXEL_INTENSITY 0.2   // 1 equivale a 100% e 0.5 a 50%, se for testar na placa não ponha em 1 pois a intensidade é muito alta e pode prejudicar a visão
#define LED_INTERVAL_TIME 100 // Como são 5 por segundo, 1000/5 = 200, mas como para piscar é chamado 2 vezes 200/2 = 100

// botão de interupção
const uint button_a = 5;
const uint button_b = 6;

const uint ledGreen_pin = 11; // Green=> GPIO11
const uint ledBlue_pin = 12;  // Blue => GPIO12
const uint ledRed_pin = 13;   // Red => GPIO13

// Variáveis globais
static volatile uint a = 0;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
PIO pio = pio0;
uint sm = 0;

static double frames_digitos[10][FRAME_SIZE][3] = {
    // Frame 0
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 1
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 2
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},

    // Frame 3
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 4
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 5
    {{1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},

    // Frame 6
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 7
    {{1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 8
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},

    // Frame 9
    {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}

};

// rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255 * PIXEL_INTENSITY;
  G = g * 255 * PIXEL_INTENSITY;
  B = b * 255 * PIXEL_INTENSITY;
  return (R << 24) | (G << 16) | (B << 8);
}

uint32_t matrix_rgb_dois(double b, double r, double g, double intensity)
{
  unsigned char R, G, B;
  R = r * 255 * intensity;
  G = g * 255 * intensity;
  B = b * 255 * intensity;
  return (R << 24) | (G << 16) | (B << 8);
}

// Função para acionar a matriz de LEDs - ws2812b
void desenho_pio(double frame[FRAME_SIZE][3], uint32_t valor_led, PIO pio, uint sm)
{
  // Ordem desejada dos índices
  int ordem_indices[FRAME_SIZE] = {24, 23, 22, 21, 20, 15, 16, 17, 18, 19, 14, 13, 12, 11, 10, 5, 6, 7, 8, 9, 4, 3, 2, 1, 0};

  // Iterar sobre os índices na ordem desejada
  for (int i = 0; i < FRAME_SIZE; i++)
  {
    int idx = ordem_indices[i];
    double r = frame[idx][0];
    double g = frame[idx][1];
    double b = frame[idx][2];
    valor_led = matrix_rgb(r, g, b);         // Assumindo que esta função converte os valores RGB em um valor de LED
    pio_sm_put_blocking(pio, sm, valor_led); // Envia o valor do LED para o PIO
  }
}

/*
//rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){
    printf("Interrupção ocorreu no pino %d, no evento %d\n", gpio, events);
    printf("HABILITANDO O MODO GRAVAÇÃO");
  reset_usb_boot(0,0); //habilita o modo de gravação do microcontrolador
}*/
// Função para exibir animação das letras
void exibir_animacao_digito(uint digito, PIO pio, uint sm)
{

  if ((digito < 10) && (digito >= 0))
  {
    desenho_pio(frames_digitos[digito], 0, pio, sm);
  }
}

// Função para lidar com a interrupção de ambos os botões
void gpio_irq_handler(uint gpio, uint32_t events)
{

  // Obtém o tempo atual em microssegundos
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  
  // Verifica se passou tempo suficiente desde o último evento
  if (current_time - last_time > 200000) // 200 ms de debouncing
  {
    last_time = current_time; // Atualiza o tempo do último evento
    if (gpio == 5)
    { // Verifica se o botão A foi pressionado
      gpio_put(ledGreen_pin, !gpio_get(ledGreen_pin));
      printf("LED verde alternado!\n");
    }
    if (gpio == 6)
    { // Verifica se o botão B foi pressionado
      gpio_put(ledBlue_pin, !gpio_get(ledBlue_pin));
      printf("LED azul alternado!\n");
    }

    
  }
}

// função principal
int main()
{

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;

  // PIO pio = pio0;
  pio = pio0;
  bool ok;
  uint16_t i;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t linha = 0;
  uint8_t coluna = 0;
  uint32_t valor_led;
  double r = 0.0, b = 0.0, g = 0.0;
  // bool apagar = false;

  // coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
  ok = set_sys_clock_khz(128000, false);

  // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
  stdio_init_all();

  printf("iniciando a transmissão PIO");
  if (ok)
    printf("clock set to %ld\n", clock_get_hz(clk_sys));

  // configurações da PIO
  uint offset = pio_add_program(pio, &pio_matrix_program);
  // uint sm = pio_claim_unused_sm(pio, true);
  sm = pio_claim_unused_sm(pio, true);
  pio_matrix_program_init(pio, sm, offset, OUT_PIN);

  // Inicializar o botão de interrupção - GPIO5
  gpio_init(button_a);
  gpio_set_dir(button_a, GPIO_IN); // Configura GPIO5 como entrada
  gpio_pull_up(button_a);          // Ativa o resistor pull-up interno
  //

  // Inicializar o botão de interrupção - GPIO6
  gpio_init(button_b);
  gpio_set_dir(button_b, GPIO_IN);                                                       // Configura GPIO6 como entrada
  gpio_pull_up(button_b);                                                                // Ativa o resistor pull-up interno
  gpio_set_irq_enabled_with_callback(button_a, GPIO_IRQ_EDGE_FALL, 1, gpio_irq_handler); // Configura a interrupção para borda de descida
  gpio_set_irq_enabled_with_callback(button_b, GPIO_IRQ_EDGE_FALL, 1, gpio_irq_handler); // Configura a interrupção para borda de descida

  gpio_init(ledRed_pin);              // Inicializa o pino do LED
  gpio_set_dir(ledRed_pin, GPIO_OUT); // Configura o pino como saída
  gpio_put(ledRed_pin, 0);            // Inicializa o pino do LED em 0

  gpio_init(ledGreen_pin);              // Inicializa o pino do LED
  gpio_set_dir(ledGreen_pin, GPIO_OUT); // Configura o pino como saída
  gpio_put(ledGreen_pin, 0);            // Inicializa o pino do LED em 0

  gpio_init(ledBlue_pin);              // Inicializa o pino do LED
  gpio_set_dir(ledBlue_pin, GPIO_OUT); // Configura o pino como saída
  gpio_put(ledBlue_pin, 0);            // Inicializa o pino do LED em 0

  exibir_animacao_digito(a, pio, sm);

  printf("RP2040 inicializado. Envie 'r', 'g' ou 'b' para alternar os LEDs.\n");
  ssd1306_fill(&ssd, !cor); // Limpa o display
  while (true)
  {

    if (stdio_usb_connected())
    { // Certifica-se de que o USB está conectado

      char c;
      if (scanf("%c", &c) == 1)
      { // Lê caractere da entrada padrão
        printf("Recebido: '%c'\n", c);

        if (coluna > 15)
        {
          coluna = 0;
          linha++;
        }
        if (linha > 7)
        {
          coluna = 0;
          linha = 0;
          ssd1306_fill(&ssd, !cor); // Limpa o display
        }
        switch (c)
        {
        // Letras maiúsculas
        case 'A':
          ssd1306_draw_string(&ssd, "A", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'B':
          ssd1306_draw_string(&ssd, "B", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'C':
          ssd1306_draw_string(&ssd, "C", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'D':
          ssd1306_draw_string(&ssd, "D", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'E':
          ssd1306_draw_string(&ssd, "E", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'F':
          ssd1306_draw_string(&ssd, "F", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'G':
          ssd1306_draw_string(&ssd, "G", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'H':
          ssd1306_draw_string(&ssd, "H", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'I':
          ssd1306_draw_string(&ssd, "I", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'J':
          ssd1306_draw_string(&ssd, "J", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'K':
          ssd1306_draw_string(&ssd, "K", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'L':
          ssd1306_draw_string(&ssd, "L", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'M':
          ssd1306_draw_string(&ssd, "M", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'N':
          ssd1306_draw_string(&ssd, "N", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'O':
          ssd1306_draw_string(&ssd, "O", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'P':
          ssd1306_draw_string(&ssd, "P", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'Q':
          ssd1306_draw_string(&ssd, "Q", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'R':
          ssd1306_draw_string(&ssd, "R", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'S':
          ssd1306_draw_string(&ssd, "S", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'T':
          ssd1306_draw_string(&ssd, "T", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'U':
          ssd1306_draw_string(&ssd, "U", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'V':
          ssd1306_draw_string(&ssd, "V", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'W':
          ssd1306_draw_string(&ssd, "W", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'X':
          ssd1306_draw_string(&ssd, "X", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'Y':
          ssd1306_draw_string(&ssd, "Y", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'Z':
          ssd1306_draw_string(&ssd, "Z", coluna * 8, linha * 8);
          coluna++;
          break;

        // Letras minúsculas
        case 'a':
          ssd1306_draw_string(&ssd, "a", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'b':
          ssd1306_draw_string(&ssd, "b", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'c':
          ssd1306_draw_string(&ssd, "c", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'd':
          ssd1306_draw_string(&ssd, "d", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'e':
          ssd1306_draw_string(&ssd, "e", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'f':
          ssd1306_draw_string(&ssd, "f", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'g':
          ssd1306_draw_string(&ssd, "g", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'h':
          ssd1306_draw_string(&ssd, "h", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'i':
          ssd1306_draw_string(&ssd, "i", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'j':
          ssd1306_draw_string(&ssd, "j", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'k':
          ssd1306_draw_string(&ssd, "k", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'l':
          ssd1306_draw_string(&ssd, "l", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'm':
          ssd1306_draw_string(&ssd, "m", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'n':
          ssd1306_draw_string(&ssd, "n", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'o':
          ssd1306_draw_string(&ssd, "o", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'p':
          ssd1306_draw_string(&ssd, "p", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'q':
          ssd1306_draw_string(&ssd, "q", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'r':
          ssd1306_draw_string(&ssd, "r", coluna * 8, linha * 8);
          coluna++;
          break;
        case 's':
          ssd1306_draw_string(&ssd, "s", coluna * 8, linha * 8);
          coluna++;
          break;
        case 't':
          ssd1306_draw_string(&ssd, "t", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'u':
          ssd1306_draw_string(&ssd, "u", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'v':
          ssd1306_draw_string(&ssd, "v", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'w':
          ssd1306_draw_string(&ssd, "w", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'x':
          ssd1306_draw_string(&ssd, "x", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'y':
          ssd1306_draw_string(&ssd, "y", coluna * 8, linha * 8);
          coluna++;
          break;
        case 'z':
          ssd1306_draw_string(&ssd, "z", coluna * 8, linha * 8);
          coluna++;
          break;

        // Números de 0 a 9
        case '0':
          ssd1306_draw_string(&ssd, "0", coluna * 8, linha * 8);
          exibir_animacao_digito(0, pio, sm);
          coluna++;
          break;
        case '1':
          ssd1306_draw_string(&ssd, "1", coluna * 8, linha * 8);
          exibir_animacao_digito(1, pio, sm);
          coluna++;
          break;
        case '2':
          ssd1306_draw_string(&ssd, "2", coluna * 8, linha * 8);
          exibir_animacao_digito(2, pio, sm);
          coluna++;
          break;
        case '3':
          ssd1306_draw_string(&ssd, "3", coluna * 8, linha * 8);
          exibir_animacao_digito(3, pio, sm);
          coluna++;
          break;
        case '4':
          ssd1306_draw_string(&ssd, "4", coluna * 8, linha * 8);
          exibir_animacao_digito(4, pio, sm);
          coluna++;
          break;
        case '5':
          ssd1306_draw_string(&ssd, "5", coluna * 8, linha * 8);
          exibir_animacao_digito(5, pio, sm);
          coluna++;
          break;
        case '6':
          ssd1306_draw_string(&ssd, "6", coluna * 8, linha * 8);
          exibir_animacao_digito(6, pio, sm);
          coluna++;
          break;
        case '7':
          ssd1306_draw_string(&ssd, "7", coluna * 8, linha * 8);
          exibir_animacao_digito(7, pio, sm);
          coluna++;
          break;
        case '8':
          ssd1306_draw_string(&ssd, "8", coluna * 8, linha * 8);
          exibir_animacao_digito(8, pio, sm);
          coluna++;
          break;
        case '9':
          ssd1306_draw_string(&ssd, "9", coluna * 8, linha * 8);
          exibir_animacao_digito(9, pio, sm);
          coluna++;
          break;

        default:
          printf("Comando inválido: '%c'\n", c);
        }
        ssd1306_send_data(&ssd); // Atualiza o display
      }
    }

    sleep_ms(LED_INTERVAL_TIME);
  }
  return 0;
}