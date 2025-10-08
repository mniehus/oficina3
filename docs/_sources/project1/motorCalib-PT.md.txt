# Calibração & software

Este documento é uma **introdução + guia** ao projeto do Sistema Automático de Calibração de Motor DC.

O projeto automatiza uma **calibração velocidade–tensão** para uma pequena unidade de propulsão DC. Um Arduino gera uma **rampa PWM (=pulse width modulation) em sinal TTL** configurável que comanda um driver em ponte-H, o qual alimenta um **motor DC de drone de 3,7 V** com **hélice de duas pás**. O motor está montado num dispositivo simples com um **sensor ótico de fenda** a funcionar como encoder incremental: cada passagem da pá gera um impulso usado para inferir a **velocidade de rotação RPM (=rotações por minuto)**. Os parâmetros de teste—máximo da rampa, duração, passo/cadência—são definidos em software para que o mesmo hardware de bancada execute várias varridas repetíveis.

Durante a execução, a rampa PWM e as medições de RPM são **sincronizadas**: as atualizações de PWM definem janelas temporais limpas nas quais os impulsos são contados e convertidos em RPM (ou, em alternativa, pode usar-se um período de amostragem fixo). Após cada medição, o sistema envia **valores separados por tabulações (TSV)** pela ligação série USB—tempo, valor de onda comandado, PWM aplicado e RPM—permitindo ao utilizador copiar e colar diretamente para o Excel (ou outra folha de cálculo) para traçar gráficos e analisar.

A **parte de hardware** é simples. Um **PC** liga por **USB** a uma **placa Arduino** (p. ex., Arduino Uno). Um **pino digital PWM** do Arduino alimenta a **entrada enable/PWM** do driver de motor em **ponte-H**.

A placa da ponte-H é alimentada por uma **fonte DC de bancada** (ajustada à tensão/corrente do motor) e aciona um **mini motor DC de drone de 3,7 V** com **hélice de duas pás**. Verifique que os pontos de funcionamento tensão–corrente durante a varrida de calibração **não** ficam limitados pela fonte DC (limite de corrente/abaixamento de tensão), sobretudo a alta velocidade e carga elevada.

Um **sensor ótico de fenda** (LED IR + fototransístor) é montado numa pequena estrutura mecânica para que cada pá interrompa o feixe uma vez por meia rotação, produzindo dois impulsos limpos por volta. Use os **LEDs indicadores** da placa do sensor para alinhar as pás e obter um sinal de RPM limpo, e **prefira a saída digital** à saída analógica. A saída do sensor liga a um **pino de interrupção do Arduino** (p. ex., D2) para contagem precisa de impulsos.

As massas lógicas devem estar comuns: GND do Arduino ↔ GND lógico do driver ↔ GND do sensor. A alimentação do motor é separada da alimentação lógica, que permanece referida no driver. Alimente o **motor** pela fonte de bancada e o **Arduino** por USB para reduzir acoplamentos; os díodos internos da ponte-H tratam do flyback.

Para observação em tempo real, use dois pontos de teste para osciloscópio: o **sinal PWM** (Arduino → enable do driver) e o **sinal de impulsos de RPM** (sensor → interrupção do Arduino). Visualize a razão cíclica e a temporização dos impulsos no osciloscópio enquanto o Arduino os utiliza (comando e medição).

Para **input do utilizador**, estão previstos um **botão**, um **potenciómetro (knob)** e a **interface série USB** para utilização futura. Para **output do utilizador**, existem **três LEDs RGB** para feedback visual e um **buzzer piezo** para sinalização acústica/feedback sonoro. Estão incluídos por completude; o seu uso neste projeto é opcional, mas podem ser úteis para implementar interfaces simples aqui ou noutros projetos.

A **parte de software** organiza-se em seis pequenas tarefas independentes, através de módulos implementados como **máquinas de estados finitos (FSMs)**.

Este sistema é um **banco de ensaio experimental para formação académica**. Embora a calibração automática do motor DC pudesse ser implementada de forma mais simples, o objetivo é introduzir e consolidar um **framework modular e reutilizável de FSMs**, escalável e fácil de adaptar a projetos futuros.

Destina-se a ensinar como funcionam as FSMs, como as implementar e testar, porque as usamos, e a mostrar que o código é reutilizável e **escalável**, mantendo-se claro e arrumado à medida que o sistema cresce (sem “spaghetti”!), e como os módulos se integram num sistema e interagem através de um pequeno conjunto de variáveis globais partilhadas.

---

## Porque usar Máquinas de Estados?

**O problema:** sistemas embebidos gerem várias tarefas—geração de onda, saída PWM, medição de RPM, registo, I/O do utilizador—muitas vezes com cadências e restrições de hardware distintas. Ciclos ingênuos com `delay()` tornam-se rapidamente ingovernáveis.

As **máquinas de estados** resolvem isto:

* **Modularidade:** cada funcionalidade vive no seu próprio ficheiro/função com estados, temporizadores e configuração; sem delays “espaguete”.
* **Testabilidade:** cada FSM pode ser exercitada isoladamente (forçar estados, injetar entradas) e validada.
* **Determinismo:** todas as FSMs são **não bloqueantes** e correm em cada `loop()`; a temporização é explícita (verificações de período), não implícita (`delay()`).
* **Extensibilidade:** adicione um estado/transição sem reescrever módulos não relacionados.
* **Baixo acoplamento:** os módulos “falam” apenas através de algumas **variáveis globais partilhadas** (bytes, floats, contadores), e não por cadeias de chamadas profundas.
* **Usabilidade:** o comportamento de cada módulo é fácil de explicar com um simples **diagrama UML de estados**.

---

## Layout do software

A arquitetura segue um modelo **modular por FSM**: cada tarefa essencial corre de forma **não bloqueante** na sua própria máquina de estados, codificada num **ficheiro separado e reutilizável**. Cada FSM mantém a sua configuração local (como `const` dentro da função), usa estado interno para gerir temporização (sem `delay()`), e comunica apenas através de algumas variáveis globais partilhadas. O **ficheiro controlador do Arduino** na mesma pasta configura as interrupções, declara as globais e monta o sistema chamando todas as FSMs no `loop()`.

**Ficheiro controlador**

```cpp
MotorCalib.ino                  // controller, globals, interrupt config, empty setup(), loop() calling all FSMs
```

**Módulos FSM**

```cpp
WaveformFSM.ino                 // WaveformFSM_update()
PWMOutputFSM.ino                // PWMOutputFSM_update()
RPM_FSM.ino                     // RPM_FSM_update()
DataLogger_FSM.ino              // DataLogger_FSM_update(...)
UserInput_FSM.ino               // UserInput_FSM_update()
UserOutput_FSM.ino              // UserOutput_FSM_update()
```

* **Waveform FSM**: Gera uma sequência de amostras (seno / quadrada / dente-de-serra / rampa única) com período de amostragem e modulação configuráveis; escreve `sig_waveform_value`.
* **PWM Output FSM**: Aplica periodicamente `sig_waveform_value` ao pino PWM com limite, mínimo opcional e **slew limiter**; escreve `sig_pwm_applied` e incrementa `sig_pwm_update_counter` a cada atualização.
* **RPM FSM**: Conta impulsos do encoder ótico (via ISR) e calcula RPM em **período fixo** ou **sincronizado** com janelas de PWM usando `sig_pwm_update_counter`; escreve `sig_rpm`.
* **Data Logger FSM**: Único logger com dois modos—**free-run** (cadência fixa) ou **sincronizado ao PWM**—imprime colunas selecionadas (tempo, onda, PWM, RPM) em TSV.
* **User Input FSM**: Debounce do botão, amostragem do potenciómetro e parser de comandos série **key=value**; escreve `sig_input_*`.
* **User Output FSM**: Comanda LEDs RGB e buzzer piezo via `tone()` (contínuo ou temporizado) ou alternância digital; consome `sig_led_*` e `sig_buzzer_*`.

**Como tudo se articula?**

1. **Waveform → PWM:** A Waveform FSM produz o **duty desejado** (`sig_waveform_value`).
2. **PWM aplica (com segurança):** A PWM FSM limita, eleva valores pequenos se necessário e **suaviza variações** antes de escrever no pino. Incrementa `sig_pwm_update_counter` em cada atualização.
3. **Amostragem de RPM:** A RPM FSM mede impulsos em período fixo ou **sincronizada** com janelas de PWM (borda-a-borda) e escreve `sig_rpm`.
4. **Registo:** A Data Logger imprime linhas TSV periodicamente ou por janela de PWM, com colunas selecionáveis **tempo / onda / PWM / RPM**.
5. **I/O do utilizador:** A User Input FSM atualiza `sig_input_*` (botão, potenciómetro, série). `sig_led_*` e os sinais do buzzer estão globalmente declarados e são acionados pela User Output FSM (qualquer FSM pode lê-los).
6. `MotorCalib.ino` é o “compositor”: declara as **variáveis globais partilhadas**, configura e anexa a **ISR do encoder**, deixa o `setup()` vazio (as FSMs auto-inicializam na primeira chamada) e, no `loop()`, chama as FSMs por ordem, p. ex.:
   `Waveform → PWM Output → RPM → Datalogger → User Input → User Output`.
7. Cada FSM é **não bloqueante** e avança por temporizadores e `switch(state)`; lê/escreve apenas as globais documentadas, mantendo os ficheiros independentes e reutilizáveis entre projetos.

## Dicas para Laboratório & Testes

* **Testes por módulo:** Comente os restantes e execute uma FSM de cada vez; use o Logger em modo free-run para observar o comportamento.
* **Rastreabilidade:** O cabeçalho de cada FSM no código documenta **tarefa, entradas (sinais), saídas (sinais), uso, testes e como estender**—explore e estude cada módulo.
* **Implemente o use-case:** Defina a Waveform como **SINGLE\_RAMP** (sem repetição) para capturar uma varrida completa de calibração PWM→RPM.

---

## Anexo 1: Base de código de referência

A base de código completa está disponível para download para os participantes na plataforma Moodle da unidade curricular.

## Anexo 2: Controlador de Calibração

O ficheiro Arduino **controlador de calibração** `motorCalib.ino` é o ponto de orquestração do projeto. **Detém os sinais partilhados** (variáveis globais simples), **define a ISR do encoder** e **executa todas as FSMs uma vez por ciclo** numa ordem fixa e não bloqueante. Cada FSM mantém a sua configuração local e temporização interna; o controlador apenas as interliga através de algumas globais.

**Tarefas**

* **Globais partilhadas:** declarar o pequeno conjunto de variáveis entre módulos (p. ex., `sig_waveform_value`, `sig_pwm_applied`, `sig_pwm_update_counter`, `sig_encoder_pulses`, `sig_rpm`, intenções de I/O do utilizador).
* **Propriedade da interrupção:** definir a **ISR** (incrementa `volatile unsigned long sig_encoder_pulses`) e o **pino do encoder**. O **attach/detach** é efetuado pela RPM FSM no estado `INIT` (fonte única de verdade para modo/borda); o controlador permanece o local canónico do símbolo e contador da ISR.
* **Modelo de execução:** escalonamento cooperativo e **não bloqueante**—todas as FSMs são chamadas uma vez por `loop()`. Cada FSM progride por temporizadores/guardas (sem `delay()`), intercalando tarefas de forma previsível.
* **Ordem de chamada (define o fluxo de dados):**
  `Waveform → PWM Output → RPM → Datalogger → User Input → User Output`.
  Garante que:

  * a PWM lê a amostra mais recente da onda,
  * a RPM pode **acoplar** a janela de medição às atualizações de PWM via `sig_pwm_update_counter`,
  * a Datalogger imprime valores **deste** ciclo,
  * I/O do utilizador é aplicada em cada ciclo sem bloquear.

**Temporização & determinismo**

* Toda a temporização é **explícita** (verificações de período dentro de cada FSM). A frequência do `loop()` pode variar; o comportamento não.
* A única fonte assíncrona é a **ISR do encoder**; aceda ao contador com `volatile` e secções críticas curtas (já tratadas na RPM FSM).

**Código:**   [motorCalib.ino](code/motorCalib/motorCalib.ino)

## Anexo 3: Sinais globais

Estas são as únicas variáveis entre módulos. Cada FSM **lê** e/ou **escreve** nelas.

| Sinal                         | Tipo                     | Produtor(es) | Consumidor(es)     | Significado                                |
| ----------------------------- | ------------------------ | ------------ | ------------------ | ------------------------------------------ |
| `sig_waveform_value`          | `byte`                   | Waveform FSM | PWM Output, Logger | Amostra de duty desejado (0..255)          |
| `sig_pwm_applied`             | `byte`                   | PWM Output   | Logger             | PWM efetivamente escrito (0..255)          |
| `sig_pwm_update_counter`      | `unsigned long`          | PWM Output   | RPM FSM, Logger    | Incrementa a cada atualização de PWM       |
| `sig_encoder_pulses`          | `volatile unsigned long` | **ISR**      | RPM FSM            | Contador de impulsos (fenda ótica)         |
| `sig_rpm`                     | `float`                  | RPM FSM      | Logger             | RPM mais recente                           |
| `sig_input_button`            | `byte`                   | User Input   | Qualquer           | Botão debounced (0/1)                      |
| `sig_input_pot`               | `int`                    | User Input   | Qualquer           | ADC do potenciómetro (0..1023)             |
| `sig_input_key`               | `char`                   | User Input   | Qualquer           | Última tecla série                         |
| `sig_input_value`             | `long`                   | User Input   | Qualquer           | Último valor série                         |
| `sig_input_has_cmd`           | `bool`                   | User Input   | Qualquer           | “Novo comando disponível” (latched)        |
| `sig_led_r/g/b`               | `byte`                   | Qualquer     | User Output        | Comandos on/off para LEDs RGB              |
| `sig_buzzer_tone_enable`      | `bool`                   | Qualquer     | User Output        | Usar `tone()` se true                      |
| `sig_buzzer_tone_freq_hz`     | `unsigned int`           | Qualquer     | User Output        | Frequência do tom                          |
| `sig_buzzer_tone_ms`          | `unsigned int`           | Qualquer     | User Output        | 0=contínuo; >0 duração do tom              |
| `sig_buzzer_toggle_enable`    | `bool`                   | Qualquer     | User Output        | Alternância digital se true e sem `tone()` |
| `sig_buzzer_toggle_period_ms` | `unsigned int`           | Qualquer     | User Output        | Meia-período para alternância              |

---

## Anexo 4: Módulos FSM — diagramas

Abaixo, um diagrama UML (unified model language) por FSM (estados e transições). 
Note a correspondência estrita entre diagrama e código.

A lista começa com um exemplo genérico e educativo de um FSM de referência, seguido pela descrição detalhada dos FSMs utilizados.

### Reference FSM (generic example)

**Task:** este exemplo **genérico** de máquina de estados com três estados — `INIT`, `STATE1`, `STATE2` — ilustra um ciclo típico: inicializar, operar num primeiro modo, alternar para um segundo modo mediante um evento/condição, e regressar.

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> STATE1 : event/condition_1 (p.ex., "inicialização concluída")
  STATE1 --> STATE2 : event/condition_2 (p.ex., "temporizador1 expirou" OU "botão premido")
  STATE1 --> STATE1 : else (permanece até ocorrer event/condition_2)
  STATE2 --> STATE1 : event/condition_3 (p.ex., "temporizador2 expirou" OU "valor abaixo do limiar")
  STATE2 --> STATE2 : else (permanece até ocorrer event/condition_3)
```

**Events/transition conditions:**
* event/condition\_1: sistema pronto (ex.: configuração feita).
* event/condition\_2: condição para mudar de `STATE1` para `STATE2` (ex.: temporizador1, entrada externa, limiar atingido).
* event/condition\_3: condição para regressar de `STATE2` para `STATE1` (ex.: temporizador2, entrada externa, limiar desfeito).

**Activities:**

* INIT: preparar variáveis/recursos; marcar sistema como pronto.
* STATE1: executar atividade A (ex.: atualizar saída/monitorizar entrada com período T1).
* STATE2: executar atividade B (ex.: outra estratégia/saída com período T2).

**Code** (Arduino/C++; não bloqueante, educativo)

```cpp
// ---------------- Optional shared signal for demos ----------------
byte sig_ref_state = 0;  // 0=INIT, 1=STATE1, 2=STATE2

// ---------------- Three-state reference FSM ----------------------
void ReferenceFSM_update() {
  // -------- Local configuration constants (tunable) --------------
  const unsigned int  PERIOD_STATE1_MS = 250;   // drives event/condition_2 (example)
  const unsigned int  PERIOD_STATE2_MS = 500;   // drives event/condition_3 (example)

  // -------- Local state constants (UPPERCASE) --------------------
  const byte INIT   = 0;
  const byte STATE1 = 1;
  const byte STATE2 = 2;

  // -------- Local static variables -------------------------------
  static byte          state = INIT;
  static unsigned long t1_ms = 0;   // timer for STATE1
  static unsigned long t2_ms = 0;   // timer for STATE2
  static bool          ready = false; // models event/condition_1

  // -------- Time base --------------------------------------------
  unsigned long now = millis();

  // -------- FSM ---------------------------------------------------
  switch (state) {

    case INIT: {
      // Activities: initialize resources/vars
      ready = true;                  // event/condition_1 satisfied
      sig_ref_state = 0;

      // Transition: INIT -> STATE1 on event/condition_1
      if (ready) {
        t1_ms = now;                 // reset STATE1 timer
        state = STATE1;
      }
      break;
    }

    case STATE1: {
      // Activities: do-A (periodic action at PERIOD_STATE1_MS)
      sig_ref_state = 1;

      // Example periodic activity (replace with your own):
      if ((unsigned long)(now - t1_ms) >= PERIOD_STATE1_MS) {
        // --- event/condition_2 occurs here (e.g., timer1 expired) ---
        t2_ms = now;                 // prepare STATE2 timer
        state = STATE2;              // Transition: STATE1 -> STATE2
      }
      // else: remain in STATE1
      break;
    }

    case STATE2: {
      // Activities: do-B (periodic action at PERIOD_STATE2_MS)
      sig_ref_state = 2;

      // Example periodic activity (replace with your own):
      if ((unsigned long)(now - t2_ms) >= PERIOD_STATE2_MS) {
        // --- event/condition_3 occurs here (e.g., timer2 expired) ---
        t1_ms = now;                 // prepare STATE1 timer
        state = STATE1;              // Transition: STATE2 -> STATE1
      }
      // else: remain in STATE2
      break;
    }

    default: {
      // Safety net
      state = INIT;
      break;
    }
  } // switch
}
```

**Uso:** chame `ReferenceFSM_update()` em cada iteração de `loop()`.
**Adaptação:** substitua os temporizadores por entradas reais (botões/sensores) para materializar `event/condition_2` e `event/condition_3`.

### Waveform Generator FSM

**Tarefa:** “Avançar a um ritmo estável e calcular o ponto seguinte da onda escolhida.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : após a configuração
  WAIT_TICK --> COMPUTE : a cada período de amostragem
  COMPUTE --> WAIT_TICK : ondas periódicas (seno/quadrada/dente-de-serra) ou rampa ainda a decorrer
  COMPUTE --> HOLD : rampa única terminada E sem repetição
  HOLD --> WAIT_TICK : se for reiniciada
```

**Eventos:** temporizador decorrido.
**Atividades:** calcular a onda a partir de `phase = (now - t0) % MOD_PERIOD`.

**Código:**
[void WaveformFSM\_update()](code/motorCalib/WaveformFSM.ino)

---

### PWM Output FSM

**Tarefa:** “A um ritmo fixo, atualizar o PWM do motor; limitar a rapidez de variação.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : após a configuração
  WAIT_TICK --> UPDATE : a cada período de atualização do PWM
  UPDATE --> WAIT_TICK : após escrever o PWM (com limites & slew)
```

**Eventos:** temporizador decorrido.
**Atividades:** aplicar `MAX_DUTY`, mínimo opcional, **slew limit** lógico; escrever PWM & sinais.

**Código:**
[PWMOutputFSM\_update()](code/motorCalib/PWMOutputFSM.ino)

---

### RPM FSM

**Tarefa:** “Abrir uma janela de medição, contar impulsos, calcular RPM; janelas cronometradas ou alinhadas com o PWM.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> IDLE : após a configuração
  IDLE --> START_WINDOW : MODO=acoplado E houve atualização de PWM
  IDLE --> START_WINDOW : MODO=free-run E temporizador decorreu
  START_WINDOW --> END_WINDOW : janela iniciada
  END_WINDOW --> COMPUTE : MODO=acoplado E próxima atualização de PWM
  END_WINDOW --> COMPUTE : MODO=free-run E tempo da janela decorreu
  COMPUTE --> IDLE : após calcular o RPM
```

**Eventos:** borda de PWM via `sig_pwm_update_counter` (acoplado) ou temporizador (free-run).
**Atividades:** instantâneo/repôr contador; calcular RPM (proteger `dt` muito curto).

**Código:**
[void RPM\_FSM\_update()](code/motorCalib/RpmFSM.ino)

---

### Data Logger FSM

**Tarefa:** “Registar por temporização fixa ou por janela de PWM—apenas as colunas escolhidas.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> IDLE : após a configuração
  IDLE --> LOG : MODO=acoplado E houve atualização de PWM
  IDLE --> LOG : MODO=free-run E período de registo decorreu
  LOG --> IDLE : após imprimir uma linha
```

**Eventos:** borda de PWM ou temporizador fixo.
**Atividades:** imprimir colunas selecionadas; reimprimir cabeçalho se layout mudar.

**Código:**
[void DataLogger\_FSM\_update()](code/motorCalib/DataLoggerFSM.ino)

---

### User Input FSM

**Tarefa:** “Ler continuamente botão e knob; quando chega uma linha série completa, fazer parse key=value.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : após a configuração
  WAIT_TICK --> SCAN_INPUTS : há dados na série
  SCAN_INPUTS --> PARSE_LINE : chegou fim de linha
  SCAN_INPUTS --> WAIT_TICK : ainda não há linha completa
  PARSE_LINE --> WAIT_TICK : após registar key/value
```

**Eventos:** debounce, amostragem, EOL na série.
**Atividades:** normalizar botão ativo-baixo para 0/1; ler ADC; interpretar `char=value`.

**Código:**
[void UserInput\_FSM\_update()](code/motorCalib/UserInputFSM.ino)

---

### User Output FSM

**Tarefa:** “A um ritmo estável, atualizar LEDs; produzir som via `tone()` ou alternância on/off.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : após a configuração
  WAIT_TICK --> APPLY : a cada período de atualização de saída
  APPLY --> WAIT_TICK : depois de refrescar LEDs/buzzer
```

**Eventos:** temporizador decorrido.
**Atividades:** `tone()` contínuo/one-shot, ou alternância digital; LEDs ativo-alto/baixo.

**Código:**
[void UserOutput\_FSM\_update()](code/motorCalib/UserOutputFSM.ino)
