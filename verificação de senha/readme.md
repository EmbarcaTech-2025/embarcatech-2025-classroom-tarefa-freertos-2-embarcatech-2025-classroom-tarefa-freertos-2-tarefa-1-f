# cofreRTOS (Segunda Parte)

Este repositório corresponde à **segunda parte do projeto**: um cofre eletrônico com senha de 4 dígitos usando Raspberry Pi Pico W, FreeRTOS e display OLED SSD1306. A **primeira parte** (*embaratech-tarefa-freertos-2*) implementou o processo de **criação e definição da senha** no dispositivo.

## Funcionalidades

* Captura de código de 4 dígitos via botões (A = '1', B = '0').
* Verificação de senha configurada no código-fonte (por padrão `1111`).
* Feedback visual via LED azul (acesso liberado) e vermelho (senha incorreta).
* Feedback sonoro via buzzer PWM.
* Exibição de mensagens no OLED:

  * Prompt de digitação da senha.
  * Mensagens **Acesso Liberado!** e **Senha Incorreta!** centralizadas, exibidas por 3 segundos.

## Hardware Requerido

* Raspberry Pi Pico W
* Display OLED SSD1306 (I2C)
* 2 Botões (GPIO 5 e GPIO 6)
* LED vermelho (GPIO 13)
* LED azul (GPIO 12)
* Buzzer (GPIO 21)
* Resistores de pull-up (internos do Pico ou externos)
* Fios de conexão

## Conexões

| Sinal Pico W   | Periférico     | Comentário      |
| -------------- | -------------- | --------------- |
| GP5            | Botão A        | Pull-up interno |
| GP6            | Botão B        | Pull-up interno |
| GP13           | LED vermelho   | Cathode → GND   |
| GP12           | LED azul       | Cathode → GND   |
| GP21 (PWM)     | Buzzer         | PWM             |
| GP14 (I2C SDA) | SSD1306 SDA    | Pull-up interno |
| GP15 (I2C SCL) | SSD1306 SCL    | Pull-up interno |
| 3V3            | VCC do SSD1306 |                 |
| GND            | GND geral      |                 |

## Software e Build

### Pré-requisitos

* [Pico SDK v2.1.1](https://github.com/raspberrypi/pico-sdk)
* Toolchain ARM (arm-none-eabi-gcc)
* CMake (≥ 3.12)
* Ninja (opcional, mas recomendado)
* FreeRTOS (incluído no repositório)

### Compilando

```bash
git clone --recursive <URL_DO_REPO>
mkdir build && cd build
cmake .. -G Ninja
ninja
```

Conecte seu Pico em modo bootloader e copie `cofreRTOS.uf2` para ele.

## Demonstração em Vídeo

[![Demonstração do Cofre](https://img.youtube.com/vi/NRRydgT8WxE/0.jpg)](https://youtu.be/NRRydgT8WxE)
clique para ver o video no YouTube

## Estrutura

```
/                           # raiz do projeto
├─ app/
│  └── main.c               # código principal
├─ include/
│  └── ssd1306_i2c.c        # driver do display
├─ FreeRTOS-Kernel/         # kernel FreeRTOS
├─ CMakeLists.txt           # configuração de build
└─ README.md                # este arquivo
```

## Uso

1. Pressione os botões A e B para inserir sua senha (A = 1, B = 0).
2. Ao completar 4 dígitos, o sistema verifica automaticamente:

   * **Correta** → LED azul aceso, mensagem “Acesso Liberado!” por 3 s.
   * **Incorreta** → LED vermelho aceso, mensagem “Senha Incorreta!” por 3 s.
3. Após exibir o resultado, retorna ao prompt “Digite a senha:”.

## Customização

* Para alterar a senha, edite a constante `correctCode` em `main.c`.
* Ajuste tempos e comportamentos nas macros e delays do FreeRTOS.

---

Desenvolvido por Karen Beatrice para BitDogLab / EmbarcaTech 2025.
