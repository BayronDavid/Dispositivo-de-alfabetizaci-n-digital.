#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Configuración de las pantallas LCD con sus respectivas direcciones I2C
LiquidCrystal_I2C lcd_1(0x27, 16, 2); 
LiquidCrystal_I2C lcd_2(0x26, 16, 2);

// Configuración del DFPlayer Mini
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

String palabras[10] = {"casa", "coche", "gato", "pera", "sol", "luna", "mar", "cielo", "flores", "amor"};
const char alfabeto[27] = "abcdefghijklmnopqrstuvwxyzñ";
int limit = 4;
char randomLetters[4];

void randomizeWord(String &randomWord, int &correctLetter, char &xd, int &correctIndex, int &wordTrack) {
  int randomNumber = random(10);
  randomWord = palabras[randomNumber];
  correctLetter = random(randomWord.length());
  xd = randomWord[correctLetter];
  correctIndex = random(limit);

  // Establecer el número de pista para la palabra
  wordTrack = 28 + randomNumber; // 28 es el primer número de pista para las palabras
}

bool contains(String a, char b) {
  return a.indexOf(b) != -1;
}

void renderResponse(String &randomWord, int &correctLetter, char &xd, int &correctIndex) {
  for (int i = 0; i < limit; i++) {
    if (i == correctIndex) {
      randomLetters[i] = xd;
    } else {
      char tempLetter;
      do {
        int randomIndex = random(27);
        tempLetter = alfabeto[randomIndex];
      } while (contains(randomWord, tempLetter));
      randomLetters[i] = tempLetter;
    }

    lcd_1.clear();
    lcd_2.clear();
  
    lcd_1.print(randomWord);
    for (int i = 0; i < limit; i++) {
      lcd_2.setCursor(i * 4, 0);
      lcd_2.print(randomLetters[i]);
    }
  }
}

void setup() {
  Serial.begin(9600);
  mySoftwareSerial.begin(9600);

  // Inicialización de las pantallas LCD
  lcd_1.init();
  lcd_1.backlight();
  lcd_2.init();
  lcd_2.backlight();

  // Inicialización del DFPlayer Mini
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Error al iniciar el DFPlayer Mini"));
    while(true);
  }
  myDFPlayer.volume(30); // Ajustar el volumen

  String randomWord = "";
  int correctLetter = 0;
  char xd = ' ';
  int correctIndex = 0;
  int wordTrack = 0;

  // Establecer una condición o bandera para la primera ejecución
  bool isFirstRun = true;

  if (!isFirstRun) {
    randomizeWord(randomWord, correctLetter, xd, correctIndex, wordTrack);
    myDFPlayer.play(wordTrack); // Reproducir sonido de la palabra
    renderResponse(randomWord, correctLetter, xd, correctIndex);
  }
}

void loop() {
  static String randomWord;
  static int correctLetter;
  static char xd;
  static int correctIndex;
  static int wordTrack;
  static bool started = false;

  if (!started) {
    randomizeWord(randomWord, correctLetter, xd, correctIndex, wordTrack);
    myDFPlayer.play(wordTrack); // Reproducir sonido de la palabra
    renderResponse(randomWord, correctLetter, xd, correctIndex);
    started = true;
  }

  int valores[4] = {analogRead(A0), analogRead(A1), analogRead(A2), analogRead(A3)};

  for (int i = 0; i < 4; i++) {
    if (valores[i] > 0) { // Verificar cada botón
      lcd_2.setCursor(i * 4, 1);
      myDFPlayer.play(randomLetters[i] - 'a' + 1); // Calcular el número de pista para la letra

      if (correctIndex == i) {
        lcd_2.print("V");
        delay(1000); // Esperar para que el sonido termine
        myDFPlayer.play(38); // Reproducir sonido "muy bien"
        delay(1000); // Esperar para que el sonido termine
        randomizeWord(randomWord, correctLetter, xd, correctIndex, wordTrack);
        myDFPlayer.play(wordTrack); // Reproducir sonido de la nueva palabra
        renderResponse(randomWord, correctLetter, xd, correctIndex);
      } else {
        lcd_2.print("X");
        delay(1000); // Esperar para que el sonido termine
        myDFPlayer.play(39); // Reproducir sonido "incorrecto"
        delay(1000); // Esperar para que el sonido termine
      }
      delay(1000); // Esperar para evitar múltiples pulsaciones
      break; // Salir del bucle después de un botón presionado
    }
  }

  delay(1000);

  // Parpadeo del cursor en lcd_1
  lcd_1.setCursor(correctLetter, 0);
  lcd_1.print("_");
  delay(500);
  lcd_1.setCursor(correctLetter, 0);
  lcd_1.print(xd);
  delay(500);
}

