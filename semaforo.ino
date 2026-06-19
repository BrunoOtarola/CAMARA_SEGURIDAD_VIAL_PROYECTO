// Definición de los pines para cada LED
const int pinRojo = 2;
const int pinAmarillo = 3;
const int pinVerde = 4;

const int pinBicicleta_Rojo = 8;
const int pinBicicleta_Amarillo = 9;
const int pinBicicleta_Verde = 10;

void setup() {
  // Configurar los pines digitales como salidas
  pinMode(pinRojo, OUTPUT);
  pinMode(pinAmarillo, OUTPUT);
  pinMode(pinVerde, OUTPUT);

  pinMode(pinBicicleta_Rojo, OUTPUT);
  pinMode(pinBicicleta_Amarillo, OUTPUT);
  pinMode(pinBicicleta_Verde, OUTPUT);
}

void loop() {
  semaforo();
}

void semaforo() {
// 1. ROJO ENCIENDE (Alto total, amarillo se apaga)
  digitalWrite(pinVerde, LOW);
  digitalWrite(pinAmarillo, LOW);
  digitalWrite(pinRojo, HIGH);

  digitalWrite(pinBicicleta_Verde, HIGH);
  digitalWrite(pinBicicleta_Amarillo, LOW);
  digitalWrite(pinBicicleta_Rojo, LOW);
  delay(10000); // Mantiene el rojo por 5 segundos

  // 2. AMARILLO ENCIENDE (Precaución, verde se apaga)
  digitalWrite(pinVerde, LOW);
  digitalWrite(pinAmarillo, HIGH);
  digitalWrite(pinRojo, LOW);
  digitalWrite(pinBicicleta_Verde, LOW);
  digitalWrite(pinBicicleta_Amarillo, HIGH);
  digitalWrite(pinBicicleta_Rojo, LOW);
  delay(2500); // Mantiene el amarillo por 2 segundos

  // 3. VERDE ENCIENDE (Los demás apagados)
  digitalWrite(pinVerde, HIGH);
  digitalWrite(pinAmarillo, LOW);
  digitalWrite(pinRojo, LOW);
  digitalWrite(pinBicicleta_Verde, LOW);
  digitalWrite(pinBicicleta_Amarillo, LOW);
  digitalWrite(pinBicicleta_Rojo, HIGH);
  delay(15000); // Mantiene el verde por 5 segundos
}

void bicicleta() {

}