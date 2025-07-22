#ifndef ESTACAO_METEREOLOGICA // Previne múltiplas inclusões do cabeçalho
#define ESTACAO_METEREOLOGICA

// ============================================================================
// === Bibliotecas do SDK do Raspberry Pi Pico ===
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// === Bibliotecas de hardware ===
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

// === Biblioteca de rede (TCP/IP via LWIP) ===
#include "lwip/tcp.h"

// === Bibliotecas padrão da linguagem C ===
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// === Bibliotecas de dispositivos externos ===
#include "ssd1306.h" // Display OLED
#include "aht20.h"   // Sensor de umidade e temperatura
#include "bmp280.h"  // Sensor de pressão e temperatura

// === Bibliotecas auxiliares do projeto ===
#include "font.h"           // Fonte do display
#include "index.h"          // Página HTML para o servidor web
#include "lib/matriz_5X5.h" // Matriz de LEDs 5x5 WS2812
#include "pio_wave.pio.h"   // Programa PIO para buzzer

// ============================================================================
// === Definições de pinos e periféricos ===
#define LED_PIN 12 // LED de status
#define BOTAO_A 5  // Botão A (não utilizado neste trecho)
#define MATRIZ_PIN 7
#define NUM_PIXELS 25
#define BRILHO_PADRAO 50

#define LED_BLUE_PIN 11
#define LED_GREEN_PIN 12
#define LED_RED_PIN 13

#define BUZZER_PIN 10

// === I2C para sensores ===
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

// === I2C para display ===
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define endereco 0x3C // Endereço I2C do SSD1306

// === Parâmetro de referência para altitude ===
#define SEA_LEVEL_PRESSURE 101325.0 // em Pascal

// === Wi-Fi ===
#define WIFI_SSID "BORGES"
#define WIFI_PASS "gomugomu"

// ============================================================================
// === Variáveis globais ===
ssd1306_t ssd;                    // Instância do display OLED
struct bmp280_calib_param params; // Parâmetros de calibração do BMP280

float leitura_temp;    // Temperatura (°C)
float leitura_pressao; // Pressão (Pa)
float leitura_umidade; // Umidade (%)
bool alerta = false;   // Indicador de alerta

PIO pio;        // Instância do PIO
int sm;         // Máquina de estado PIO
uint slice_num; // Canal PWM usado

char ip_str[24]; // IP da rede em formato string

// ============================================================================
// === Protótipos de funções utilitárias ===

// === Funções de cálculo e extração de dados HTTP ===
double calculate_altitude(double pressure); // Calcula altitude com base na pressão
int extrair_valor_offset(const char *req, const char *tipo, float *valor);
int extrair_valores_limite(const char *req, const char *tipo, float *min, float *max);

// === Inicializações gerais ===
void configurar_matriz_leds(void);
void inicializar_pwm_buzzer(void);
void inicializar_leds(void);
void inicializar_botoes(void);
void inicializar_i2c(i2c_inst_t *i2c_port, uint sda, uint scl);
void inicializar_display(ssd1306_t *ssd);
void inicializar_sensores(struct bmp280_calib_param *params);
bool conectar_wifi(ssd1306_t *ssd);

// === Lógica da estação ===
void monitorar_alertas(void);

// === Controle de dispositivos ===
void desenha_fig(uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm);
void tocar_pwm_buzzer(uint duracao_ms, int led_gpio);

// === Interrupções ===
void gpio_irq_handler(uint gpio, uint32_t events);

#endif // ESTACAO_METEREOLOGICA
