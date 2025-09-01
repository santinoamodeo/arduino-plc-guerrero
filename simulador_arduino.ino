// Entradas desde el PLC (Arduino las LEE)
int PLC_OUT_ESCOTILLA_ABRIR = 2;
int PLC_OUT_ESCOTILLA_CERRAR = 3;
int PLC_OUT_INFLAR = 4;
int PLC_OUT_DESCARGAR = 5;
int PLC_OUT_VALVULA_ALIVIO = 7;
int PLC_OUT_GIRO_HOR = 8;
int PLC_OUT_GIRO_ANTIHOR = 12;

// Salidas hacia el PLC (Arduino las ESCRIBE)
int ARD_OUT_FC_ESCOTILLA_CERRADA = 9;
int ARD_OUT_FC_ESCOTILLA_ABIERTA = 10;
int ARD_OUT_FC_TAMBOR = 11;
int PWM_PRESION = 6;  // Salida PWM de presión

// Variables internas
float presion = 0.0;
bool escotillaCerrada = true;
int vueltas = 0;
float objetivo_inflar = 2.2;
float objetivo_descargar = 0.15;
float objetivo_aliviar = 1;

void setup() {
  // Entradas
  pinMode(PLC_OUT_ESCOTILLA_ABRIR, INPUT);
  pinMode(PLC_OUT_ESCOTILLA_CERRAR, INPUT);
  pinMode(PLC_OUT_INFLAR, INPUT);
  pinMode(PLC_OUT_DESCARGAR, INPUT);
  pinMode(PLC_OUT_VALVULA_ALIVIO, INPUT);
  pinMode(PLC_OUT_GIRO_HOR, INPUT);
  pinMode(PLC_OUT_GIRO_ANTIHOR, INPUT);

  // Salidas
  pinMode(ARD_OUT_FC_ESCOTILLA_CERRADA, OUTPUT);
  pinMode(ARD_OUT_FC_ESCOTILLA_ABIERTA, OUTPUT);
  pinMode(ARD_OUT_FC_TAMBOR, OUTPUT);
  pinMode(PWM_PRESION, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // --- Escotilla ---
  if (digitalRead(PLC_OUT_ESCOTILLA_ABRIR) == HIGH) {
    escotillaCerrada = false;
  }
  if (digitalRead(PLC_OUT_ESCOTILLA_CERRAR) == HIGH) {
    escotillaCerrada = true;
  }

  if (escotillaCerrada == true) {
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, HIGH);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, LOW);
  } 
  if{
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, LOW);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, HIGH);
  }

  // --- Presión neumática ---
  if (digitalRead(PLC_OUT_INFLAR) == HIGH) {
    if (presion < objetivo_inflar) {
      presion = presion + (objetivo_inflar - presion) * 0.1;  //2.2
      delay(200);
    }
  }

  if (digitalRead(PLC_OUT_DESCARGAR) == HIGH) {
    if (presion > -0.15) {
      presion = presion - (presion - objetivo_descargar) * 0.1;  //-0.15 
      delay(200);
    }
  }

  if (digitalRead(PLC_OUT_VALVULA_ALIVIO) == HIGH) {  //presion atmosfera  1
    if (presion > objetivo_aliviar) {
      presion = presion - (presion - objetivo_aliviar) * 0.1;
      delay(200);
    }
  }

  // PWM presión (0 a 2 bar → 0 a 255 PWM)
  int pwmValor = presion * 127;
  if (pwmValor < 0) pwmValor = 0;
  if (pwmValor > 255) pwmValor = 255;
  analogWrite(PWM_PRESION, pwmValor);

  // --- Giros del tambor (1 vuelta cada 3 segundos) ---
  if (digitalRead(PLC_OUT_GIRO_HOR) == HIGH || digitalRead(PLC_OUT_GIRO_ANTIHOR) == HIGH) {

    delay(3000); // cada 3 segundos = 1 vuelta
    digitalWrite(ARD_OUT_FC_TAMBOR, HIGH);
    delay(100);
    digitalWrite(ARD_OUT_FC_TAMBOR, LOW);
  }

  

  // --- DEBUG ---
  Serial.print("Presion: ");
  Serial.print(presion);
  Serial.print(" bar | PWM: ");
  Serial.print(pwmValor);
  Serial.print(" | Vueltas tambor: ");
  Serial.println(vueltas);

  delay(100);
}
