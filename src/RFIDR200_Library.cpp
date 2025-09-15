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
    Serial.begin(115220);
    Serial.println("Iniciando leitor RFID com configuração de Alta Penetração...");

    MySerial.begin(115200, SERIAL_8N1, 16, 17);
    rfidReader.begin();

    // --- CONFIGURAÇÃO AVANÇADA E COMPLETA ---

    // 1. POTÊNCIA MÁXIMA para alcance e penetração
    if (rfidReader.setTransmitPower(3000)) {
        Serial.println("Potência definida com SUCESSO.");
    } else {
        Serial.println("FALHA ao definir a potência!");
    }

    // 2. MODO DE ALTA SENSIBILIDADE usando os parâmetros do manual
    Serial.println("Definindo modo de leitura para máxima sensibilidade...");
    uint8_t mixerGain = 0x03; // 9dB
    uint8_t ifGain = 0x06;    // 36dB
    uint16_t threshold = 0x01B0;
    if (rfidReader.setDemodulatorParameters(mixerGain, ifGain, threshold)) {
        Serial.println("Parâmetros do demodulador definidos com SUCESSO.");
    } else {
        Serial.println("FALHA ao definir os parâmetros do demodulador.");
    }

    // 3. ALGORITMO OTIMIZADO para poucas etiquetas (Q=1, Session=S0)
    if(rfidReader.setQueryParameters(1, 1, 1, 0)) {
        Serial.println("Parâmetros de Query definidos com SUCESSO.");
    } else {
        Serial.println("FALHA ao definir os parâmetros de Query.");
    }
    
    // 4. Iniciar a leitura
    Serial.println("\n--- Configuração concluída, iniciando leitura ---");
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

