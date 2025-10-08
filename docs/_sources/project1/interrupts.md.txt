# Interrupções

O Arduino permite associar funções a **interrupções de hardware**, que são executadas automaticamente quando ocorre um evento (ex.: transição num pino digital, overflow de um timer).

### Exemplo: Contagem de Pulsos (Encoder Simples)

```cpp
#define ENCODER_PIN 2
volatile long counter = 0;

void ISR_encoder() {
  counter++;
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), ISR_encoder, RISING);
  Serial.begin(115200);
}

void loop() {
  Serial.println(counter);
  delay(500);
}
```

 Para **encoders em quadratura**, usam-se duas interrupções (canais A e B), permitindo detetar também o sentido de rotação.

