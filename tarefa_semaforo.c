#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"


const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BTN_A_PIN 5


int A_state = 0;    //Botão A está pressionado?
uint8_t ssd[ssd1306_buffer_length];
struct render_area frame_area;

//Aqui é a função que usei para exibir mensagens no display OLED
void ExibirMensagemOLED(char *text[], int num_lines) {
    memset(ssd, 0, ssd1306_buffer_length);

    //Aqui mostra as mensagens em relação ao número de linhas para poder caber no display
    for (int i = 0; i < num_lines; i++) {
        ssd1306_draw_string(ssd, 0, 16 * i, text[i]);
    }
    render_on_display(ssd, &frame_area);
}

// Funções do semáforo
void SinalAberto() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    
    //Essa mensagem para o sinal aberto só coube no display em 3 linhas
    char *text[] = {
        " Sinal aberto",
        "  Atravessar ",
        " com cuidado"
    };
    ExibirMensagemOLED(text, 3);
}

void SinalAtencao() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    
    //Essa mensagem para o sinal de atenção só coube no display em 2 linhas
    char *text[] = {
        "Sinal de atencao",
        "   Prepare-se   "
    };
    ExibirMensagemOLED(text, 2);
}

void SinalFechado() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    
    //Essa mensagem para o sinal fechado só coube no display em 2 linhas
    char *text[] = {
        " Sinal fechado",
        "    Aguarde   "
    };
    ExibirMensagemOLED(text, 2);
}

int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i = i + 100) {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1; //Se o botão for pressionado vai retornar 1
        }
        sleep_ms(100); //aguarda 100 ms
    }
    return 0; //Se o botão não for pressionado vai retorna 0
}

int main() {
    stdio_init_all();

    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    //Aqui são as configuração da área de renderização do OLED
    frame_area = (struct render_area){
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    //Configuração dos LEDs
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    //Configuração do botão
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    bool pedestreApertou = false;

    while (true) {
        //Começa com o LED vermelho (Sinal fechado por 8 segundos)
        SinalFechado(); 
        pedestreApertou = WaitWithRead(8000); //Espera o botão ser pressionado por até 8 segundos

        //Verifica se o botão foi pressionado durante o Sinal Fechado
        if (pedestreApertou) { //Alguém apertou o botão
            SinalAtencao(); //Sinal de atenção por 5 segundos
            sleep_ms(5000);

            SinalAberto(); //Sinal aberto (verde) por 10 segundos
            sleep_ms(10000);
        } else {
            //Se o botão não for pressionado, segue a sequência normal
            SinalAtencao(); //Sinal de atenção por 2 segundos
            sleep_ms(2000);

            SinalAberto(); //Sinal aberto (verde) por 15 segundos
            sleep_ms(15000);
        }
    }

    return 0;
}
