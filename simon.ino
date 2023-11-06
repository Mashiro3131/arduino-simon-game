/******************************************************
 
 * Simon Says Game - Copyright (c) 2023 Nico Anthony Mengisen
 * Released under the MIT License.
 * See LICENSE.txt for details.
 
******************************************************/


#include <Arduino.h>


#define blueButtonLedPin 2
#define yellowButtonLedPin 3
#define redButtonLedPin 4
#define greenButtonLedPin 5

const int blueButtonPin = A2;
const int yellowButtonPin = A3;
const int redButtonPin = A4;
const int greenButtonPin = A5;

#define buzzerPin 6


const int blueButtonNote = 165;   // 165 Hz = Mi3 (bleu)
const int yellowButtonNote = 131; // 131 Hz = Sol3 (jaune)
const int redButtonNote = 110;    // 110 Hz = La3 (rouge)
const int greenButtonNote = 82;   // 82 Hz = Mi2 (vert)

const int errorButtonNote = 33;   // 33 Hz = Do1 (erreur)


const int buttonLedPins[] = {blueButtonLedPin, yellowButtonLedPin, redButtonLedPin, greenButtonLedPin};
const int buttonPins[] = {blueButtonPin, yellowButtonPin, redButtonPin, greenButtonPin};
const int buttonTones[] = {blueButtonNote, yellowButtonNote, redButtonNote, greenButtonNote};


#define gameLength 3
int sequence[gameLength] = {0};
int sequenceIndex = 0;


// pour la fonction gameGenerateSequence()
const int delayBetweenNotes = 50;
const int delayAfterSequence = 50;


// Pour la fonction getButtonState()
int buttonState[4] = {HIGH, HIGH, HIGH, HIGH};
int lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};

unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
const unsigned long debounceDelay = 5;


/**
    Pour la fonction gameOver, ca va nous permettre d'éviter l'animation du début
    si on à déjà démarré la partie comme ca si on souhaite rejour on aura pas le 
    jingle du démarrage
  */
bool startupAnimationPlayed  = false;
bool gameStarted = false;


/* On va séparer numLeds et numButtons pour pouvoir les utiliser dans les fonctions
   et si dans le futur le joueur souhaite ajouter des leds ou des boutons
   ca sera possible plus facilement en changant le nombre de leds et de boutons
*/
const int numLeds = 4;
const int numButtons = 4;


// bool playStartUpTone = true;


/**
   Arduino Functions
*/

void setup() {
    Serial.begin(9600);

    // Set up LED pins as outputs
    for (int i = 0; i < numLeds; i++) {
        pinMode(buttonLedPins[i], OUTPUT);
    }

    // Set up button pins as inputs with pull-up resistors
    for (int i = 0; i < numButtons; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    // Set up speaker pin as output
    pinMode(buzzerPin, OUTPUT);

    // Seed the random number generator
    randomSeed(analogRead(A0));
}


void loop() {

    // On va vérifier si le jeu a démarré ou non
    if (!gameStarted) {
        gameStarted = true;
        startUpGame();
    }

    // Ca va générer un nombre aléatoire entre 0 et le nombre de leds qu'on a
    sequence[sequenceIndex] = random(0, numLeds);
    sequenceIndex++;

    // Vérifie si l'index de la séquence est supérieur à la longueur de la séquence
    if (sequenceIndex >= gameLength) {
        sequenceIndex = gameLength - 1;
    }
    
    // Génère la séquence de jeu
    gameGenerateSequence();

    // Vérifie si le joueur a répliqué la bonne séquence
    if (!playerReplicateSequence()) {
        gameStarted = false; 
        gameOver();
    }

    delay(300);

    // Joue un petit son pour indiquer que le joueur a réussi à répliquer la séquence
    if (sequenceIndex > 0) {
        playLevelUpTone();
        delay(300);
    }

    delay(300);

    // Vérifie si le joueur a atteint le nombre de séquence maximum, si oui, il a gagné et on va jouer une petite musique de victoire
    if (sequenceIndex == gameLength - 1) {
        gameStarted = false;
        gameVictory();

    }
}



/**
   Game Functions
*/

// OK
void startUpGame() {

    // Allume la led et joue un jingle en attendant que le joueur press le bouton de notre led 
    digitalWrite(buttonLedPins[0], HIGH);
    // startUpGameToneV2();
    while (digitalRead(A2) == HIGH ){ // ajouter que la LED bleu s'allume pour indiquer le bouton a appuyer pour démarrer
        // Ici la boucle while va attendre que le joueur appuie sur le bouton. En attendant on va faire clignotter la led blue pour la led du bouton de démarrage ;)
    }

    // Arrete la musique une fois que le joueur démarre la partie
    // playStartUpTone = false;

    // Ca va éteindre la led un fois que la partie est lancée
    digitalWrite(buttonLedPins[0], LOW);
    
    // On va ajouter un délai de 100 pour éviter que la partie démarre instentanéement après ravoir relancé le jeu après avoir perdu
    delay(250);

    // digitalRead(buttonLedPins[1], LOW) // éteindre la led une fois que le joueur a appuyer le bouton de démarrage
    if (!startupAnimationPlayed) {
        startUpGameAnimation();
        startupAnimationPlayed = true;
    }
    delay(500);
    
}


// 2)
void lightAndSound(int id) {

    tone(buzzerPin, buttonTones[id]);
    digitalWrite(buttonLedPins[id], HIGH);

    delay(300);

    noTone(buzzerPin);
    digitalWrite(buttonLedPins[id], LOW);

}

// Ca sera la fonciton qui nous permettra de générer la séquence de jeu
void gameGenerateSequence() {
    
    // On va faire une boucle pour allumer les leds et jouer les sons
    for (int i = 0; i < sequenceIndex; i++) {
        int currentLed = sequence[i];

        // On va vérifier si la led est dans le nombre de leds qu'on a
        if (currentLed >= 0 && currentLed < numLeds) {
            lightAndSound(currentLed);
            delay(delayBetweenNotes);
        }
    }
    delay(delayAfterSequence);
}


int getButtonState() {
    // Vérifie l'état des boutons et renvoie l'index du bouton qui est pressé
    while (true) {

        // Vérifie l'état des boutons
        for (int i = 0; i < numButtons; i++) {
            int buttonPin = buttonPins[i];
            buttonState[i] = digitalRead(buttonPin);

            // Si le bouton est pressé, on va vérifier si le bouton a été pressé récemment
            if (buttonState[i] != lastButtonState[i]) {
                lastDebounceTime[i] = millis();
            }
            
            // Si le bouton a été pressé récemment, on va vérifier si le bouton est toujours pressé
            if ((millis() - lastDebounceTime[i]) > debounceDelay) {
                if (buttonState[i] == LOW) {
                    return i;
                }
            }
            // On met à jour l'état du bouton
            lastButtonState[i] = buttonState[i];
        }
        // On attend un peu avant de vérifier à nouveau l'état des boutons
        delay(1);
    }
}


// On va faire un booléen pour vérifier si le joueur a répliqué la bonne séquence
bool playerReplicateSequence() {

    // Vérifie si le joueur peut répliquer la séquence
    for (int i = 0; i < sequenceIndex; i++) {
        int buttonPressed = getButtonState();
        lightAndSound(buttonPressed);

        // Vérifie si le bouton pressé est le bon
        if (buttonPressed != sequence[i]) {
            return false;
        }
    }
    // Si le joueur a répliqué la bonne séquence, on retourne true
    return true;
}


// 6) On va faire une fonction pour si le joueur a perdu, pour afficher son score et réinitialiser le jeu
void gameOver() {
    
    // Patiente un moment avant d'afficher le score
    delay(500);

    //Print dans le moniteur série pour afficher le score
    Serial.print("Game over! Your score: ");
    Serial.println(sequenceIndex - 1);

    // Réinitialise le jeu à 0
    sequenceIndex = 0;
    gameOverAnimation();

}


// Fonction pour si le joueur a gagné, on affiche son score, réinitialiser le jeu et joue une petite musique + animation de victoire
void gameVictory() {

    // Patiente un moment avant d'afficher le score
    delay(500);

    //Print dans le moniteur série pour afficher le score
    Serial.print("Félicitation, vous avez réussi à libérer la Princesse Zelda!");
    Serial.print("Ton score vicotieux : ");
    Serial.println(sequenceIndex - 1);

    // Réinitialise le jeu à 0
    sequenceIndex = 0;
    gameVictoryAnimation();
    gameVictoryTone();

}



/**
    Game Animations
*/

// On va faire une petite animation pour le démarrage du jeu
void startUpGameAnimation() {
    
    // Ici on va aller chercher à faire clignoter les leds et jouer un jingle pour indiquer le début de la partie
    startUpGameTone();
    delay(200);

    // On va faire clignoter les leds 3 fois de suite
    for (int i = 0; i < 3; i++) { 
        tone(buzzerPin, 1000);

        // On va faire clignoter les leds 3 fois de suite sur le nombre de leds qu'on a
        for (int j = 0; j < numLeds; j++) {
            digitalWrite(buttonLedPins[j], HIGH);
        }

        // On va ajouter un délai de 200ms entre chaque clignotement
        delay(200);
        noTone(buzzerPin);

        // On va éteindre les leds
        for (int j = 0; j < numLeds; j++) {
            digitalWrite(buttonLedPins[j], LOW);
        }
        // On va rajouter un délai de 200ms entre chaque clignotement
        delay(200);
    }
}


// Ca sera la petite animation game over dans le cas ou le joueur a perdu
void gameOverAnimation(){

    // Bon la ca va juste allumer toutes es des d'un coup 1 fois
    for (int i = 0; i < 1; i++) { // i < 1 ca sera le nombre de fois ca va clignotter

        // On va allumer toutes les leds par le nombre de leds qu'on a
        for (int j = 0; j < numLeds; j++) { // j < 4 ca sera le nombre de led
        digitalWrite(buttonLedPins[j], HIGH);
        }

        // Jouer le jingle de game over
        gameOverTone();
        delay(200);

        // On va éteindre les leds en fonction du nombre de leds qu'on a
        for (int j = 0; j < numLeds; j++) {
        digitalWrite(buttonLedPins[j], LOW);
        }

        // On va arrêter le buzzer de jouer le jingle de game over
        noTone(buzzerPin);
        delay(150);

    }
}


// Animation pour si le joueur a gagné gagné en atteignant le nombre de séquence maximum !
// Note: Malheureusement on ne peu pas jouer la musique avec l'animation simultanément...
void gameVictoryAnimation(){
    
    delay(200);
    
    unsigned long startTime = millis();
    unsigned long duration = 4000; // 4 secondes
    
    // Pendant 4 secondes, on va allumer une LED aléatoirement
    while (millis() - startTime < duration) {
        
        // Allumer une LED aléatoirement pour l'animation de victoire
        int randomLed = random(numLeds);
        digitalWrite(buttonLedPins[randomLed], HIGH);

        // Délai de 5ms pour que la LED reste allumée
        delay(20);

        // Eteindre la LED aléatoirement pour l'animation de victoire
        digitalWrite(buttonLedPins[randomLed], LOW);

        delay(200);
    }
}



/**
   Game musics
   Note : Ca aurait été mieux de pouvoir séparer les musiques dans un autre fichier pour éviter d'avoir un fichier trop long
          mais apparament c'est compliqué de faire ca avec Arduino IDE.
*/

void startUpGameTone(){
  tone(buzzerPin, 880);
  delay(473);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(724);
  noTone(buzzerPin);

  tone(buzzerPin, 698);
  delay(391);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(449);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(721);
  noTone(buzzerPin);

  tone(buzzerPin, 698);
  delay(388);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(244);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(202);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(447);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(444);
  noTone(buzzerPin);

  tone(buzzerPin, 698);
  delay(201);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(215);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(449);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(444);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(210);
  noTone(buzzerPin);

  tone(buzzerPin, 659);
  delay(201);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(762);
  noTone(buzzerPin);
}


void playLevelUpTone() {
  tone(buzzerPin, 784);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 740);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 622);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 415);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 659);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 831);
  delay(141);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(362);
  noTone(buzzerPin);
}


void gameOverTone(){
  tone(buzzerPin, 622);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 554);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 554);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 554);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 466);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 466);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 466);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 415);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 370);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 349);
  delay(83);
  noTone(buzzerPin);

  tone(buzzerPin, 330);
  delay(400);
  noTone(buzzerPin);
}


void gameVictoryTone() {

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 392);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(1650);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 523);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(1650);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 392);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 440);
  delay(1650);
  noTone(buzzerPin);

  tone(buzzerPin, 494);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 587);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 1175);
  delay(4050);
  noTone(buzzerPin);

  tone(buzzerPin, 1175);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 880);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 659);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 1175);
  delay(1050);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 988);
  delay(294);
  noTone(buzzerPin);

  tone(buzzerPin, 784);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 1047);
  delay(500);
  noTone(buzzerPin);

  tone(buzzerPin, 1568);
  delay(3450);
  noTone(buzzerPin);
  
}
