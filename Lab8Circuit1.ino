#include <LiquidCrystal.h>

#define BUTTON_PIN 3
#define START_BUTTON_PIN 2
#define BUZZER_PIN 4

#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.'
#define SPRITE_JUMP_LOWER 4
#define SPRITE_TERRAIN_EMPTY ' '
#define SPRITE_TERRAIN_SOLID 5
#define SPRITE_TERRAIN_SOLID_RIGHT 6
#define SPRITE_TERRAIN_SOLID_LEFT 7

#define HERO_HORIZONTAL_POSITION 1
#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

#define HERO_POSITION_OFF 0
#define HERO_POSITION_RUN_LOWER_1 1
#define HERO_POSITION_RUN_LOWER_2 2
#define HERO_POSITION_JUMP_1 3
#define HERO_POSITION_JUMP_2 4
#define HERO_POSITION_JUMP_3 5
#define HERO_POSITION_JUMP_4 6
#define HERO_POSITION_JUMP_5 7
#define HERO_POSITION_JUMP_6 8
#define HERO_POSITION_JUMP_7 9
#define HERO_POSITION_JUMP_8 10
#define HERO_POSITION_RUN_UPPER_1 11
#define HERO_POSITION_RUN_UPPER_2 12

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];
static bool buttonPushed = false;
static bool startButtonPushed = false;
static unsigned int maxScore = 0;

const int melody[] = {
  392,587,466,440,392,466,440,392,370,440,294,392,294,440,294,466,
  440,392,440,294,392,294,392,440,294,440,466,440,392,440,294,587,
  523,466,440,392,466,440,392,370,440,392,294,392,440,466,523,587,
  659,698,659,587,698,659,587,554,659,587,440,587,659,698,784,698,
  784,880,784,880,784,880,784,880,784,880,698,784,880,784,880,932,
  880,784,698,659,698,880,784,880,554,880,784,880,587,880,784,880,
  554,880,784,880
};
const int durations[] = {
  100,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8
};

int melodyLength = sizeof(melody) / sizeof(melody[0]);
int currentNote = 0;
unsigned long previousMillis = 0;
bool notePlaying = false;

void initializeGraphics() {
  static byte graphics[] = {
    B00111, B01111, B11110, B11100, B11110, B01111, B00111, B00000, 
    B00111, B01111, B11111, B11111, B11111, B01111, B00111, B00000,  
    B00111, B01111, B11110, B11100, B11110, B01111, B00111, B00000,  
    B11110, B01101, B11111, B10000, B00000, B00000, B00000, B00000,
    B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111,
    B00011, B00011, B00011, B00011, B00011, B00011, B00011, B00011,
    B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000,
  };
  for (int i = 0; i < 7; ++i)
    lcd.createChar(i + 1, &graphics[i * 8]);
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}

void playBackgroundMusic() {
  unsigned long currentMillis = millis();
  if (!notePlaying) {
    tone(BUZZER_PIN, melody[currentNote], durations[currentNote]);
    previousMillis = currentMillis;
    notePlaying = true;
  }
  if (notePlaying && (currentMillis - previousMillis >= durations[currentNote])) {
    noTone(BUZZER_PIN);
    currentNote++;
    if (currentNote >= melodyLength) currentNote = 0;
    notePlaying = false;
  }
}

void buttonCheck() {
  if (digitalRead(BUTTON_PIN) == LOW) buttonPushed = true;
  if (digitalRead(START_BUTTON_PIN) == LOW) startButtonPushed = true;
}

void advanceTerrain(char* terrain, byte newTerrain) {
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
    switch (current) {
      case SPRITE_TERRAIN_EMPTY:
        terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID_RIGHT : SPRITE_TERRAIN_EMPTY;
        break;
      case SPRITE_TERRAIN_SOLID:
        terrain[i] = (next == SPRITE_TERRAIN_EMPTY) ? SPRITE_TERRAIN_SOLID_LEFT : SPRITE_TERRAIN_SOLID;
        break;
      case SPRITE_TERRAIN_SOLID_RIGHT: terrain[i] = SPRITE_TERRAIN_SOLID; break;
      case SPRITE_TERRAIN_SOLID_LEFT:  terrain[i] = SPRITE_TERRAIN_EMPTY; break;
    }
  }
}

bool drawHero(byte position, char* terrainUpper, char* terrainLower, unsigned int score) {
  bool collide = false;
  char upperSave = terrainUpper[HERO_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[HERO_HORIZONTAL_POSITION];
  byte upper, lower;

  switch (position) {
    case HERO_POSITION_OFF: upper = lower = SPRITE_TERRAIN_EMPTY; break;
    case HERO_POSITION_RUN_LOWER_1: upper = SPRITE_TERRAIN_EMPTY; lower = SPRITE_RUN1; break;
    case HERO_POSITION_RUN_LOWER_2: upper = SPRITE_TERRAIN_EMPTY; lower = SPRITE_RUN2; break;
    case HERO_POSITION_JUMP_1:
    case HERO_POSITION_JUMP_8: upper = SPRITE_TERRAIN_EMPTY; lower = SPRITE_JUMP; break;
    case HERO_POSITION_JUMP_2:
    case HERO_POSITION_JUMP_7: upper = SPRITE_JUMP_UPPER; lower = SPRITE_JUMP_LOWER; break;
    case HERO_POSITION_JUMP_3:
    case HERO_POSITION_JUMP_4:
    case HERO_POSITION_JUMP_5:
    case HERO_POSITION_JUMP_6: upper = SPRITE_JUMP; lower = SPRITE_TERRAIN_EMPTY; break;
    case HERO_POSITION_RUN_UPPER_1: upper = SPRITE_RUN1; lower = SPRITE_TERRAIN_EMPTY; break;
    case HERO_POSITION_RUN_UPPER_2: upper = SPRITE_RUN2; lower = SPRITE_TERRAIN_EMPTY; break;
  }

  if (upper != ' ') {
    terrainUpper[HERO_HORIZONTAL_POSITION] = upper;
    collide = (upperSave != SPRITE_TERRAIN_EMPTY);
  }
  if (lower != ' ') {
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave != SPRITE_TERRAIN_EMPTY);
  }

  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;

  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';

  char temp = terrainUpper[16 - digits];
  terrainUpper[16 - digits] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(terrainUpper);
  terrainUpper[16 - digits] = temp;

  lcd.setCursor(0, 1);
  lcd.print(terrainLower);

  lcd.setCursor(16 - digits, 0);
  lcd.print(score);

  terrainUpper[HERO_HORIZONTAL_POSITION] = upperSave;
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave;

  return collide;
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  lcd.begin(16, 2);
  initializeGraphics();
}

void loop() {
  buttonCheck();
  playBackgroundMusic();

  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = TERRAIN_EMPTY;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;

  if (!playing) {
    drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainUpper, terrainLower, distance >> 3);

    lcd.setCursor(0, 0);
    lcd.print("Runner           ");  
    lcd.setCursor(0, 1);
    lcd.print("Presione boton   ");

    if (maxScore > 0) {
      lcd.setCursor(0, 0);
      lcd.print("Ptj Max:");
      lcd.print(maxScore);
    }

    delay(500);
    blink = !blink;

    if (startButtonPushed) {
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      startButtonPushed = false;
      distance = 0;
    }
    return;
  }

  advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
  advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);

  if (--newTerrainDuration == 0) {
    newTerrainType = (newTerrainType == TERRAIN_EMPTY) ?
                     ((random(3) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK) :
                     TERRAIN_EMPTY;
    newTerrainDuration = (newTerrainType == TERRAIN_EMPTY) ? 10 + random(10) : 2 + random(10);
  }

  if (buttonPushed) {
    if (heroPos <= HERO_POSITION_RUN_LOWER_2) heroPos = HERO_POSITION_JUMP_1;
    buttonPushed = false;
  }

  if (drawHero(heroPos, terrainUpper, terrainLower, distance >> 3)) {
    playing = false;
    tone(BUZZER_PIN, 400, 300);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Perdiste! Ptj:");
    lcd.print(distance >> 3);
    if ((distance >> 3) > maxScore) maxScore = distance >> 3;
    lcd.setCursor(0, 1);
    lcd.print("Ptj Max: ");
    lcd.print(maxScore);
    delay(2000);
    return;
  } else {
    if (heroPos == HERO_POSITION_RUN_LOWER_2 || heroPos == HERO_POSITION_JUMP_8)
      heroPos = HERO_POSITION_RUN_LOWER_1;
    else if ((heroPos >= HERO_POSITION_JUMP_3 && heroPos <= HERO_POSITION_JUMP_5) &&
             terrainLower[HERO_HORIZONTAL_POSITION] != SPRITE_TERRAIN_EMPTY)
      heroPos = HERO_POSITION_RUN_UPPER_1;
    else if (heroPos >= HERO_POSITION_RUN_UPPER_1 &&
             terrainLower[HERO_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY)
      heroPos = HERO_POSITION_JUMP_5;
    else if (heroPos == HERO_POSITION_RUN_UPPER_2)
      heroPos = HERO_POSITION_RUN_UPPER_1;
    else
      ++heroPos;
    ++distance;
  }

  delay(100);
}
