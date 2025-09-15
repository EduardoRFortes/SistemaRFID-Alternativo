#include <Arduino.h>
#include <HardwareSerial.h>
#include "RFIDR200.h"

unsigned long tempoUltimaLeitura = 0;
const long intervaloDeLeitura = 20;

// Usaremos a UART2 do ESP32. Criamos um objeto HardwareSerial para representá-la.
HardwareSerial MySerial(2);

// Instanciamos o leitor RFID, passando o objeto HardwareSerial que acabamos de criar.
// Isso vai chamar o construtor correto na sua biblioteca.
RFIDR200 rfidReader(MySerial, 115200);

uint8_t targetepc[12] = {0xE2, 0x00, 0x00, 0x17, 0x57, 0x0D, 0x01, 0x23, 0x06, 0x30, 0xD6, 0x8E};

unsigned long lastMultiplePollMessage = millis();

// ... includes e objetos ...

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando leitor RFID...");

    MySerial.begin(115200, SERIAL_8N1, 16, 17);
    rfidReader.begin();

    // --- INÍCIO DA CONFIGURAÇÃO OTIMIZADA ---

    // 1. POTÊNCIA MÁXIMA para garantir a penetração do sinal
    Serial.println("Definindo potência de transmissão para o máximo...");
    if (rfidReader.setTransmitPower(2700)) { // Use 2700 para 27dBm ou o máximo do seu módulo
        Serial.println("Potência definida com SUCESSO.");
    } else {
        Serial.println("FALHA ao definir a potência!");
    }

    // 2. ALGORITMO OTIMIZADO para poucas etiquetas e leitura rápida
    // Q Fixo e Baixo (Q=1 => 2 slots), Sessão S0 (releituras constantes)
    uint8_t startQ = 1;
    uint8_t minQ = 1;
    uint8_t maxQ = 1;
    uint8_t session = 0; // S0 -> A etiqueta responde sempre

    Serial.println("Otimizando parâmetros para leitura rápida de poucas tags...");
    if(rfidReader.setQueryParameters(startQ, minQ, maxQ, session)) {
        Serial.println("Parâmetros de Query definidos com SUCESSO.");
    } else {
        Serial.println("FALHA ao definir os parâmetros de Query.");
    }
    
    // --- FIM DA CONFIGURAÇÃO OTIMIZADA ---
    
    // 3. Iniciar a leitura
    Serial.println("Iniciando polling múltiplo...");
    rfidReader.initiateMultiplePolling(10000);
}

void loop() {
    unsigned long tempoAtual = millis();

    uint8_t responseBuffer[256];

    if(tempoAtual - tempoUltimaLeitura >= intervaloDeLeitura) {

        if (rfidReader.getResponse(responseBuffer, 256)) {
            if(rfidReader.hasValidTag(responseBuffer)) {
            uint8_t rssi;
            uint8_t epc[12];
            rfidReader.parseTagResponse(responseBuffer, rssi, epc);

            Serial.print("Tag Encontrada! -> ");
            Serial.print("RSSI: ");
            Serial.print(rssi, DEC);
            Serial.print(" | ");

            Serial.print("EPC: ");
            for (int i = 0; i < 12; ++i) {
                if (epc[i] < 0x10) Serial.print("0");
                Serial.print(epc[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
            }
        }
        tempoUltimaLeitura = tempoAtual;
    }
}

