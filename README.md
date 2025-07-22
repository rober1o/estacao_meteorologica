# ESTAÇÃO METEOROLÓGICA 

Este projeto visa desenvolver uma estação meteorológica interativa que monitora temperatura, 
umidade e pressão em tempo real. Utilizando sensores digitais e a plataforma BitDogLab, os dados 
são exibidos em um display OLED e em uma interface web responsiva. O sistema permite ajustar 
limites e calibrações, emitindo alertas visuais e sonoros sempre que os valores ultrapassarem os pa
râmetros definidos. 

## Componentes Utilizados


1. **Botão Pushbutton**
2. **Display OLED 1306**
3. **Buzzer**
4. **Matriz de LED 5x5 WS2812** 
5. **Led RGB**
6. **Modulo WIFI (CYW4)**
7. **Jumpers**
8. **Extensor i2c**
9. **Sensor BMP280**
10. **sensor AHT10**

## Funcionalidade

Esse sistema de estação meteorológica permite

Visualizar graficamente os dados dos sensores via interface web

Calibrar os sensores via interface web

Definir valores máximos e minimos para cada um dos sensores via interface web

Exibir os dados dos sensores no display OLED

Verificação em tepo real via intereface web e localmente com o display OLED

Monitoramento para alarmes caso os dados não estejam no limite definido pelo usuário

Ao iniciar o sistema pela primeira vez, é necessário configurar a rede Wi-Fi (SSID e senha) para que o dispositivo se conecte à internet. Após a conexão bem-sucedida, o endereço IP será exibido via UART, permitindo o acesso à interface web por esse endereço.

### Como Usar

#### Usando a BitDogLab

- Clone este repositório: git clone https://github.com/rober1o/estacao_meteorologica.git;
- Usando a extensão Raspberry Pi Pico importar o projeto;
- Ajuste a rede wifi e senha 
- Compilar o projeto;
- Plugar a BitDogLab usando um cabo apropriado

## Demonstração
<!-- TODO: adicionar link do vídeo -->
Vídeo demonstrando as funcionalidades da solução implementada: [Demonstração](https://youtu.be/kiLWuSoZEak)
