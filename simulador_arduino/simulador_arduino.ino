// Entradas desde el PLC (Arduino las LEE)
int PLC_OUT_ESCOTILLA_E_VALVULA = 2; 
int PLC_OUT_INFLAR = 4;
int PLC_OUT_DESCARGAR = 5;
int PLC_OUT_VALVULA_ALIVIO = 7;
int PLC_OUT_GIRO_HOR = 8;
int PLC_OUT_GIRO_ANTIHOR = 12;
int PLC_OUT_ESCOTILLA_ABRIR = 14;
int PLC_OUT_ESCOTILLA_CERRAR = 15;

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
float objetivo_descargar = -0.15;
float objetivo_aliviar = 1.2;

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
  // --- ESCOTILLA --- (Abrir y cerrar compuerta + fines de carrera de salida)

  //Escotilla abierta
  if(digitalRead(PLC_OUT_ESCOTILLA_E_VALVULA) == HIGH) {
    delay(200);
    ARD_OUT_FC_ESCOTILLA_ABIERTA = HIGH;
    ARD_OUT_FC_ESCOTILLA_CERRADA = LOW;
  }

  //Escotilla cerrado
  if(digitalRead(PLC_OUT_ESCOTILLA_E_VALVULA) == LOW){
    delay(200);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, HIGH);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, LOW);
  }



  // --- CASOS CURVAS --- 

  // INFLAR
  if((digitalRead(PLC_OUT_VALVULA_ALIVIO) == HIGH)&& (presion<1.1) && digitalRead(PLC_OUT_INFLAR) == LOW){  //venteo de negativo a 1.2
        presion = presion + (objetivo_aliviar - presion) * 0.1;
        delay(200);
      }

  if((digitalRead(PLC_OUT_VALVULA_ALIVIO) == HIGH) && (digitalRead(PLC_OUT_INFLAR) == HIGH)){
    if((1 < presion) &&  (presion < 1.3)){
        presion = presion + (1.3 - presion) * 0.1;  //nivelacion a 1.3
        delay(50);
    }
  }

  if((digitalRead(PLC_OUT_INFLAR) == HIGH) && (digitalRead(PLC_OUT_VALVULA_ALIVIO) == LOW)){
    if((presion>1.1) && (presion<1.4))
        presion = presion + (objetivo_inflar - presion) * 0.1;  //subir hasta 2.2
        delay(200);
  }



  // DESINFLAR
  if((digitalRead(PLC_OUT_VALVULA_ALIVIO) == HIGH) && (digitalRead(PLC_OUT_DESCARGAR) == LOW)){
    if(presion > 1){
      presion = presion - (presion - objetivo_aliviar) * 0.1;  //bajar a 1.2
      delay(200);
    }
  }

  if((digitalRead(PLC_OUT_VALVULA_ALIVIO) == HIGH) && (digitalRead(PLC_OUT_DESCARGAR) == HIGH)){
    if((1 < presion) &&  (presion < 1.3)){
      presion = presion + (1 - presion) * 0.1;  //nivelacion a 1
      delay(50);
    }
  }
  
  if((digitalRead(PLC_OUT_DESCARGAR) == HIGH) && (digitalRead(PLC_OUT_VALVULA_ALIVIO) == LOW)){
    if((presion>0.9) && (presion<1.3))
      presion = presion + (objetivo_descargar - presion) * 0.1;  //bajar hasta -0.15
  }


/*
  // --- Presión neumática ---
  if (PLC_OUT_INFLAR == HIGH) {
    if (presion < objetivo_inflar) {
      presion = presion + (objetivo_inflar - presion) * 0.1;  //2.2
      delay(200);
    }
  }

  if (PLC_OUT_DESCARGAR == HIGH) {
    if (presion > objetivo_descargar) {
      presion = presion - (presion - objetivo_descargar) * 0.1;  //-0.15 
      delay(200);
    }
  }

  if (PLC_OUT_VALVULA_ALIVIO == HIGH) {  //  1.2
    if (presion > objetivo_aliviar) { // de 2 a 1.2
      presion = presion - (presion - objetivo_aliviar) * 0.1;
      delay(200);
    }
    if (presion < objetivo_aliviar) { // de -0.15 a 1.2
      presion = presion + (objetivo_aliviar - presion) * 0.1;
      delay(200);
    }
  }
*/

// --- PWM presión (0 a 2 bar → 0 a 255 PWM) ---
  int pwmValor = map(presion * 100, 0, 200, 0, 255); // 0 bar = 0, 2 bar = 255

  // Seguridad por si se pasa del rango
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
