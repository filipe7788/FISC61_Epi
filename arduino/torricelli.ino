// Pinos do sensor HC-SR04
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// Parâmetros do tanque (ajuste conforme sua montagem)
const float DIST_SENSOR_FUNDO = 14.0;  // Distância do sensor até o fundo do tanque (cm)

// Constantes físicas
const float GRAVIDADE = 981.0;  // Aceleração da gravidade em cm/s²

// Intervalo entre medições (ms)
const unsigned long INTERVALO = 500;  // 0.5 segundos entre cada medida

// Variáveis de tempo
unsigned long tempoInicio;
unsigned long ultimaMedicao = 0;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // Aguarda comando para iniciar (pressione qualquer tecla no Serial Monitor)
  Serial.println("Pressione qualquer tecla para iniciar a medicao...");
  while (Serial.available() == 0) {}
  Serial.read();  // Limpa o buffer

  // Cabeçalho CSV para fácil importação no Python/Excel
  Serial.println("tempo_s,distancia_cm,altura_cm,velocidade_teorica_cm_s");

  tempoInicio = millis();
}

void loop() {
  unsigned long agora = millis();

  // Verifica se é hora de fazer uma nova medição
  if (agora - ultimaMedicao >= INTERVALO) {
    ultimaMedicao = agora;

    // Calcula tempo desde o início (em segundos)
    float tempo = (agora - tempoInicio) / 1000.0;

    // Faz a medição de distância
    float distancia = medirDistancia();

    // Verifica se a medição foi válida
    if (distancia < 0) {
      Serial.println("# Erro na medicao - pulando leitura");
      return;
    }

    // Calcula altura da água
    float altura = DIST_SENSOR_FUNDO - distancia;

    // Se altura for negativa (tanque vazio), para as medições
    if (altura < 0.5) {
      Serial.println("# Tanque vazio - medicao encerrada");
      while(true) {}  // Para o programa
    }

    // Calcula velocidade teórica de Torricelli: v = sqrt(2 * g * h)
    float velocidade = sqrt(2.0 * GRAVIDADE * altura);

    // Envia dados no formato CSV
    Serial.print(tempo, 2);
    Serial.print(",");
    Serial.print(distancia, 2);
    Serial.print(",");
    Serial.print(altura, 2);
    Serial.print(",");
    Serial.println(velocidade, 2);
  }
}

float medirDistancia() {
  // Garante que o TRIG está LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Envia pulso de 10 microsegundos
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mede o tempo do pulso ECHO
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout de 30ms

  // Se timeout, retorna -1
  if (duracao == 0) {
    return -1;
  }

  // Calcula distância em cm
  // Velocidade do som ≈ 343 m/s = 0.0343 cm/μs
  // Distância = (tempo × velocidade) / 2
  float distancia = duracao * 0.0343 / 2.0;

  return distancia;
}
