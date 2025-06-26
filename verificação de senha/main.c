#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "ssd1306.h"

// === PINOS ===
#define BUTTON_A     5
#define BUTTON_B     6
#define LED_VERMELHO 13
#define LED_AZUL     12
#define BUZZER       21
#define I2C_SDA      14
#define I2C_SCL      15

// === CONFIGURAÇÃO ===
#define MAX_CODE_LENGTH 4
static const char correctCode[5] = "1111";

static uint8_t ssd_buffer[ssd1306_buffer_length];
static struct render_area frame_area;

typedef enum { STATE_INPUT, STATE_RESULT } State;
static State currentState = STATE_INPUT;

char enteredCode[5] = "";
int digitIndex = 0;
TickType_t stateStartTick = 0;

// Calcula posição X para centralizar uma string de largura N (6px por caractere)
static inline int center_x(const char *s) {
    return (ssd1306_width - 6 * strlen(s)) / 2;
}

// Inicializa periféricos e display
void init_peripherals() {
    stdio_init_all();
    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT); gpio_put(LED_VERMELHO, 0);
    gpio_init(LED_AZUL);     gpio_set_dir(LED_AZUL,     GPIO_OUT); gpio_put(LED_AZUL,     0);

    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_enabled(slice, true);

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
}

// Toca um tom no buzzer
void play_tone(int freq, int duration_ms) {
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_wrap(slice, 125000000 / freq);
    pwm_set_chan_level(slice, PWM_CHAN_A, (125000000 / freq) / 2);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    pwm_set_chan_level(slice, PWM_CHAN_A, 0);
}

// Exibe duas linhas em posições customizáveis
void show_message(const char* l1, const char* l2, int x1, int y1, int x2, int y2) {
    memset(ssd_buffer, 0, sizeof(ssd_buffer));
    ssd1306_draw_string(ssd_buffer, x1, y1, (char*)l1);
    ssd1306_draw_string(ssd_buffer, x2, y2, (char*)l2);
    render_on_display(ssd_buffer, &frame_area);
}

// Tarefa principal com máquina de estados
void taskMain(void *params) {
    // Prompt inicial
    show_message("Digite a senha:", "", 0, 0, 0, 20);

    while (1) {
        TickType_t now = xTaskGetTickCount();

        switch (currentState) {
            case STATE_INPUT:
                if (digitIndex < MAX_CODE_LENGTH) {
                    if (!gpio_get(BUTTON_A)) {
                        enteredCode[digitIndex++] = '1';
                        play_tone(1000, 100);
                        vTaskDelay(pdMS_TO_TICKS(300));
                    } else if (!gpio_get(BUTTON_B)) {
                        enteredCode[digitIndex++] = '0';
                        play_tone(500, 100);
                        vTaskDelay(pdMS_TO_TICKS(300));
                    }
                    enteredCode[digitIndex] = '\0';
                    char buf[32];
                    snprintf(buf, sizeof(buf), "Codigo: %s", enteredCode);
                    show_message("Digite a senha:", buf, 0, 0, 0, 20);
                }
                if (digitIndex == MAX_CODE_LENGTH) {
                    stateStartTick = now;
                    if (strcmp(enteredCode, correctCode) == 0) {
                        gpio_put(LED_AZUL, 1);
                        int x1 = center_x("Acesso");
                        int x2 = center_x("Liberado!");
                        show_message("Acesso", "Liberado!", x1, 16, x2, 32);
                        play_tone(1200, 200);
                    } else {
                        gpio_put(LED_VERMELHO, 1);
                        int x1 = center_x("Senha");
                        int x2 = center_x("Incorreta!");
                        show_message("Senha", "Incorreta!", x1, 16, x2, 32);
                        play_tone(300, 400);
                    }
                    currentState = STATE_RESULT;
                }
                break;

            case STATE_RESULT:
                // mantém LED aceso por 2s
                if (now - stateStartTick >= pdMS_TO_TICKS(2000)) {
                    gpio_put(LED_AZUL, 0);
                    gpio_put(LED_VERMELHO, 0);
                }
                // total de 3s de mensagem
                if (now - stateStartTick >= pdMS_TO_TICKS(3000)) {
                    digitIndex = 0;
                    enteredCode[0] = '\0';
                    currentState = STATE_INPUT;
                    show_message("Digite a senha:", "", 0, 0, 0, 20);
                }
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main() {
    init_peripherals();
    frame_area.start_column = 0;
    frame_area.end_column   = 127;
    frame_area.start_page   = 0;
    frame_area.end_page     = 7;
    calculate_render_area_buffer_length(&frame_area);

    xTaskCreate(taskMain, "Main", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1) {}
    return 0;
}

