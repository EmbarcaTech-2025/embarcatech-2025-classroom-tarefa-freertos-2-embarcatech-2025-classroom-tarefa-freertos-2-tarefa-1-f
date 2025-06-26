//melhor até agora
//falta so aumentar o tempo que as mensagens ficam na tela 
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "ssd1306.h"

// === DEFINIÇÕES DE PINOS (BitDogLab) ===
#define BUTTON_A     5
#define BUTTON_B     6
#define LED_VERMELHO 13
#define LED_AZUL     12
#define BUZZER       21
#define I2C_SDA      14
#define I2C_SCL      15

// === DEFINIÇÕES GERAIS ===
#define MAX_CODE_LENGTH 4
static const char correctCode[5] = "1111";

char enteredCode[5] = "";
int digitIndex = 0;
bool resetMensagem = true;

static uint8_t ssd_buffer[ssd1306_buffer_length];
static struct render_area frame_area;

// === INICIALIZAÇÕES ===
void init_peripherals() {
    stdio_init_all();

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, 0);

    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_enabled(slice_num, true);

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
}

// === FUNÇÕES AUXILIARES ===
void play_tone(int freq, int duration_ms) {
    uint slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_wrap(slice, 125000000 / freq);
    pwm_set_chan_level(slice, PWM_CHAN_A, (125000000 / freq) / 2);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    pwm_set_chan_level(slice, PWM_CHAN_A, 0);
}

void show_message(const char* line1, const char* line2, int x1, int y1, int x2, int y2) {
    memset(ssd_buffer, 0, sizeof(ssd_buffer));
    ssd1306_draw_string(ssd_buffer, x1, y1, (char*)line1);
    ssd1306_draw_string(ssd_buffer, x2, y2, (char*)line2);
    render_on_display(ssd_buffer, &frame_area);
}

// === TAREFAS ===
void taskCaptureCode(void *params) {
    while (1) {
        if (digitIndex < MAX_CODE_LENGTH) {
            if (!gpio_get(BUTTON_A)) {
                enteredCode[digitIndex] = '1';
                digitIndex++;
                play_tone(1000, 100);
                vTaskDelay(pdMS_TO_TICKS(300));
            } else if (!gpio_get(BUTTON_B)) {
                enteredCode[digitIndex] = '0';
                digitIndex++;
                play_tone(500, 100);
                vTaskDelay(pdMS_TO_TICKS(300));
            }

            enteredCode[digitIndex] = '\0';

            if (resetMensagem) {
                show_message("Digite a senha:", "", 0, 0, 0, 0);
                resetMensagem = false;
            }

            char displayText[32];
            snprintf(displayText, sizeof(displayText), "Codigo: %s", enteredCode);
            show_message("Digite a senha:", displayText, 0, 0, 0, 20);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void taskVerifyCode(void *params) {
    while (1) {
        if (digitIndex == MAX_CODE_LENGTH) {
            if (strcmp(enteredCode, correctCode) == 0) {
                gpio_put(LED_AZUL, 1);
                show_message("✅ Acesso", "Liberado!", 30, 0, 20, 20);
                play_tone(1200, 200);
            } else {
                gpio_put(LED_VERMELHO, 1);
                show_message("❌ Senha", "Incorreta!", 30, 0, 20, 20);
                play_tone(300, 400);
            }

            vTaskDelay(pdMS_TO_TICKS(2000));
            gpio_put(LED_VERMELHO, 0);
            gpio_put(LED_AZUL, 0);

            digitIndex = 0;
            memset(enteredCode, 0, sizeof(enteredCode));
            resetMensagem = true;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// === MAIN ===
int main() {
    init_peripherals();

    frame_area.start_column = 0;
    frame_area.end_column = 127;
    frame_area.start_page = 0;
    frame_area.end_page = 7;
    calculate_render_area_buffer_length(&frame_area);

    show_message("Digite a senha:", "", 0, 0, 0, 0);

    xTaskCreate(taskCaptureCode, "Capture", 1024, NULL, 1, NULL);
    xTaskCreate(taskVerifyCode, "Verify", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1) {}
    return 0;
}
