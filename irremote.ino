#include <IRremote.h>
IRsend irsend;
IRrecv irrecv(11);
decode_results irresults;
  
void setup() { // INIT
  Serial.begin(19200);
  pinMode(A1, 'output'); // R1
  pinMode(A2, 'output'); // R2
  pinMode(A3, 'output'); // R3
  pinMode(A4, 'output'); // R4
  pinMode(2, 'output'); // BTN
  pinMode(3, 'output'); // IRT
  pinMode(5, 'input'); // LED
  pinMode(6, 'input'); // LED
  pinMode(11, 'input'); // IRR
  set(1, 1, 1, 1);
  setled(0);
  irrecv.enableIRIn();
  irrecv.blink13(true);
  Serial.println("<Ready>");
}

int inp_size = 0;
int stop_flg = 0;
int btn_lock = 0;

void loop() { // MAIN
  int push_btn = digitalRead(2);
  if (push_btn == 1 && btn_lock == 0){ btn_lock = 1; startelectric(); }
  if (Serial.available() > 0) {
    String in_data = Serial.readString();
    in_data.trim();
    if (in_data == "R_1_0"){ digitalWrite(A1, 1); }
    else if (in_data == "R_1_1"){ digitalWrite(A1, 0); }
    else if (in_data == "R_2_0"){ digitalWrite(A2, 1); }
    else if (in_data == "R_2_1"){ digitalWrite(A2, 0); }
    else if (in_data == "R_3_0"){ digitalWrite(A3, 1); }
    else if (in_data == "R_3_1"){ digitalWrite(A3, 0); }
    else if (in_data == "R_4_0"){ digitalWrite(A4, 1); }
    else if (in_data == "R_4_1"){ digitalWrite(A4, 0); }
    else if (in_data.substring(0, 7) == "IRSEND_"){
      String input = in_data.substring(7, in_data.length());
      long code = strtol(input.c_str(), NULL, 16);
      Serial.println("IR SEND "+input+" ("+getFunc(strtol(in_data.substring(9, in_data.length()).c_str(), NULL, 16))+") CODE");
      for (int i = 1; i<20; i++){
        irsend.sendNEC(code, 32);
      }
    }
    else if (in_data == "IRRECV"){
      Serial.println("START IRRECV");
      setled(2);
      while (btn_abort() != 1) {
        if (irrecv.decode(&irresults)){
          dump(&irresults);
          delay(10);
          irrecv.resume();
        }
      }
      setled(0);
      Serial.println("STOP IRRECV");
      while (digitalRead(2) == 1){ delay(10); }
    }
    else if (in_data == "STARTELECTRIC"){ startelectric(); }
  }
}

void set(int T1, int T2, int T3, int T4){
  digitalWrite(A1, T1);
  digitalWrite(A2, T2);
  digitalWrite(A3, T3);
  digitalWrite(A4, T4);
  delay(200);
}

void setled(int s){
  if (s == 0) {
    digitalWrite(5, 0);
    delay(10);
    digitalWrite(6, 0);
  } else if (s == 1) {
    digitalWrite(5, 1);
    delay(10);
    digitalWrite(6, 0);
  } else if (s == 2) {
    digitalWrite(5, 0);
    delay(10);
    digitalWrite(6, 1);
  }
}

int btn_abort(){
  if (Serial.readString() == "STOP\n" || digitalRead(2) == 1){ return 1; } else { return 0; }
}

long stol(String str){
  char c[str.length() + 1];
  str.toCharArray(c, str.length() + 1);
  return strtol(c, NULL, 16);  
}

void delay_interrupt(long ms){
  long currTime = millis();
  while (millis() < currTime+ms){
    if (btn_abort() != 1){ } else { break; }
  }
}

void startelectric(){
  Serial.println("START ELECTRIC");
  while (digitalRead(2) == 1){ delay(10); }
  while (btn_abort() != 1) {
    setled(1);
    long timer = 2000;
    while (timer < 30000 ){
      Serial.print("timer ");
      Serial.println(timer);
      if (btn_abort() != 1){ timer = timer + 1000;} else { break; }
      if (btn_abort() != 1){ digitalWrite(A2, 0); } else { break; }
      if (btn_abort() != 1){ delay_interrupt(1000); } else { break; }
      if (btn_abort() != 1){ digitalWrite(A2, 1); } else { break; }
      if (btn_abort() != 1){ delay_interrupt(timer); } else { break; }
    }
    if (btn_abort() != 1){ digitalWrite(A2, 0); } else { break; }
    if (btn_abort() != 1){ delay_interrupt(1000); } else { break; }
    if (btn_abort() != 1){ digitalWrite(A2, 1); } else { break; }
    if (btn_abort() != 1){ delay_interrupt(timer); } else { break; }
  }
  setled(0);
  digitalWrite(A2, 1);
  Serial.println("STOP ELECTRIC");
  while (digitalRead(2) == 1){ delay(10); }
  btn_lock = 0;
}

void dump(decode_results *results) {
  int count = results->rawlen;
  Serial.print(results->value, HEX); Serial.print(" ("); Serial.print(results->bits, DEC); Serial.println(" bits)");
  unsigned long vendor = results->value >> 16;
  unsigned long value = results->value & 0xFFFF;
  Serial.print("Function: 0x"); Serial.print(results->value & 0xFFFF, HEX); Serial.println(" (" + getFunc(value) + ")");
  unsigned long hash = 2166136261;
  for (int i = 1; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    hash = (hash * 16777619) ^ value;
  }
  Serial.print("Hash: "); Serial.println(hash);
}

int compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

String getFunc(unsigned long value) {
  String fname = "UNKNOWN";
        switch (value) {
          case 0x48B7: //BF48B7
            fname = "power"; break;
          case 0xA857: //BFA857
            fname = "1"; break;
          case 0x6897: //BF6897
            fname = "2"; break;
          case 0xE817: //BFE817
            fname = "3"; break;
          case 0x9867: //BF9867
            fname = "4"; break;
          case 0x58A7: //BF58A7
            fname = "5"; break;
          case 0xD827: //BFD827
            fname = "6"; break;
          case 0xB847: //BFB847
            fname = "7"; break;
          case 0x7887: //BF7887
            fname = "8"; break;
          case 0xF807: //BFF807
            fname = "9"; break;
          case 0x827D: //BF827D
            fname = "0"; break;
          case 0xE01F: //BFE01F
            fname = "menu"; break;
          case 0xB04F: //BFB04F
            fname = "ok"; break;
          case 0x18E7: //BF18E7
            fname = "channel_next"; break;
          case 0x38C7: //BF38C7
            fname = "channel_prev"; break;
          case 0xD02F: //BFD02F
            fname = "up"; break;
          case 0xF00F: //BFF00F
            fname = "down"; break;
          case 0x52AD: //BF52AD
            fname = "right"; break;
          case 0x926D: //BF926D
            fname = "left"; break;
          case 0x20DF: //BF20DF
            fname = "media_play_pause"; break;
          case 0x708F: //BF708F
            fname = "media_fastforward"; break;
          case 0x5AA5: //BF5AA5
            fname = "media_fastbackward"; break;
          case 0x40BF: //BF40BF
            fname = "back"; break;
          case 0x06F9: //BF06F9
            fname = "screen_size"; break;
          case 0x1AE5: //BF1AE5
            fname = "toggle"; break;
        }
  return fname;
}
