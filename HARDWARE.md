# 🔌 Documentação de Hardware e Diagrama de Ligações

Este documento descreve o mapeamento de pinos (pinout) e os esquemas elétricos para a conexão dos sensores ao microcontrolador **ESP32** no projeto de monitoramento de compressores.

---

## 🧰 Componentes Utilizados

- **Microcontrolador:** ESP32 DevKit v1
- **Sensor de Temperatura:** DS18B20 (à prova d'água)
- **Sensor de Vibração/Acelerômetro:** MPU-6050 (I2C)
- **Componentes Passivos:** 1x Resistor de $4.7\text{ k}\Omega$ (Pull-up para barramento 1-Wire)

---

## 📋 Tabela de Mapeamento de Pinos (Pinout)

| Componente / Módulo | Pino no Componente | Pino no ESP32 | Sinal / Descrição | Observação |
| :--- | :--- | :--- | :--- | :--- |
| **DS18B20** *(Temperatura)* | VCC (Vermelho) | `3V3` | Alimentação ($3.3\text{V}$) | Tensão contínua |
| | GND (Preto) | `GND` | Terra | Referência comum |
| | DATA (Amarelo/Azul) | `GPIO 4` | Dados (1-Wire) | Requer resistor pull-up de $4.7\text{ k}\Omega$ para $3.3\text{V}$ |
| **MPU-6050** *(Vibração)* | VCC | `3V3` | Alimentação ($3.3\text{V}$) | Tensão contínua |
| | GND | `GND` | Terra | Referência comum |
| | SCL | `GPIO 22` | I2C Clock | Linha de relógio do I2C |
| | SDA | `GPIO 21` | I2C Data | Linha de dados do I2C |
| | AD0 | `GND` | Endereço I2C | Define o endereço I2C como `0x68` |

---

## 📐 Esquemático Geral de Ligações (ASCII)

```text
               +----------------------------------------+
               |              ESP32 DEV KIT             |
               |                                        |
       (3.3V) -| 3V3                                GPIO4|- (Data) ------------+
        (GND) -| GND                               GND  |- (GND)              |
               |                                 GPIO21 |- (SDA) ---+         |
               |                                 GPIO22 |- (SCL) -+ |         |
               +----------------------------------------+          | |         |
                                                                   | |         |
                                                                   | |         |
+------------------------------------+                             | |         |
|        SENSOR DE VIBRAÇÃO          |                             | |         |
|              MPU-6050              |                             | |         |
|                                    |                             | |         |
|  VCC  --------------> 3.3V (ESP32) |                             | |         |
|  GND  --------------> GND  (ESP32) |                             | |         |
|  SCL  -----------------------------------------------------------+ |         |
|  SDA  -------------------------------------------------------------+         |
|  AD0  --------------> GND  (Endereço I2C 0x68)                               |
+------------------------------------+                                         |
                                                                               |
                                                                               |
+------------------------------------+                                         |
|       SENSOR DE TEMPERATURA        |                                         |
|               DS18B20              |                                         |
|                                    |                                         |
|  VCC (Vermelho) ----> 3.3V (ESP32) |<----+                                   |
|  GND (Preto)   ----> GND  (ESP32)  |     | (Resistor Pull-up                 |
|  DATA(Amarelo/Azul)-> GPIO 4 -------+----[ 4.7kΩ ]                           |
+------------------------------------+                                         |
                                                                               |
                                                                               +
```

---

## 🔍 Detalhe do Circuito Barramento 1-Wire (DS18B20)

O sensor DS18B20 necessita obrigatoriamente de um resistor de **pull-up de $4.7\text{ k}\Omega$** conectado entre a linha de **Dados (DATA)** e a alimentação de **$3.3\text{V}$** para manter o sinal estável no protocolo 1-Wire.

```text
            +3.3V
              |
             [ ] Resistor 4.7kΩ
              |
              +-----------------------+
              |                       |
       +--------------+        +--------------+
       |   DS18B20    |        |    ESP32     |
       |              |        |              |
       |  DATA (Pino) |--------|  GPIO 4      |
       |  VCC  (Pino) |--------|  3.3V        |
       |  GND  (Pino) |--------|  GND         |
       +--------------+        +--------------+
```