# Tarefa: Roteiro de FreeRTOS #2  - EmbarcaTech 2025

# Projeto Cofre

Autores: **[Fábio Gabriel](https://github.com/fabinsz)** e **[Karen Beatrice](https://github.com/karenbeat)**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

---

##  Descrição do Projeto

Este projeto implementa um sistema de senha para "cofre eletrônico" utilizando FreeRTOS no Raspberry Pi Pico, display OLED SSD1306, buzzer e botões físicos. O objetivo é demonstrar o uso de multitarefas, periféricos e lógica de autenticação em sistemas embarcados.

---

## Hardware Requerido

* Raspberry Pi Pico W
* Display OLED SSD1306 (I2C)
* 2 Botões (GPIO 5 e GPIO 6)
* LED vermelho (GPIO 13)
* LED azul (GPIO 12)
* LED verde (GPIO 11)
* Buzzer (GPIO 21)
* Resistores de pull-up (internos do Pico ou externos)
* Fios de conexão

---
##  Funcionalidades Desenvolvidas

- **Definição de Senha:**  
  Ao iniciar o sistema, o usuário deve definir uma senha de 4 dígitos utilizando os botões A e B. Cada botão representa um caractere (A ou B), e a senha é exibida em tempo real no display OLED.

- **Feedback Sonoro e Visual:**  
  Cada pressionamento de botão gera um som diferente no buzzer. LEDs indicam status de sucesso ou erro durante o uso.

- **Autenticação:**  
  Após definir a senha, o sistema entra no modo de autenticação. O usuário deve digitar a senha correta para liberar o acesso. O display mostra a senha sendo digitada.

- **Acesso Liberado e Fechamento:**  
  Se a senha estiver correta, o sistema exibe "Acesso Liberado!" e acende o LED verde. O acesso permanece liberado até que o usuário pressione o botão B, que "fecha o cadeado", apaga o LED e retorna para a tela de autenticação.

- **Senha Incorreta:**  
  Se a senha estiver errada, o sistema exibe "Senha Incorreta!" e acende o LED vermelho por um tempo antes de permitir nova tentativa.

- **Gerenciamento de Tasks:**  
  O projeto utiliza FreeRTOS para alternar entre as tarefas de cadastro e autenticação, garantindo fluxo seguro e organizado.

---

##  Execução

1. **Compilação:**  
   Compile o projeto normalmente utilizando o CMake e o ambiente do Raspberry Pi Pico.

2. **Execução:**  
   - Ao ligar, defina a senha pressionando os botões A e B.
   - Após os 4 dígitos, o sistema entra no modo de autenticação.
   - Digite a senha para liberar o acesso.
   - Para "fechar o cadeado", pressione o botão B.
   - O sistema retorna para a tela de autenticação para novas tentativas.

3. **Reinicialização:**  
   Para redefinir a senha, reinicie o dispositivo.


## [Vídeo de demonstração](https://youtube.com/shorts/3IecbByduek?feature=share)


## 📜 Licença
GNU GPL-3.0.
