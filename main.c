#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// Configurações do teclado matricial 4x3
#define LINHAS 4
#define COLUNAS 3

uint linhas[LINHAS] = {18, 16, 19, 17};  // GPIOs das linhas
uint colunas[COLUNAS] = {4, 20, 9};     // GPIOs das colunas

char mapa_teclado[LINHAS][COLUNAS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// GPIOs do LED RGB
#define LED_VERMELHO 13
// #define LED_VERDE    11
// #define LED_AZUL     12

// GPIO do buzzer
#define BUZZER 21

// Função para inicializar GPIOs
void init_gpio() {
    // Linhas como saída
    for (int i = 0; i < LINHAS; i++) {
        gpio_init(linhas[i]);
        gpio_set_dir(linhas[i], GPIO_OUT);
        gpio_put(linhas[i], 1); // Inicializa como HIGH
    }

    // Colunas como entrada com pull-up
    for (int i = 0; i < COLUNAS; i++) {
        gpio_init(colunas[i]);
        gpio_set_dir(colunas[i], GPIO_IN);
        gpio_pull_up(colunas[i]);
    }

    // LED vermelho
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);

    // Buzzer
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);
}

// Função de leitura de tecla (varredura do teclado)
char ler_tecla() {
    for (int l = 0; l < LINHAS; l++) {
        // Desativa todas as linhas
        for (int i = 0; i < LINHAS; i++) {
            gpio_put(linhas[i], 1);
        }

        // Ativa a linha atual
        gpio_put(linhas[l], 0);
        sleep_us(3); // Estabiliza

        // Varre colunas
        for (int c = 0; c < COLUNAS; c++) {
            if (gpio_get(colunas[c]) == 0) {
                while (gpio_get(colunas[c]) == 0) {
                    tight_loop_contents(); // Espera liberar tecla
                }
                return mapa_teclado[l][c];
            }
        }
    }
    return '\0'; // Nenhuma tecla pressionada
}

// Tarefa principal
void task_teclado(void *params) {
    char senha[32] = {0};
    int pos = 0;

    printf("Senha: ");
    fflush(stdout);

    while (true) {
        char tecla = ler_tecla();
        if (tecla != '\0') {
            if (pos < sizeof(senha) - 1) {
                senha[pos++] = tecla;
                printf("%c", tecla);
                fflush(stdout);
            }

            // Aciona LED e buzzer
            gpio_put(LED_VERMELHO, 1);
            gpio_put(BUZZER, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_put(LED_VERMELHO, 0);
            gpio_put(BUZZER, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Evita loop muito rápido
    }
}

int main() {
    stdio_init_all();
    sleep_ms(10000); // Aguarda 10 segundos antes de iniciar (útil para abrir minicom)

    init_gpio();

    xTaskCreate(task_teclado, "Leitura Teclado", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true) {
        tight_loop_contents(); // Nunca deve chegar aqui
    }
}
