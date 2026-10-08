# Firmware de monitoramento de compressor

Este projeto executa em um ESP32 e coleta temperatura com um DS18B20 e aceleração
nos três eixos com um MPU-6050. O dispositivo envia as leituras em JSON por HTTPS
para uma API de telemetria. Um portal local permite cadastrar a rede Wi-Fi, e as
credenciais ficam salvas na memória NVS para os próximos reinícios.

## Stack e organização

| Tecnologia | Uso |
| --- | --- |
| C e ESP-IDF v6.0 | Firmware e drivers do ESP32 |
| FreeRTOS | Tarefa de leitura e envio de telemetria |
| CMake e Ninja | Compilação do projeto |
| Python e Espressif Installation Manager (EIM) | Instalação e execução das ferramentas |
| One-Wire / RMT e I²C | Comunicação com DS18B20 e MPU-6050 |
| cJSON | Montagem do payload JSON |
| `esp_http_client` e mbedTLS | POST HTTPS com verificação de certificado |
| `esp_http_server`, mDNS e NVS | Portal Wi-Fi, nome `device.local` e persistência |
| API PHP e MariaDB na Hostinger | Recebimento e armazenamento fora do firmware |

A versão usada e validada neste projeto é **ESP-IDF v6.0**. O manifesto aceita
versões mais antigas, mas isso não garante compatibilidade com os drivers atuais.
As bibliotecas externas são obtidas automaticamente pelo IDF Component Manager,
conforme `main/idf_component.yml`: `mdns`, `cjson`, `ds18b20` e `onewire_bus`.

```text
main/                       Inicialização e tarefa de telemetria
components/api_client/      Cliente HTTPS e montagem do JSON
components/wifi/            Conexão Wi-Fi e portal de configuração
components/temperature/     Leitura do DS18B20
components/vibration/       Leitura do MPU-6050
sdkconfig.defaults          Configuração inicial do firmware
```

O backend PHP e o banco de dados são serviços externos; eles não são compilados
por este repositório. O ESP32 se comunica com a API, sem acessar diretamente o banco.

## Hardware necessário

- ESP32 com flash de 4 MB e cabo USB com transferência de dados.
- Sensor de temperatura DS18B20 e resistor de pull-up adequado ao circuito One-Wire.
- Sensor MPU-6050, alimentação adequada aos módulos e GND comum ao ESP32.
- Rede Wi-Fi de 2,4 GHz com acesso à internet.

| Sinal | GPIO do ESP32 |
| --- | --- |
| Dados do DS18B20 | 4 |
| SDA do MPU-6050 | 21 |
| SCL do MPU-6050 | 22 |

O firmware usa o endereço I²C `0x68` e frequência de 400 kHz para o MPU-6050.
Confira a alimentação e os níveis lógicos dos módulos antes de conectar.

## Antes de começar

Copie ou clone este repositório para uma pasta local. Use o caminho dessa pasta
nos comandos abaixo. Cada sistema deve gerar seu próprio diretório `build`;
não reutilize os arquivos de compilação do Windows no Linux ou vice-versa.

O fluxo é: **instalar o ESP-IDF → configurar → build → flash → monitor → configurar Wi-Fi**.
Build compila; flash grava na placa; monitor mostra os logs pela porta serial.
O primeiro build precisa de internet para baixar as dependências.

## Windows

### 1. Instalar as ferramentas

No PowerShell, instale o gerenciador oficial pelo WinGet:

```powershell
winget install --id Espressif.EIM-CLI --exact --source winget
```

Feche e reabra o PowerShell para atualizar o PATH. Instale o ESP-IDF e selecione a versão:

```powershell
eim install -i v6.0 -t esp32 -p C:\Espressif -a true
eim select v6.0
```

O instalador verifica os pré-requisitos, incluindo Git e Python, e instala a
toolchain. Se o WinGet não estiver disponível, siga a
[instalação oficial para Windows](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/get-started/windows-setup.html).

Neste computador, o SDK já está instalado em `C:\Espressif\v6.0\esp-idf` e a placa
CH9102 foi reconhecida como **COM3**. Em outro computador, confirme a porta no
Gerenciador de Dispositivos, em **Portas (COM e LPT)**. Se não aparecer, confira o
cabo USB e o driver correspondente ao conversor USB da placa.

### 2. Compilar, gravar e monitorar pelo terminal

Abra o ambiente do ESP-IDF e entre na pasta do projeto:

```powershell
eim shell v6.0
cd C:\SideProjects\IOT\embedded-compressor-firmware
idf.py --version
idf.py build
idf.py -p COM3 flash monitor
```

Substitua `COM3` se a placa estiver em outra porta. Para abrir somente o monitor:

```powershell
idf.py -p COM3 monitor
```

Em uma nova sessão de terminal, execute novamente `eim shell v6.0`. Nesta
instalação também é possível ativar o ambiente no PowerShell atual com:

```powershell
. C:\Espressif\tools\Microsoft.v6.0.PowerShell_profile.ps1
```

### 3. Usar o VS Code

Instale as extensões **Espressif IDF** (`espressif.esp-idf-extension`) e
**C/C++** (`ms-vscode.cpptools`). Abra a pasta raiz do projeto no VS Code.
Selecione a instalação v6.0 pelo comando **ESP-IDF: Select Current ESP-IDF Version**
na paleta (`Ctrl+Shift+P`), e escolha a porta serial da placa.

Nesta máquina, `.vscode/settings.json` e `.vscode/tasks.json` já estão configurados:

- **Ctrl+Shift+B**: executa `ESP-IDF: Build (v6.0)`.
- **Ctrl+Shift+P → Tasks: Run Task → ESP-IDF: Flash (COM3)**: compila e grava.
- **Tasks: Run Task → ESP-IDF: Monitor (COM3)**: abre os logs a 115200 baud.
- **Tasks: Run Task → ESP-IDF: Build, Flash e Monitor (COM3)**: executa o fluxo completo.

Se o VS Code estava aberto durante a instalação, execute **Developer: Reload Window**.
Caso a porta mude, atualize `idf.port`, `idf.monitorPort` e os argumentos das tarefas.
Os arquivos `.vscode/` são ignorados pelo Git: em outro checkout, configure a extensão
localmente. As tarefas preparadas nesta máquina usam caminhos do Windows.

## Linux

### 1. Instalar o EIM e o ESP-IDF

Em Ubuntu/Debian, a Espressif disponibiliza um repositório APT. Os comandos abaixo
seguem a [documentação oficial do ESP-IDF v6.0](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/get-started/linux-setup.html):

```bash
echo "deb [trusted=yes] https://dl.espressif.com/dl/eim/apt/ stable main" | sudo tee /etc/apt/sources.list.d/espressif.list
sudo apt update
sudo apt install eim-cli
eim install -i v6.0 -t esp32
eim select v6.0
```

Execute a instalação do ESP-IDF como seu usuário comum. Para outras distribuições,
consulte as alternativas de instalação na documentação oficial acima.

### 2. Identificar a porta USB e liberar acesso

Conecte a placa e liste as portas:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Use a porta que aparece ao conectar a placa. Nos exemplos abaixo, ela é
`/dev/ttyACM0`; seu computador pode usar `/dev/ttyUSB0` ou outro número.

Em Ubuntu/Debian, se houver erro de permissão, adicione seu usuário ao grupo serial:

```bash
sudo usermod -aG dialout "$USER"
```

Saia da sessão do Linux e entre novamente para aplicar a associação ao grupo.
Em outras distribuições, confira o grupo proprietário da porta com `ls -l`.

### 3. Compilar, gravar e monitorar

```bash
eim shell v6.0
cd ~/Documentos/embedded-compressor-firmware
idf.py --version
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

Ajuste o caminho do projeto e a porta. Para abrir apenas o monitor:

```bash
idf.py -p /dev/ttyACM0 monitor
```

Para usar o VS Code no Linux, instale as mesmas extensões, abra o projeto e
selecione a instalação v6.0 e a porta `/dev/ttyACM0` ou `/dev/ttyUSB0` pela extensão.
Use os comandos de build, flash e monitor da extensão oficial; as tarefas Windows
desta máquina não funcionam no Linux.

## Configurar o firmware

Dentro de um terminal com ESP-IDF ativo e na pasta do projeto, execute:

```text
idf.py menuconfig
```

Em **Configuracoes da Telemetria**, ajuste a URL e o intervalo de envio.
A configuração inicial em `sdkconfig.defaults` usa:

| Opção | Valor |
| --- | --- |
| Target | `esp32` |
| Flash | 4 MB |
| Partição do aplicativo | 1,5 MB, sem OTA |
| Verificação HTTPS | Pacote completo de certificados |
| URL | `https://darkblue-viper-448014.hostingersite.com/` |

O intervalo padrão definido em `main/Kconfig.projbuild` é 5000 ms. Ele é uma pausa
**após** cada ciclo: leitura dos sensores e envio HTTP também consomem tempo.
Portanto, o firmware não garante um registro exatamente a cada cinco segundos.

`sdkconfig.defaults` fornece os valores iniciais. Se `sdkconfig` já existe, altere
as opções em `menuconfig` ou no editor SDK Configuration da extensão. Depois de
salvar, execute build e flash novamente. Para outra placa, confira o tamanho real
da flash antes de manter a configuração de 4 MB.

## Conectar o dispositivo ao Wi-Fi

1. Ligue ou reinicie o ESP32 após gravar o firmware.
2. Conecte o computador ou celular à rede **device**, com senha **12345678**.
3. Abra **http://192.168.4.1** no navegador.
4. Informe o nome e a senha da sua rede Wi-Fi e pressione **Salvar e Conectar**.
5. Observe no monitor a mensagem **IP Obtido na rede STA**.

As credenciais são salvas na NVS e reutilizadas nos próximos reinícios.
O portal também anuncia **http://device.local** quando o cliente suporta mDNS.
O modo AP+STA mantém o ponto de acesso local disponível junto da conexão à rede.

## Telemetria e resultado esperado

O firmware envia um POST com `Content-Type: application/json`, por exemplo:

```json
{
  "temperature": 26.625,
  "vibration": {
    "x": -0.1728515625,
    "y": -0.593994140625,
    "z": -1.77587890625
  }
}
```

A temperatura é expressa em °C e a aceleração em g. O firmware subtrai 1 g do
eixo Z; a orientação do sensor influencia essa leitura. A calibração adicional e
o cálculo RMS são realizados pelo backend PHP usado neste ambiente.

No monitor, um envio bem-sucedido à API atual apresenta:

```text
esp-x509-crt-bundle: Certificate validated
HTTP_CLIENT: Sucesso HTTP [201]
MAIN_APP: Telemetria enviada com sucesso!
```

O cliente usa `esp_crt_bundle_attach` para verificar o servidor HTTPS. Os registros
recebem seu horário no banco; o payload do firmware não contém um timestamp.

## Problemas comuns

| Sintoma | O que verificar |
| --- | --- |
| `idf.py` não encontrado | Abra o ambiente com `eim shell v6.0` antes dos comandos. |
| Porta ocupada ou acesso negado | Feche outros monitores; no Linux, confira o grupo serial. |
| Falha ao conectar durante o flash | Confira porta/cabo; se necessário, mantenha BOOT pressionado enquanto aparece `Connecting`. |
| Sensor não encontrado | Confira GPIOs, alimentação, GND, pull-up e endereço I²C. O envio é ignorado se uma leitura falhar. |
| `No server verification option set` | Confira `crt_bundle_attach` e a opção `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE`; recompile e grave. |
| HTTP 500 | Verifique os logs da API PHP e o acesso ao banco no servidor. |
| Aplicativo maior que a partição | Confirme a partição de 1,5 MB em `sdkconfig`, definida nos defaults. |

**Problema observado:** a negociação TLS já acionou o watchdog de `IDLE0` durante
cálculos criptográficos, embora o POST tenha terminado com HTTP 201. Esse alerta
continua pendente de ajuste; um envio bem-sucedido não comprova estabilidade em
execução contínua. Confira o backtrace e o tempo de negociação antes de ajustar
processamento, escalonamento ou timeout. Não desative o watchdog como solução.

Use **Ctrl+]** para sair do monitor e liberar a porta. O flash normal mantém a NVS
com esta tabela de partições. `idf.py erase-flash` apaga também as credenciais
Wi-Fi e exige configurar o dispositivo novamente.

## Estado de validação

O build foi concluído no Windows com ESP-IDF v6.0, e a placa foi reconhecida na
COM3. O monitor confirmou validação do certificado e HTTP 201 após a gravação.
As instruções Linux seguem o fluxo oficial do EIM; não foram executadas neste
computador Windows. O alerta de watchdog descrito acima ainda precisa ser tratado.
