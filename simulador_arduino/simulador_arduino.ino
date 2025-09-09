// Entradas desde el PLC (Arduino las LEE)
int PLC_OUT_ESCOTILLA_E_VALVULA = 13; 
int PLC_OUT_INFLAR = 12;
int PLC_OUT_DESCARGAR = 11;
int PLC_OUT_VALVULA_ALIVIO = 10;
int PLC_OUT_GIRO_HOR = 9;
int PLC_OUT_GIRO_ANTIHOR = 8;
//int PLC_OUT_ESCOTILLA_ABRIR = 14;
//int PLC_OUT_ESCOTILLA_CERRAR = 15;

// Salidas hacia el PLC (Arduino las ESCRIBE)
int ARD_OUT_FC_ESCOTILLA_CERRADA = 2;
int ARD_OUT_FC_ESCOTILLA_ABIERTA = 3;
int ARD_OUT_FC_TAMBOR_ARRIBA = 4;
int ARD_OUT_FC_TAMBOR_ABAJO = 5;
int PWM_PRESION = 6;  // Salida PWM de presión

//Negar salidas PLC
int EscotillaValvula = !digitalRead(PLC_OUT_ESCOTILLA_E_VALVULA);
int Inflar = !digitalRead(PLC_OUT_INFLAR);
int Descargar = !digitalRead(PLC_OUT_DESCARGAR);
int Alivio = !digitalRead(PLC_OUT_VALVULA_ALIVIO);
int GiroHorario = !digitalRead(PLC_OUT_GIRO_HOR);
int GiroAntihorario = !digitalRead(PLC_OUT_GIRO_ANTIHOR);



// Variables internas
float presion = 0.0;
bool escotillaCerrada = true;
int vueltas = 0;
float objetivo_inflar = 2.2;
float objetivo_descargar = -0.15;
float objetivo_aliviar = 1.2;

int cont=0;

void setup() {
  // Entradas
  //pinMode(PLC_OUT_ESCOTILLA_ABRIR, INPUT);
  //pinMode(PLC_OUT_ESCOTILLA_CERRAR, INPUT);
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

void loop() {
  // --- ESCOTILLA --- (Abrir y cerrar compuerta + fines de carrera de salida)

  //Escotilla abierta
  if(digitalRead(EscotillaValvula) == HIGH) {
    delay(200);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, HIGH);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, LOW);
  }

  //Escotilla cerrado
  if(digitalRead(EscotillaValvula) == LOW){
    delay(200);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_ABIERTA, LOW);
    digitalWrite(ARD_OUT_FC_ESCOTILLA_CERRADA, HIGH);
  }



  // --- CASOS CURVAS --- 

  // INFLAR
  if((digitalRead(Alivio) == HIGH)&& (presion<1.1) && digitalRead(Inflar) == LOW){  //venteo de negativo a 1.2
        presion = presion + (objetivo_aliviar - presion) * 0.1;
        delay(200);
      }

  if((digitalRead(Alivio) == HIGH) && (digitalRead(Inflar) == HIGH)){
    if((1 < presion) &&  (presion < 1.3)){
        presion = presion + (1.3 - presion) * 0.1;  //nivelacion a 1.3
        delay(50);
    }
  }

  if((digitalRead(Inflar) == HIGH) && (digitalRead(Alivio) == LOW)){
    if((presion>1.1) && (presion<1.4))
        presion = presion + (objetivo_inflar - presion) * 0.1;  //subir hasta 2.2
        delay(200);
  }



  // DESINFLAR
  if((digitalRead(Alivio) == HIGH) && (digitalRead(Descargar) == LOW)){
    if(presion > 1){
      presion = presion - (presion - objetivo_aliviar) * 0.1;  //bajar a 1.2
      delay(200);
    }
  }

  if((digitalRead(Alivio) == HIGH) && (digitalRead(Descargar) == HIGH)){
    if((1 < presion) &&  (presion < 1.3)){
      presion = presion + (1 - presion) * 0.1;  //nivelacion a 1
      delay(50);
    }
  }
  
  if((digitalRead(Descargar) == HIGH) && (digitalRead(Alivio) == LOW)){
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
  if ((digitalRead(GiroHorario) == HIGH ) and (cont=!359)){
  cont++;
  delay(100);    

  }
  
 if ((digitalRead(GiroAntihorario) == HIGH) and (cont!=0)){
  cont--;
  delay(100);

  }
  if ((digitalRead(GiroHorario) == HIGH ) and (cont==359)) {

    cont=0;
    delay(100);    
  }
  if ((digitalRead(GiroAntihorario) == HIGH) and (cont==0)){
    cont=359;
    delay(100);
  }

  if (cont==0)
    ARD_OUT_FC_TAMBOR_ARRIBA=1;
  else
    ARD_OUT_FC_TAMBOR_ARRIBA=0;

  if(cont==180)
    ARD_OUT_FC_TAMBOR_ABAJO=1;
  else
    ARD_OUT_FC_TAMBOR_ABAJO=0;


  
  // --- DEBUG ---
  Serial.print("Presion: ");
  Serial.print(presion);
  Serial.print(" bar | PWM: ");
  Serial.print(pwmValor);
  Serial.print(" | Vueltas tambor: ");
  Serial.println(vueltas);

  delay(100);
}
