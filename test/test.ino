// ------------------ PINES ------------------

// Entradas desde el PLC (Arduino las LEE)
int PLC_OUT_ESCOTILLA_E_VALVULA = 13; 
int PLC_OUT_INFLAR = 12;
int PLC_OUT_DESCARGAR = 11;
int PLC_OUT_VALVULA_ALIVIO = 10;
int PLC_OUT_GIRO_HOR = 9;
int PLC_OUT_GIRO_ANTIHOR = 8;

// Salidas hacia el PLC (Arduino las ESCRIBE)
int ARD_OUT_FC_ESCOTILLA_CERRADA = 2;
int ARD_OUT_FC_ESCOTILLA_ABIERTA = 3;
int ARD_OUT_FC_TAMBOR_ARRIBA = 4;
int ARD_OUT_FC_TAMBOR_ABAJO = 5;
int PWM_PRESION = 6;  // Salida PWM de presión

// ------------------ VARIABLES ------------------
float presion = 0.0;
float objetivo_inflar = 2.2;
float objetivo_descargar = -0.15;
float objetivo_aliviar = 1.2;

int cont = 0;     // ángulo simulado del tambor (0-359)
int vueltas = 0;  // contador de vueltas completas

// ------------------ SETUP ------------------
void setup() {
  // Entradas con pull-up
  pinMode(PLC_OUT_INFLAR, INPUT_PULLUP);
  pinMode(PLC_OUT_DESCARGAR, INPUT_PULLUP);
  pinMode(PLC_OUT_VALVULA_ALIVIO, INPUT_PULLUP);
  pinMode(PLC_OUT_GIRO_HOR, INPUT_PULLUP);
  pinMode(PLC_OUT_GIRO_ANTIHOR, INPUT_PULLUP);
  pinMode(PLC_OUT_ESCOTILLA_E_VALVULA, INPUT_PULLUP);

  // Salidas
  pinMode(ARD_OUT_FC_ESCOTILLA_CERRADA, OUTPUT);
  pinMode(ARD_OUT_FC_ESCOTILLA_ABIERTA, OUTPUT);
  pinMode(ARD_OUT_FC_TAMBOR_ARRIBA, OUTPUT);
  pinMode(ARD_OUT_FC_TAMBOR_ABAJO, OUTPUT);
  pinMode(PWM_PRESION, OUTPUT);

  Serial.begin(9600);
}

// ------------------ LOOP ------------------
void loop() {
  // Leer entradas negadas
  bool EscotillaValvula = !digitalRead(PLC_OUT_ESCOTILLA_E_VALVULA);
  bool Inflar           = !digitalRead(PLC_OUT_INFLAR);
  bool Descargar        = !digitalRead(PLC_OUT_DESCARGAR);
  bool Alivio           = !digitalRead(PLC_OUT_VALVULA_ALIVIO);
  bool GiroHorario      = !digitalRead(PLC_OUT_GIRO_HOR);
  bool GiroAntihorario  = !digitalRead(PLC_OUT_GIRO_ANTIHOR);

  // ------------------ ESCOTILLA ------------------
  if (EscotillaValvula) { // abierta
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, HIGH);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, LOW);
  } else {                // cerrada
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, LOW);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, HIGH);
  }

  // ------------------ PRESIÓN ------------------
  if (Alivio && (presion < 1.1) && !Inflar) {  
    presion = presion + (objetivo_aliviar - presion) * 0.1;
    delay(200);
  }
  else if (Alivio && Inflar) {
    if ((1 < presion) && (presion < 1.3)) {
      presion = presion + (1.3 - presion) * 0.1;
      delay(50);
    }
  }
  else if (Inflar && !Alivio) {
    if ((presion > 1.1) && (presion < 1.4)) {
      presion = presion + (objetivo_inflar - presion) * 0.1;
      delay(200);
    }
  }
  else if (Alivio && !Descargar) {
    if (presion > 1) {
      presion = presion - (presion - objetivo_aliviar) * 0.1;
      delay(200);
    }
  }
  else if (Alivio && Descargar) {
    if ((1 < presion) && (presion < 1.3)) {
      presion = presion + (1 - presion) * 0.1;
      delay(50);
    }
  }
  else if (Descargar && !Alivio) {
    if ((presion > 0.9) && (presion < 1.3)) {
      presion = presion + (objetivo_descargar - presion) * 0.1;
      delay(200);
    }
  }

  // PWM presión (0 a 2 bar → 0 a 255 PWM)
  int pwmValor = map(presion * 100, 0, 200, 0, 255);
  pwmValor = constrain(pwmValor, 0, 255);
  analogWrite(PWM_PRESION, pwmValor);

  // ------------------ GIRO DEL TAMBOR ------------------
  if (GiroHorario && cont != 359) {
    cont++;
    delay(100);
  }
  else if (GiroAntihorario && cont != 0) {
    cont--;
    delay(100);
  }
  else if (GiroHorario && cont == 359) {
    cont = 0;
    vueltas++;  // sumo vuelta completa
    delay(100);
  }
  else if (GiroAntihorario && cont == 0) {
    cont = 359;
    vueltas--;  // resto vuelta completa
    delay(100);
  }

  // Fines de carrera (sin operador ternario)
  if (cont == 0) {
    digitalWrite(ARD_OUT_FC_TAMBOR_ARRIBA, HIGH);
  } else {
    digitalWrite(ARD_OUT_FC_TAMBOR_ARRIBA, LOW);
  }

  if (cont == 180) {
    digitalWrite(ARD_OUT_FC_TAMBOR_ABAJO, HIGH);
  } else {
    digitalWrite(ARD_OUT_FC_TAMBOR_ABAJO, LOW);
  }

  // ------------------ DEBUG ------------------
  Serial.print("Presion: ");
  Serial.print(presion, 2);
  Serial.print(" bar | PWM: ");
  Serial.print(pwmValor);
  Serial.print(" | cont: ");
  Serial.print(cont);
  Serial.print(" | vueltas: ");
  Serial.println(vueltas);
}
