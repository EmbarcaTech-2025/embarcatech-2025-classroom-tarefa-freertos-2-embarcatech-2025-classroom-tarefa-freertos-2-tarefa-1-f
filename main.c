#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"

// Definição dos pinos
#define I2C_SDA 14
#define I2C_SCL 15
#define LED_GREEN 11
#define LED_BLUE  12
#define LED_RED   13
#define BUZZER_PIN  10
#define BUTTON_A    5
#define BUTTON_B    6

#define MAX_PASSWORD_LENGTH 4

// Variáveis globais para a senha
uint8_t password[MAX_PASSWORD_LENGTH]; // Array que guarda a senha
uint8_t password_length = 0;
bool password_defined = false;
char senha_atual[5] = "";  // Array para senha (4 caracteres + terminador)

// variávreis para o painel
static struct render_area frame_area;
static uint8_t ssd_buffer[ssd1306_buffer_length];


// Handles globais para controle das tasks
TaskHandle_t xPasswordSetupHandle = NULL;
TaskHandle_t xAutenticaHandle = NULL;

// Função para iniciar i2c
void init_i2c() {
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Função para configurar o PWM do buzzer
void setup_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_wrap(slice_num, 32768);
    pwm_set_enabled(slice_num, true);
}

// Função para tocar um tom específico
void play_tone(int frequency, int duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_clkdiv(slice_num, 125.0f);
    pwm_set_wrap(slice_num, 1000000/frequency - 1);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, (1000000/frequency)/2);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
}

void setup_buttons() {
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
}

void password_setup_task(void *pvParameters) {
    setup_buttons();
    setup_buzzer();
    init_i2c();
    ssd1306_init();

    frame_area.start_column = 0;
    frame_area.end_column = 127;
    frame_area.start_page = 0;
    frame_area.end_page = 7;
    
    calculate_render_area_buffer_length(&frame_area);
    
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    
    memset(ssd_buffer, 0, ssd1306_buffer_length);
    char msg[64];
    senha_atual[4] = '\0';     // Garante terminação da string
    
    snprintf(msg, sizeof(msg), "Digite a senha:");
    ssd1306_draw_string(ssd_buffer, 0, 0, msg);
    render_on_display(ssd_buffer, &frame_area);
    
    while (true) {
        if (!password_defined) {
            if (!gpio_get(BUTTON_A) && password_length < MAX_PASSWORD_LENGTH) {
                password[password_length] = 0;
                senha_atual[password_length] = 'A';
                password_length++;
                
                // Atualiza display com efeito de digitação
                memset(ssd_buffer, 0, ssd1306_buffer_length);
                snprintf(msg, sizeof(msg), "Digite a senha:");
                ssd1306_draw_string(ssd_buffer, 0, 0, msg);
                snprintf(msg, sizeof(msg), " %s", senha_atual); 
                ssd1306_draw_string(ssd_buffer, 40, 30, msg);
                render_on_display(ssd_buffer, &frame_area);
                
                play_tone(1000, 100);
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            else if (!gpio_get(BUTTON_B) && password_length < MAX_PASSWORD_LENGTH) {
                password[password_length] = 1;
                senha_atual[password_length] = 'B';
                password_length++;
                
                // Atualiza display com efeito de digitação
                memset(ssd_buffer, 0, ssd1306_buffer_length);
                snprintf(msg, sizeof(msg), "Digite a senha:");
                ssd1306_draw_string(ssd_buffer, 0, 0, msg);
                snprintf(msg, sizeof(msg), " %s", senha_atual); 
                ssd1306_draw_string(ssd_buffer, 40, 30, msg);
                render_on_display(ssd_buffer, &frame_area);
                
                play_tone(500, 100);
                vTaskDelay(pdMS_TO_TICKS(300));
            }
            
            if (password_length == MAX_PASSWORD_LENGTH) {
                password_defined = true;
                
   
                memset(ssd_buffer, 0, ssd1306_buffer_length);
                snprintf(msg, sizeof(msg), "Senha definida!");
                ssd1306_draw_string(ssd_buffer, 0, 1, msg);
                render_on_display(ssd_buffer, &frame_area);
                
                // Feedback visual e sonoro
                for (int i = 0; i < 3; i++) {
                    gpio_put(LED_BLUE, 1);
                    play_tone(2000, 100);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    gpio_put(LED_BLUE, 0);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                
                // Troca para autenticação
                vTaskDelay(pdMS_TO_TICKS(1000));
                vTaskResume(xAutenticaHandle); // Ativa autenticação
                vTaskSuspend(NULL);            // Suspende cadastro
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void autentica_task(void *pvParameters) {
    char msg[64];
    char tentativa[5] = "";
    uint8_t tentativa_idx = 0;
    bool autenticado = false;

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    while (1) {
        // Exibe prompt
        memset(ssd_buffer, 0, ssd1306_buffer_length);
        snprintf(msg, sizeof(msg), "Digite a senha:");
        ssd1306_draw_string(ssd_buffer, 0, 0, msg);
        snprintf(msg, sizeof(msg), "%s", tentativa);
        ssd1306_draw_string(ssd_buffer, 40, 30, msg);
        render_on_display(ssd_buffer, &frame_area);

        // Entrada de senha
        while (tentativa_idx < MAX_PASSWORD_LENGTH) {
            if (!gpio_get(BUTTON_A)) {
                tentativa[tentativa_idx] = 'A';
                play_tone(1000, 100);
                tentativa_idx++;
                tentativa[tentativa_idx] = '\0';
                vTaskDelay(pdMS_TO_TICKS(100));
            } else if (!gpio_get(BUTTON_B)) {
                tentativa[tentativa_idx] = 'B';
                play_tone(500, 100);
                tentativa_idx++;
                tentativa[tentativa_idx] = '\0';
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            // Atualiza display a cada tecla
            memset(ssd_buffer, 0, ssd1306_buffer_length);
            snprintf(msg, sizeof(msg), "Digite a senha:");
            ssd1306_draw_string(ssd_buffer, 0, 0, msg);
            snprintf(msg, sizeof(msg), " %s", tentativa);
            ssd1306_draw_string(ssd_buffer, 40, 30, msg);
            render_on_display(ssd_buffer, &frame_area);

            vTaskDelay(pdMS_TO_TICKS(50));
        }

        // Verifica senha
        autenticado = true;
        for (uint8_t i = 0; i < MAX_PASSWORD_LENGTH; i++) {
            if ((tentativa[i] == 'A' && password[i] != 0) ||
                (tentativa[i] == 'B' && password[i] != 1)) {
                autenticado = false;
                break;
            }
        }

        // Feedback
        memset(ssd_buffer, 0, ssd1306_buffer_length);
        if (autenticado) {
            snprintf(msg, sizeof(msg), "Acesso Liberado!");
            ssd1306_draw_string(ssd_buffer, 0, 2, msg);
            render_on_display(ssd_buffer, &frame_area);
            gpio_put(LED_GREEN, 1);
            play_tone(1200, 200);

            // Espera até o botão B ser pressionado para "fechar o cadeado"
            while (1) {
                if (!gpio_get(BUTTON_B)) {
                    // Fecha o cadeado
                    gpio_put(LED_GREEN, 0);
                    memset(ssd_buffer, 0, ssd1306_buffer_length);
                    snprintf(msg, sizeof(msg), "Cadeado Fechado!");
                    ssd1306_draw_string(ssd_buffer, 0, 2, msg);
                    render_on_display(ssd_buffer, &frame_area);
                    play_tone(400, 200);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    break; // Sai do loop e volta para autenticação
                }
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else {
            snprintf(msg, sizeof(msg), "Senha Incorreta!");
            ssd1306_draw_string(ssd_buffer, 0, 2, msg);
            render_on_display(ssd_buffer, &frame_area);
            gpio_put(LED_RED, 1);
            play_tone(300, 400);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_put(LED_RED, 0);
        }

        // Limpa tentativa para próxima autenticação
        tentativa_idx = 0;
        tentativa[0] = '\0';
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


int main() {
    stdio_init_all();

    xTaskCreate(password_setup_task, "PASSWORD_Setup_Task", 512, NULL, 1, &xPasswordSetupHandle);
    xTaskCreate(autentica_task, "AUTENTICA_Task", 512, NULL, 1, &xAutenticaHandle);

    // Começa com autenticação suspensa
    vTaskSuspend(xAutenticaHandle);

    vTaskStartScheduler();
    while(1){};
}