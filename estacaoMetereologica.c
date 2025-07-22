#include "estacaoMetereologica.h"

// Offsets de calibração para temperatura, pressão e umidade
float offset_temp = 0.0f, offset_pressao = 0.0f, offset_umidade = 0.0f;

// Limites aceitáveis para temperatura, pressão e umidade
float min_temp = -50.0f, max_temp = 50.0f;
float min_pressao = 900.0f, max_pressao = 1100.0f;
float min_umidade = 0.0f, max_umidade = 100.0f;

// Função que extrai um valor float da URL para o offset, com base no tipo (temp, pressao, umidade)
// req: string da requisição HTTP recebida
// tipo: string que indica qual offset procurar (ex: "temp")
// valor: ponteiro onde o valor extraído será armazenado
// Retorna 1 se conseguiu extrair o valor, 0 caso contrário
int extrair_valor_offset(const char *req, const char *tipo, float *valor)
{
    char pattern[64];
    // Monta o padrão de busca na URL, ex: "GET /offset/temp/"
    snprintf(pattern, sizeof(pattern), "GET /offset/%s/", tipo);
    // Procura o padrão na string da requisição
    const char *p = strstr(req, pattern);
    if (p)
    {
        // Avança o ponteiro para a posição logo após o padrão encontrado
        p += strlen(pattern);
        // Tenta ler um float a partir dessa posição
        return sscanf(p, "%f", valor) == 1;
    }
    // Caso padrão não encontrado, retorna 0
    return 0;
}

// Função que extrai valores mínimos e máximos da URL para os limites, com base no tipo
// req: string da requisição HTTP recebida
// tipo: string que indica qual limite procurar (ex: "temp")
// min, max: ponteiros para armazenar os valores extraídos
// Retorna 1 se conseguiu extrair ambos os valores, 0 caso contrário
int extrair_valores_limite(const char *req, const char *tipo, float *min, float *max)
{
    char pattern[64];
    // Monta o padrão de busca na URL, ex: "GET /limites/temp/min/"
    snprintf(pattern, sizeof(pattern), "GET /limites/%s/min/", tipo);
    // Procura o padrão na string da requisição
    const char *p = strstr(req, pattern);
    if (p)
    {
        // Avança o ponteiro para posição após o padrão encontrado
        p += strlen(pattern);
        // Tenta ler dois floats no formato "%f/max/%f"
        return sscanf(p, "%f/max/%f", min, max) == 2;
    }
    // Caso padrão não encontrado, retorna 0
    return 0;
}

// Estrutura para manter o estado da resposta HTTP enquanto é enviada via TCP
struct http_state
{
    char response[12000]; // buffer da resposta HTTP a ser enviada
    size_t len;           // comprimento total da resposta
    size_t sent;          // quantidade já enviada
};

// Callback chamado quando dados HTTP foram enviados pela TCP
// arg: ponteiro para o estado HTTP
// tpcb: ponteiro para o PCB TCP (controle da conexão)
// len: quantidade de bytes enviados nessa chamada
// Retorna ERR_OK para indicar sucesso
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    // Atualiza quantos bytes já foram enviados
    hs->sent += len;
    // Se enviou toda a resposta, fecha conexão e libera memória do estado
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    // Se não recebeu dados (p == NULL), fecha a conexão TCP
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Ponteiro para o payload da requisição HTTP
    char *req = (char *)p->payload;

    // Aloca memória para o estado HTTP da resposta
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs)
    {
        // Falha na alocação, libera o pbuf e fecha conexão
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0; // Inicializa bytes enviados com zero

    // Ponteiro para mensagem de texto de resposta (se aplicável)
    const char *txt = NULL;

    // Variáveis auxiliares para capturar valores extraídos da URL
    float val1, val2;

    // Verifica se a requisição atualiza o offset de temperatura
    if (extrair_valor_offset(req, "temp", &val1))
    {
        offset_temp = val1;
        txt = "Offset de temperatura atualizado";
    }
    // Atualiza offset de pressão
    else if (extrair_valor_offset(req, "pressao", &val1))
    {
        offset_pressao = val1;
        txt = "Offset de pressão atualizado";
    }
    // Atualiza offset de umidade
    else if (extrair_valor_offset(req, "umidade", &val1))
    {
        offset_umidade = val1;
        txt = "Offset de umidade atualizado";
    }
    // Atualiza limites de temperatura
    else if (extrair_valores_limite(req, "temp", &val1, &val2))
    {
        min_temp = val1;
        max_temp = val2;
        txt = "Limites de temperatura atualizados";
    }
    // Atualiza limites de pressão (note que na extração usa "press")
    else if (extrair_valores_limite(req, "press", &val1, &val2))
    {
        min_pressao = val1;
        max_pressao = val2;
        txt = "Limites de pressão atualizados";
    }
    // Atualiza limites de umidade (usa "umid")
    else if (extrair_valores_limite(req, "umid", &val1, &val2))
    {
        min_umidade = val1;
        max_umidade = val2;
        txt = "Limites de umidade atualizados";
    }
    // Se a requisição for para consultar o estado atual (JSON)
    else if (strstr(req, "GET /estado"))
    {
        // Monta payload JSON com os dados atuais das leituras e configurações
        char json_payload[512];
        int json_len = snprintf(json_payload, sizeof(json_payload),
                                "{"
                                "\"x\":%.2f,"
                                "\"y\":%.2f,"
                                "\"z\":%.2f,"
                                "\"offset_temp\":%.2f,"
                                "\"offset_pressao\":%.2f,"
                                "\"offset_umidade\":%.2f,"
                                "\"min_temp\":%.2f,"
                                "\"max_temp\":%.2f,"
                                "\"min_press\":%.2f,"
                                "\"max_press\":%.2f,"
                                "\"min_umid\":%.2f,"
                                "\"max_umid\":%.2f"
                                "}",
                                leitura_temp,
                                leitura_pressao,
                                leitura_umidade,
                                offset_temp,
                                offset_pressao,
                                offset_umidade,
                                min_temp,
                                max_temp,
                                min_pressao,
                                max_pressao,
                                min_umidade,
                                max_umidade);

        // Prepara cabeçalho HTTP e payload JSON para resposta
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           json_len, json_payload);

        // Vai direto para o envio da resposta
        goto send_response;
    }
    else
    {
        // Se não for nenhuma das rotas acima, retorna a página HTML padrão
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(HTML_BODY), HTML_BODY);

        // Envia resposta HTML
        goto send_response;
    }

    // Se txt foi definido (para rotas de atualização de offset/limite), responde texto simples
    if (txt)
    {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(txt), txt);
    }

send_response:
    // Associa o estado HTTP à conexão TCP para callbacks
    tcp_arg(tpcb, hs);
    // Registra callback para envio de dados TCP
    tcp_sent(tpcb, http_sent);

    // Envia os dados da resposta pela conexão TCP
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    // Libera buffer da requisição
    pbuf_free(p);

    return ERR_OK;
}

// Callback chamado quando uma nova conexão TCP é aceita
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    // Define a função de callback para receber dados dessa conexão TCP
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Função que inicia o servidor HTTP na porta 80
static void start_http_server(void)
{
    // Cria um novo PCB TCP (Protocolo Control Block)
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    // Vincula o PCB TCP à porta 80, qualquer IP
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    // Coloca o PCB para escutar conexões entrantes
    pcb = tcp_listen(pcb);
    // Define o callback para aceitar novas conexões TCP
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

#include "pico/bootrom.h"
#define BOTAO_B 6

// Handler para interrupção do GPIO associado ao botão B
void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Reinicia o dispositivo para modo boot USB (bootrom) ao pressionar botão B
    reset_usb_boot(0, 0);
}

// Configura o PIO para uso com matriz de LEDs 5x5
void configurar_matriz_leds()
{
    // Configura o clock do sistema para 133 MHz (133000 kHz)
    bool clock_setado = set_sys_clock_khz(133000, false);

    // Define o PIO a ser usado (PIO0)
    pio = pio0;

    // Reivindica um state machine livre do PIO para uso exclusivo
    sm = pio_claim_unused_sm(pio, true);

    // Adiciona o programa da matriz 5x5 ao PIO e obtém o offset na memória do PIO
    uint offset = pio_add_program(pio, &Matriz_5x5_program);

    // Inicializa o state machine com o programa e pino definido (MATRIZ_PIN)
    Matriz_5x5_program_init(pio, sm, offset, MATRIZ_PIN);
}

// Inicializa PWM para controle do buzzer
void inicializar_pwm_buzzer()
{
    // Define a função do pino do buzzer como PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Obtém o slice do PWM correspondente ao pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Configura o divisor do clock do PWM para reduzir frequência base para 12.5 MHz
    pwm_set_clkdiv(slice_num, 10.0f);

    // Define o wrap do PWM para gerar uma frequência de 400 Hz
    pwm_set_wrap(slice_num, 31250); // 12.5 MHz / 31250 = 400 Hz

    // Configura duty cycle em 50% para o canal PWM do pino do buzzer
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BUZZER_PIN), 15625);

    // Desliga o PWM, buzzer começa desligado
    pwm_set_enabled(slice_num, false);
}

// Função para desenhar na matriz
void desenha_fig(uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm) // FUNÇÃO PARA DESENHAR O SEMAFORO NA MATRIZ
{
    uint32_t pixel = 0;
    uint8_t r, g, b;

    for (int i = 24; i > 19; i--) // Linha 1
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 15; i < 20; i++) // Linha 2
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (b << 16) | (r << 8) | g;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 14; i > 9; i--) // Linha 3
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 5; i < 10; i++) // Linha 4
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 4; i > -1; i--) // Linha 5
    {
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }
}

// Função para tocar o buzzer com PWM por uma duração determinada e acender um LED associado
// duracao_ms: tempo em milissegundos para buzzer tocar
// led_gpio: pino GPIO do LED a acender; -1 para nenhum LED
void tocar_pwm_buzzer(uint duracao_ms, int led_gpio)
{
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true); // Liga o PWM do buzzer

    // Inicializa e acende o LED se led_gpio válido
    gpio_init(led_gpio);
    gpio_set_dir(led_gpio, GPIO_OUT);
    gpio_put(led_gpio, 1); // Acende o LED

    sleep_ms(duracao_ms); // Aguarda o tempo determinado

    // Desliga o buzzer e apaga o LED
    pwm_set_enabled(slice_num, false);
    gpio_put(led_gpio, 0); // Apaga o LED
}

// Função que monitora os alertas baseados nas leituras e limites definidos
void monitorar_alertas()
{
    alerta = false;         // Inicializa flag de alerta
    int alertas_ativos = 0; // Contador de alertas ativos
    int led_alerta = -1;    // Pino do LED que será acionado (inicialmente nenhum)

    // Verifica se temperatura está fora dos limites
    if (leitura_temp < min_temp || leitura_temp > max_temp)
    {
        alerta = true;
        alertas_ativos++;
        led_alerta = LED_RED_PIN; // LED vermelho para temperatura
    }

    // Verifica se pressão está fora dos limites
    if (leitura_pressao < min_pressao || leitura_pressao > max_pressao)
    {
        alerta = true;
        alertas_ativos++;
        led_alerta = LED_GREEN_PIN; // LED verde para pressão
    }

    // Verifica se umidade está fora dos limites
    if (leitura_umidade < min_umidade || leitura_umidade > max_umidade)
    {
        alerta = true;
        alertas_ativos++;
        led_alerta = LED_BLUE_PIN; // LED azul para umidade
    }

    // Se algum alerta está ativo
    if (alerta)
    {
        if (alertas_ativos == 1)
        {
            // Apenas 1 alerta: tocar buzzer e piscar LED correspondente
            tocar_pwm_buzzer(500, led_alerta);
        }
        else
        {
            // Vários alertas: apenas tocar buzzer, sem LED
            tocar_pwm_buzzer(500, -1);

            // Acende matriz LED com figura de "luz quarto"
            desenha_fig(luz_quarto, BRILHO_PADRAO, pio, sm);
            sleep_ms(500);

            // Apaga matriz LED
            desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);
        }
    }
}

// Inicializa os LEDs configurando-os como saída e desligados
void inicializar_leds()
{
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, false); // Apaga LED azul

    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, false); // Apaga LED verde

    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, false); // Apaga LED vermelho
}

// Inicializa o botão BOTAO_B como entrada com pull-up e configura interrupção na borda de descida
void inicializar_botoes()
{
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
}

// Inicializa o barramento I2C com frequência de 400kHz e configura os pinos SDA e SCL
void inicializar_i2c(i2c_inst_t *i2c_port, uint sda, uint scl)
{
    i2c_init(i2c_port, 400 * 1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
}

// Inicializa o display OLED SSD1306 e mostra mensagem inicial
void inicializar_display(ssd1306_t *ssd)
{
    ssd1306_init(ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT_DISP);
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false); // Limpa tela
    ssd1306_draw_string(ssd, "Iniciando Wi-Fi", 0, 0);
    ssd1306_draw_string(ssd, "Aguarde...", 0, 30);
    ssd1306_send_data(ssd);
}

// Inicializa sensores BMP280 e AHT20 na interface I2C
void inicializar_sensores(struct bmp280_calib_param *params)
{
    bmp280_init(I2C_PORT);
    bmp280_get_calib_params(I2C_PORT, params);

    aht20_reset(I2C_PORT);
    aht20_init(I2C_PORT);
}

// Função para conectar à rede Wi-Fi utilizando o chip CYW43 e exibir status no display SSD1306
// Parâmetro: ponteiro para estrutura do display SSD1306 onde as mensagens serão exibidas
// Retorna true se a conexão foi bem sucedida, false caso contrário
bool conectar_wifi(ssd1306_t *ssd)
{
    // Inicializa o driver/arquitetura CYW43 responsável pela interface Wi-Fi
    // Retorna diferente de zero em caso de falha
    if (cyw43_arch_init())
    {
        // Se falhar na inicialização do Wi-Fi, limpa a tela do display e exibe mensagem de falha
        ssd1306_fill(ssd, false);
        ssd1306_draw_string(ssd, "WiFi => FALHA", 0, 0);
        ssd1306_send_data(ssd);
        return false; // Indica falha
    }

    // Configura o chip Wi-Fi para o modo estação (modo cliente)
    cyw43_arch_enable_sta_mode();

    // Tenta conectar na rede Wi-Fi usando SSID e senha, com timeout de 10 segundos
    // Retorna diferente de zero em caso de erro na conexão
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        // Se falhar na conexão, limpa tela e exibe mensagem de erro
        ssd1306_fill(ssd, false);
        ssd1306_draw_string(ssd, "WiFi => ERRO", 0, 0);
        ssd1306_send_data(ssd);
        return false; // Indica falha
    }

    // Recupera o endereço IP atribuído à interface após conexão bem sucedida
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);

    // Converte endereço IP binário para string decimal com pontos
    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    // Limpa o display e mostra mensagem de sucesso com o IP obtido
    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, "WiFi => OK", 0, 0);
    ssd1306_draw_string(ssd, ip_str, 0, 10);
    ssd1306_send_data(ssd);

    // Retorna true indicando que conexão foi estabelecida com sucesso
    return true;
}

int main()
{
    // Inicializa stdio (UART) para debug/console
    stdio_init_all();
    sleep_ms(2000); // Aguarda 2 segundos para estabilidade

    // Inicializa botões, barramentos I2C para display e sensores
    inicializar_botoes();
    inicializar_i2c(I2C_PORT_DISP, I2C_SDA_DISP, I2C_SCL_DISP);
    inicializar_i2c(I2C_PORT, I2C_SDA, I2C_SCL);

    // Configura matriz de LEDs (via PIO) e PWM do buzzer
    configurar_matriz_leds();
    inicializar_pwm_buzzer();
    inicializar_leds(); // LEDs indicadoras

    // Inicializa display OLED SSD1306
    ssd1306_t ssd;
    inicializar_display(&ssd);

    // Inicializa sensores BMP280 e AHT20 (sensor de temperatura/pressão e umidade)
    struct bmp280_calib_param params;
    inicializar_sensores(&params);

    // Conecta à rede Wi-Fi, mostra status no display; aborta se falhar
    if (!conectar_wifi(&ssd))
    {
        return 1;
    }

    // Inicia servidor HTTP para receber comandos e enviar estado
    start_http_server();

    // Variáveis auxiliares para leitura dos sensores e formatação de strings
    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;

    char str_tmp1[5], str_tmp2[5];
    char str_umi[5];
    char str_alt[5];
    char str_x[5], str_y[5];
    bool cor = true; // Usado para inversão de cores no display para piscar

    // Apaga matriz de LEDs ao iniciar
    desenha_fig(matriz_apagada, BRILHO_PADRAO, pio, sm);

    // Loop principal infinito
    while (true)
    {
        // Processa eventos da pilha Wi-Fi CYW43 (necessário para manter conexão)
        cyw43_arch_poll();

        // Lê valores brutos do BMP280 (temperatura e pressão)
        int32_t raw_temp_bmp, raw_pressure;
        bmp280_read_raw(I2C_PORT, &raw_temp_bmp, &raw_pressure);

        // Converte valores brutos para temperatura em centésimos de grau e pressão em Pa
        int32_t temperature_bmp = bmp280_convert_temp(raw_temp_bmp, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);

        // Lê dados do sensor AHT20 (temperatura e umidade)
        AHT20_Data data;
        bool aht_ok = aht20_read(I2C_PORT, &data);

        // Extrai temperatura do AHT20 (0 se erro)
        float temp_aht = aht_ok ? data.temperature : 0.0f;

        // Converte temperatura BMP280 para graus Celsius
        float temp_bmp = temperature_bmp / 100.0f;

        // Calcula a média das temperaturas dos dois sensores, aplica offset configurado
        leitura_temp = ((temp_bmp + temp_aht) / 2.0f) + offset_temp;

        // Atualiza pressão e aplica offset configurado
        leitura_pressao = (pressure / 100.0f) + offset_pressao;

        // Atualiza umidade (0 se erro no sensor), aplica offset configurado
        leitura_umidade = aht_ok ? data.humidity + offset_umidade : 0.0f;

        // Formata strings para mostrar no display
        sprintf(str_tmp1, "%.1fC", temp_bmp);                        // Temperatura BMP280
        sprintf(str_alt, "%.0fhPa", leitura_pressao);                // Pressão atmosférica
        sprintf(str_tmp2, aht_ok ? "%.1fC" : "--", temp_aht);        // Temperatura AHT20 ou "--"
        sprintf(str_umi, aht_ok ? "%.1f%%" : "--", leitura_umidade); // Umidade ou "--"

        // Atualiza display OLED com informações formatadas
        ssd1306_fill(&ssd, !cor);                     // Preenche fundo com cor invertida
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);      // Linhas divisórias
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);

        ssd1306_draw_string(&ssd, ip_str, 15, 12);          // IP da rede Wi-Fi
        ssd1306_draw_string(&ssd, "BMP280  AHT10", 10, 28); // Cabeçalho sensores
        ssd1306_line(&ssd, 63, 25, 63, 60, cor);            // Linha vertical divisória
        ssd1306_draw_string(&ssd, str_tmp1, 14, 41);        // Temp BMP280
        ssd1306_draw_string(&ssd, str_alt, 14, 52);         // Pressão
        ssd1306_draw_string(&ssd, str_tmp2, 73, 41);        // Temp AHT20
        ssd1306_draw_string(&ssd, str_umi, 73, 52);         // Umidade

        ssd1306_send_data(&ssd); // Envia dados para o display

        // Monitora os alertas baseados nos limites e aciona buzzer/LEDs se necessário
        monitorar_alertas();

        sleep_ms(500); // Espera 500ms antes de nova leitura
    }

    // Finaliza o driver Wi-Fi (não alcançado neste código)
    cyw43_arch_deinit();
    return 0;
}
