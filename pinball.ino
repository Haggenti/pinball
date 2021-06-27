#include <avr/io.h>
#include <EEPROM.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define DATA_PIN  45 // MOSI - DIN - marron
#define CLK_PIN   46 // SCK - CLK - vert
#define CS_PIN    47 // SS - CS - blanc

struct hiscores{uint16_t score; char name[5];};

MD_Parola DMD = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
 
 //DMD stuff
uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE  75
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Pinball machine Ready !" };
char alphabet[38] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<"} ;
bool newMessageAvailable = true;

  //solenoid activation times
  uint32_t fire_sol_ball;
  uint32_t sol_timer_rearm;
  uint32_t sol_timer_ball;
  uint32_t sol_timer_h1;
  uint32_t sol_timer_h2;
  uint32_t sol_timer_s1;
  uint32_t sol_timer_s2;
  uint32_t sol_timer_b1;
  uint32_t sol_timer_b2;
  const uint8_t soso=65;

  // pwm variables

  uint8_t f_duty=100; // power of flippers in percent
  uint8_t s_duty=75;  // power of slingshots in percent

  boolean f1_active,f1_state;
  boolean f2_active,f2_state;
  uint8_t f1_c_on,f1_c_off,f2_c_on,f2_c_off;
  const uint8_t pwm_f_length=20;
  uint8_t f2_on_per=(20*f_duty)/100;
  uint8_t f1_on_per=f2_on_per;
  uint8_t f2_off_per=pwm_f_length-f2_on_per;
  uint8_t f1_off_per=pwm_f_length-f1_on_per;

  boolean s1_active,s1_state;
  boolean s2_active,s2_state;
  uint8_t s1_c_on,s1_c_off,s2_c_on,s2_c_off;
  const uint8_t pwm_s_length=20;
  uint8_t s2_on_per=(20*s_duty)/100;
  uint8_t s1_on_per=s2_on_per;
  uint8_t s2_off_per=pwm_s_length-s2_on_per;
  uint8_t s1_off_per=pwm_s_length-s1_on_per;



  // functionnals variables

  hiscores hi[6];
  long start_time;
  long printduration=1000;
  uint32_t saveball_time=10000;
  uint8_t ballonplayfield=0;
  uint8_t ball;
  boolean empty=1;
  boolean ll=1, al, lr=1, ar, lo=1, ao;
  boolean activate_newball;
  boolean ended=0;
  boolean saveball=0;
  boolean saveball_lock;
  boolean block1;
  boolean block2;
  boolean ballblocked1;
  boolean ballblocked2;
  boolean lhole1=1, ahole1;
  boolean lhole2=1, ahole2;
  boolean ldrain=1, adrain;
  boolean lds1=1, ads1;
  boolean lds2=1, ads2;
  boolean lds3=1, ads3;
  boolean lft1=1, aft1;
  boolean lft2=1, aft2;
  boolean lbmp1=1, abmp1;
  boolean lbmp2=1, abmp2;
  boolean lsling1=1, asling1;
  boolean lsling2=1, asling2;
  boolean lramp=1, aramp;
  boolean lpass1=1, apass1, pass1_light;
  boolean lpass2=1, apass2, pass2_light;
  boolean lleft=1, aleft;
  boolean lright=1, aright;
  boolean llflp=1, alflp;
  boolean lrflp=1, arflp;
  uint16_t deb[21];
  uint16_t debmax=200;

  uint32_t p_score;
  uint32_t score;

  //score values
   uint8_t bonus_mult=1;
   uint8_t ball_max=3;
   uint16_t hole1_value=50;
   uint16_t hole2_value=50;
   uint16_t bumper_value=100;
   uint16_t drop_target_value=75;
   uint16_t drop_target_extra_value=750;
   uint16_t extra_ball1=5000;
   uint16_t extra_ball2=12000;
   uint16_t extra_ball3=30000;
   uint16_t ft1_value=50;
   uint16_t ft2_value=50;
   uint16_t sling_value=50;
   uint16_t ramp_value=50;
   uint16_t pass_value=50;
   uint16_t looseway_value=500;
   uint16_t flipway_value=40;

 // activation counters
  uint16_t drop_counter;
  uint16_t ramp_counter;
  uint16_t bump_counter;
  uint16_t ft1_counter;
  uint16_t ft2_counter;
  uint16_t h1_counter;
  uint16_t h2_counter;
  uint16_t left_loose_counter;
  uint16_t right_loose_counter;
  uint16_t left_flip_counter;
  uint16_t right_flip_counter;

  //solenoids : 9 outputs
  //power_relay=32; // PC5
  //hole1_sol=33;   // PC4
  //hole2_sol=34;   // PC3
  //new_ball_sol=35;// PC2
  //drop_sol=36;    // PC1
  //sling1=37;      // PC0
  //sling2=38;      // PD7
  //bump1=39        // PG2
  //bump1=40        // PG1
  //flip1           // PL7
  //flip2           // PL6
  

  //sensors - 22 input sensors
 
  const uint8_t start_button=10;  //PB4
  const uint8_t hole1_sen=11;     //PB5
  const uint8_t hole2_sen=12;     //PB6
  const uint8_t drain_sen=13;     //PB7
  const uint8_t drop1_sen=14;     //PJ1
  const uint8_t drop2_sen=15;     //PJ0
  const uint8_t drop3_sen=16;     //PH1
  const uint8_t fix1_sen=17;      //PH0
  const uint8_t fix2_sen=18;      //PD3
  const uint8_t bump1_sen=19;     //PD2
  const uint8_t bump2_sen=20;     //PD1
  const uint8_t sling1_sen=21;    //PD0
  const uint8_t sling2_sen=22;    //PA0
  const uint8_t ramp_sen=23;      //PA1
  const uint8_t pass1_sen=24;     //PA2
  const uint8_t pass2_sen=25;     //PA3
  const uint8_t left_sen=26;      //PA4
  const uint8_t right_sen=27;     //PA5
  const uint8_t lflp_sen=28;      //PA6
  const uint8_t rflp_sen=29;      //PA7
  const uint8_t flip_l_sen=30;    //PC7
  const uint8_t flip_r_sen=31;    //PC6


void setup(){
   Serial.begin(57600);
   DMD.begin();

  //pinmode define input/output
  pinMode( start_button, INPUT_PULLUP);
  pinMode( hole1_sen, INPUT_PULLUP);
  pinMode( hole2_sen, INPUT_PULLUP);
  pinMode( drain_sen, INPUT);
  pinMode( drop1_sen, INPUT_PULLUP);
  pinMode( drop2_sen, INPUT_PULLUP);
  pinMode( drop3_sen, INPUT_PULLUP);
  pinMode( fix1_sen, INPUT_PULLUP);
  pinMode( fix2_sen, INPUT_PULLUP);
  pinMode( bump1_sen, INPUT_PULLUP);
  pinMode( bump2_sen, INPUT_PULLUP);
  pinMode( sling1_sen, INPUT_PULLUP);
  pinMode( sling2_sen, INPUT_PULLUP);
  pinMode( ramp_sen, INPUT_PULLUP);
  pinMode( pass1_sen, INPUT_PULLUP);
  pinMode( pass2_sen, INPUT_PULLUP);
  pinMode( left_sen, INPUT_PULLUP);
  pinMode( right_sen, INPUT_PULLUP);
  pinMode( lflp_sen, INPUT_PULLUP);
  pinMode( rflp_sen, INPUT_PULLUP);
  pinMode( flip_l_sen, INPUT_PULLUP);
  pinMode( flip_r_sen, INPUT_PULLUP);
  DDRC |= (1 << PIN5);
  DDRC |= (1 << PIN4);
  DDRC |= (1 << PIN3);
  DDRC |= (1 << PIN2);
  DDRC |= (1 << PIN1);
  DDRC |= (1 << PIN0);
  DDRD |= (1 << PIN7);
  DDRG |= (1 << PIN2);
  DDRG |= (1 << PIN1);
  DDRL |= (1 << PIN7);
  DDRL |= (1 << PIN6);
  
   //read eeprom hiscores
   int eeAddress = 0; //EEPROM address to start reading from
   for(int i=1; i<5; i++){
       EEPROM.get( eeAddress, hi[i] );
       eeAddress+=sizeof(hi[i]);
   }

  init_scores();
  //sort_hiscores();
  //write_eeprom();
  //enteryourname();


    uint8_t self=self_check();
    if (self!=0) {
      DMD.print("Self check failed");
      delay(1500);
      DMD.print("System halted.");
      for(;;){}
    }

PORTC |= (1 << PIN5); // disable solenoids VCC 24V

scrollPause=0;
strcpy(curMessage, newMessage);
DMD.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
while(!DMD.displayAnimate()){}
scrollEffect=PA_SCROLL_UP;
scrollPause=1500;
DMD.setTextEffect(scrollEffect, scrollEffect);
DMD.setPause(scrollPause);

} //end off setup function

void loop() {
      PORTC |= (1 << PIN5); // disable solenoids VCC 24V
    int index=0;
      do {

        // display hiscores on DMD with t_hi function
        
      if(DMD.displayAnimate()) {
        ++index;
        if (index>10) index=1;
        if(index<10){
          scrollEffect=PA_SCROLL_UP;
          scrollPause=1500;
          DMD.setTextAlignment(PA_CENTER);
          DMD.setTextEffect(scrollEffect, scrollEffect);
          DMD.setPause(scrollPause);
        }
        if(index==10){
          scrollEffect=PA_SCROLL_LEFT;
          scrollPause=0;
          DMD.setTextAlignment(PA_LEFT);
          DMD.setTextEffect(scrollEffect, scrollEffect);
          DMD.setPause(scrollPause);
        }
        if (index==1) {strcpy(curMessage, "Hiscores");}
        if (index==2) {strcpy(curMessage, hi[1].name);}
        if (index==3) {itoa (hi[1].score,newMessage,10);strcpy(curMessage, newMessage);}
        if (index==4) strcpy(curMessage, hi[2].name);
        if (index==5) {itoa (hi[2].score,newMessage,10);strcpy(curMessage, newMessage);}
        if (index==6) strcpy(curMessage, hi[3].name);
        if (index==7) {itoa (hi[3].score,newMessage,10);strcpy(curMessage, newMessage);}
        if (index==8) strcpy(curMessage, hi[4].name);
        if (index==9) {itoa (hi[4].score,newMessage,10);strcpy(curMessage, newMessage);}
        if (index==10) strcpy(curMessage, "Press start button !");
        DMD.displayReset();
        }
      } while ((PINB & (1<<PB4)));  // wait for start button


      PORTC &= ~(1 << PIN5);  // enable solenoids VCC


      // raise drop targets & eject balls in holes
      PORTC |= (1 << PIN1);  
      PORTC |= (1 << PIN4);
      PORTC |= (1 << PIN3);
      delay(soso);
      PORTC &= ~(1 << PIN1); 
      PORTC &= ~(1 << PIN4);
      PORTC &= ~(1 << PIN3);

       reset_variables();


    //game main loop  


    do {
      fall_drain();
      if (ball==ball_max && ballonplayfield==0 && !saveball) ended=1;
      if (ballonplayfield==0 && ball<(ball_max) && !saveball) {
        ++ball; 
        start_time=millis();
        fire_sol_ball=millis();
        saveball=1;
        saveball_lock=0;
        char toprint[]="Ball : ";
        char ballchar[1];
        itoa (ball,ballchar,10);
        strcat(toprint, ballchar);
        strcpy(curMessage, toprint);
        empty=1;
        DMD.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
        DMD.displayReset();
        get_ball();
      }

        if (ballonplayfield==0 && saveball) {
        saveball=0; 
        if(saveball_lock==1) saveball_lock=0;
        fire_sol_ball=millis();
        strcpy(curMessage, "Ball saved !");
        empty=1;
        DMD.displayText(curMessage, PA_CENTER, scrollSpeed, 1500, PA_BLINDS, PA_BLINDS);
        DMD.displayReset();
        get_ball();
      }
      
      if ((millis()-start_time)>=saveball_time && saveball==1 && !saveball_lock) {
        saveball=0;
        strcpy(curMessage, "Save OFF!");
        DMD.displayText(curMessage, PA_CENTER, scrollSpeed, 1500, PA_BLINDS, PA_BLINDS);
        empty=1;
        } 
      get_ball();
      flips();
      DMD.displayAnimate();
      extra_ball_req();
      block_req();
      save_req();
      hole1();
      hole2();
      multiball();
      drop_target();
      fixed_target();
      bumper();
      sling();
      //ramp();
      global();
      for(int i=0; i<21; i++){
        if (deb[i]<debmax)  ++deb[i]; // count debounce cycles for each sensor
          
      }

      if (p_score!=score || (empty && DMD.displayAnimate())) {
        p_score=score;
        empty=0;
        itoa (score,newMessage,10);
        strcpy(curMessage, newMessage);
        DMD.displayText(curMessage, PA_RIGHT, 25, 0, PA_SCROLL_UP, PA_NO_EFFECT);
      }

      } while (!ended); //loop until ball quota exceded

    extra_ball1=5000;
    extra_ball2=12000;
    extra_ball3=30000;
    PORTC |= (1 << PIN5); // disable solenoids VCC 24V
    while(!DMD.displayAnimate()){}
    scrollEffect=PA_WIPE;
    scrollPause=2000;
    DMD.setTextAlignment(PA_CENTER);
    DMD.setTextEffect(scrollEffect, scrollEffect);
    DMD.setPause(scrollPause);
    DMD.displayReset();
    strcpy(curMessage, "Game over");
    while(!DMD.displayAnimate()){}
     DMD.displayReset();
     scrollEffect=PA_SCROLL_DOWN;
    scrollPause=1500;
    DMD.setTextAlignment(PA_CENTER);
    DMD.setTextEffect(scrollEffect, scrollEffect);
    DMD.setPause(scrollPause);
    strcpy(curMessage, "Scored");
    while(!DMD.displayAnimate()){}
     DMD.displayReset();
      DMD.setTextAlignment(PA_CENTER);
    DMD.setTextEffect(PA_OPENING_CURSOR, PA_OPENING_CURSOR);
    itoa (score,newMessage,10);
    strcpy(curMessage, newMessage);
    while(!DMD.displayAnimate()){}
   // if (score>=hi[4].score) enteryourname();
    delay(1000);    
}

void init_scores(){

  strcpy(hi[1].name,"AAA");
  hi[1].score=10;
  strcpy(hi[2].name,"BBB");
  hi[2].score=20;
  strcpy(hi[3].name,"CCC");
  hi[3].score=30;
  strcpy(hi[4].name,"DDD");
  hi[4].score=40;
  strcpy(hi[5].name,"JOE");
  hi[5].score=50;
}

void write_eeprom() {
    int eeAddress = 0; 
   for(int i=1; i<5; i++){
       EEPROM.put( eeAddress, hi[i]);
       eeAddress+=sizeof(hi[i]);
      }
}

void flips(){
    al=(PINC & (1<<PC7)); // reads left flip
    ar=(PINC & (1<<PC6)); // read right flip
    if(ll==1 && al==0) {ll=al;f2_active=1;}
    if(lr==1 && ar==0) {lr=ar;f1_active=1;}
    if(ll==0 && al==1) {ll=al;f2_active=0;}
    if(lr==0 && ar==1) {lr=ar;f1_active=0;}
    pwm_f1();
    pwm_f2();

}







void enteryourname() {
  int point=0;
  int selector=0;
  DMD.displayText("Hiscore !", PA_CENTER, 25, 500, PA_SCROLL_UP, PA_SCROLL_DOWN);
  while(!DMD.displayAnimate()){}
  DMD.displayText("Enter your initials", PA_CENTER, 25, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while(!DMD.displayAnimate()){}
  hi[5].score=score;
  hi[5].name[0]='A';
  hi[5].name[1]=0;
  hi[5].name[2]=0;
  hi[5].name[point]=alphabet[selector];
  DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_NO_EFFECT );
  while(!DMD.displayAnimate()){} 
  do{
      al=(PINC & (1<<PC7)); // reads left flip
      ar=(PINC & (1<<PC6)); // read right flip
      ao=(PINB & (1<<PB4)); // read start button
      ++deb[20];
      ++deb[21];
      ++deb[22];
      DMD.displayAnimate();
    if(ll && !al && selector>=1  && deb[20]>=debmax) {
      --selector;
      deb[20]=0;
      hi[5].name[point]=alphabet[selector];
      DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_NO_EFFECT );
      while(!DMD.displayAnimate()){}    
    }
    if(lr && !ar && selector<36 && deb[21]>=debmax) {
      ++selector;
      deb[21]=0;
      hi[5].name[point]=alphabet[selector];
      DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_NO_EFFECT );
      while(!DMD.displayAnimate()){}
    }
    
  if (lo && !ao && selector!=36 && deb[22]>=debmax){
   
    ++point;
    deb[22]=0;
    if (point<3){
     hi[5].name[point]=alphabet[selector];
      DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_NO_EFFECT );
      while(!DMD.displayAnimate()){}  }
    delay(500);
    
      
  } 
  if (lo && !ao && selector==36 && point>0 && deb[22]>=debmax*10){
    hi[5].name[point]=0;
    --point;
    deb[22]=0;
      DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_NO_EFFECT );
      while(!DMD.displayAnimate()){} 
  }

  ll=al;
  lr=ar;
  lo=ao;
  } while (point<3);
  DMD.displayText(hi[5].name, PA_LEFT, DMD.getSpeed(), 0, PA_PRINT , PA_SCROLL_RIGHT );
  while(!DMD.displayAnimate()){}  
  //sort hiscores and drop n°5
  sort_hiscores();
  write_eeprom();
}
  

//earn extra ball when score reach 3 levels
void extra_ball_req() {
   if (score>=extra_ball3 && extra_ball3!=0) {
     ++ball_max;
     strcpy(curMessage, "Extra ball 1!");
     DMD.displayReset();
     extra_ball3=0;
     }

  if (score>=extra_ball2 && extra_ball2!=0) {
    ++ball_max;
    strcpy(curMessage, "Extra ball 2!");
    DMD.displayReset();
    extra_ball2=0;
    }

  if (score>=extra_ball1 && extra_ball1!=0) {
    ++ball_max;
    strcpy(curMessage, "Extra ball 3!");
     DMD.displayReset();
    extra_ball1=0;
    }
}

void serial_printhi()  {
      Serial.println("Hiscores");
      for(int i=1; i<5; i++) {
        Serial.print(i);
        Serial.print(" : ");
        Serial.print(hi[i].name);
        Serial.print(" -> ");
        Serial.println(hi[i].score);
    }
}


void sort_hiscores() {
  char temp_hi[3];
  uint16_t temp_sc;

  for(int sortii=0; sortii<5; ++sortii) {
    for(int sorti=4; sorti>0; --sorti) {
      if ((hi[sorti].score)<(hi[sorti+1].score)) {
   
        strcpy(temp_hi, hi[sorti].name);
        strcpy(hi[sorti].name, hi[sorti+1].name);
        strcpy(hi[sorti+1].name,temp_hi);

        temp_sc=hi[sorti].score;
        hi[sorti].score=hi[sorti+1].score;
        hi[sorti+1].score=temp_sc;
      }
    }
  }
}


void multiball() {
  // take ramp when balls are blocked to launch multiball !!!!!
    if (ballblocked2 && ballblocked1 && ramp_counter==1) {
    if (sol_timer_h1==0){
      PORTC |= (1 << PIN4);
      PORTC |= (1 << PIN3);
      sol_timer_h1=millis();
    }
    
    if (((millis()-sol_timer_h1)>soso) && sol_timer_h1!=0) {
      PORTC &= ~(1 << PIN4);
      PORTC &= ~(1 << PIN3);
      ballblocked1=0;
      sol_timer_h1=0;
      ballblocked2=0;
      sol_timer_h2=0;
    }  

    if (!ballblocked2 && !ballblocked1) {
      strcpy(curMessage, "Multi ball ready");
      DMD.displayReset();
      empty=1;
    }
  }
}


void save_req() {
  // 4 passages each to activate save ball
  if ( left_flip_counter>=4 && right_flip_counter>=4) {
    right_flip_counter=0;
    left_flip_counter=0;
    saveball=1;
    saveball_lock=1;
    strcpy(curMessage, "Save ball award !");
    DMD.displayReset();
    empty=1;
  }

}



//to enable lock ball, kill 2 rows of drop targets and touch each fixed target once
void block_req(){
  if (drop_counter==2 && ft1_counter==1 && ft2_counter==1 && block1==0 && !ballblocked1) {
      block1=1;
        drop_counter=0;
        ft1_counter=0;
        ft2_counter=0;
        strcpy(curMessage, "Block a ball !");
        DMD.displayReset();
     }

  if (drop_counter==2 && ft1_counter==1 && ft2_counter==1 && block2==0 && !ballblocked2) {
      block2=1;
        drop_counter=0;
        ft1_counter=0;
        ft2_counter=0;
        strcpy(curMessage, "Block a ball !");
        DMD.displayReset();
     }
}


void global(){
  aleft=(PINA & (1<<PA4)); // reads left loose sensor
  aright=(PINA & (1<<PA5)); // reads right loose sensor
  alflp=(PINA & (1<<PA6)); // reads left flip lane
  arflp=(PINA & (1<<PA7)); // reads right flip lane
  if (lleft && !aleft && deb[16]>=debmax) {score+=looseway_value;++left_loose_counter;deb[16]=0;} 
  if (lright && !aright && deb[17]>=debmax) {score+=looseway_value;++right_loose_counter;deb[17]=0;} 
  if (llflp && !alflp && deb[18]>=debmax) {score+=flipway_value;++left_flip_counter;deb[18]=0;} 
  if (lrflp && !arflp && deb[19]>=debmax) {score+=flipway_value;++right_flip_counter;deb[19]=0;} 
  lleft=aleft;
  lright=aright;
  llflp=alflp;
  lrflp=arflp;
}






// activate passage, and advance bonus up to 4
void passage(){
  apass1=(PINA & (1<<PA2)); // reads pass 1
  apass2=(PINA & (1<<PA3)); // reads pass 2
  if (lpass1 && !apass1 && deb[14]>=debmax) {score=+pass_value; pass1_light=1;deb[14]=0;} 
  if (lpass2 && !apass2 && deb[15]>=debmax) {score=+pass_value; pass2_light=1;deb[15]=0;}
  if (pass1_light && pass2_light && bonus_mult<4) {
    ++bonus_mult;
    strcpy(curMessage, "Advance bonus:X");
    DMD.displayReset();
    DMD.print(bonus_mult);

    pass1_light=0;
    pass2_light=0;
  }
  lpass1=apass1;
  lpass2=apass2;
}



void ramp() {
    aramp=(PINA & (1<<PA1)); // reads ramp
    if (lramp && !aramp && deb[13]>=debmax) {
      score=score+ramp_value;
      ++ramp_counter;
      deb[13]=0;
    } 
    lramp=aramp;
    strcpy(curMessage, "Ramp !");
    DMD.displayReset();
}


void fixed_target(){

  aft1=(PINH & (1<<PH0)); // reads ft1
  aft2=(PIND & (1<<PD3)); // reads ft2
  if (lft1 && !aft1 && deb[11]>=debmax) {score=+ft1_value;++ft1_counter;deb[11]=0;} 
  if (lft2 && !aft2 && deb[12]>=debmax) {score=+ft2_value;++ft2_counter;deb[12]=0;}
  lft1=aft1;
  lft2=aft2;
}


void bumper(){
  
  abmp1=(PIND & (1<<PD2)); // reads bump1
  abmp2=(PIND & (1<<PD1)); // reads bump2

  if (lbmp1 && !abmp1 && deb[9]>=debmax) {
    PORTG |= (1 << PIN2);
    sol_timer_b1=millis();
    score=+(bumper_value*bonus_mult);
    ++bump_counter;
    deb[9]=0;
  } 

  if (lbmp2 && !abmp2 && deb[10]>=debmax) {
    PORTG |= (1 << PIN1);
    sol_timer_b2=millis();
    score=+(bumper_value*bonus_mult);
    ++bump_counter;
    deb[10]=0;
  } 

  if (((millis()-sol_timer_b1)>soso) && sol_timer_b1!=0) {PORTG &= ~(1 << PIN2);sol_timer_b1=0;} 
  if (((millis()-sol_timer_b2)>soso) && sol_timer_b2!=0) {PORTG &= ~(1 << PIN1);sol_timer_b2=0;} 
  // add flash to bumpers ?
  lbmp1=abmp1;
  lbmp2=abmp2;
}

void sling(){

  asling1=(PIND & (1<<PD0)); // reads sling1
  asling2=(PINA & (1<<PA0)); // reads sling2
  if ((lsling1 && !asling1 && deb[7]>=debmax && sol_timer_s1==0)) {
    //PORTC |= (1 << PIN0);
    s1_active=1;
    sol_timer_s1=millis();
    score=score+sling_value*bonus_mult;
    deb[7]=0;
  } 

  if ((!lsling2 && asling2 && deb[8]>=debmax && sol_timer_s2==0)) {
    //PORTD |= (1 << PIN7);
    s2_active=1;
    sol_timer_s2=millis();
    score=score+sling_value*bonus_mult;
    deb[8]=0;
    
  } 

  lsling1=asling1;
  lsling2=asling2;
  if (((millis()-sol_timer_s1)>(soso/2)) && sol_timer_s1!=0) {s1_active=0;sol_timer_s1=0;} //desactivate solenoid  
  if (((millis()-sol_timer_s2)>(soso/2)) && sol_timer_s2!=0) {s2_active=1;sol_timer_s2=0;} //desactivate solenoid  

pwm_s1();
pwm_s2();

}


// drop targets score - 5 rows of targets down gives extra bonus
void drop_target(){
  if (((millis()-sol_timer_rearm)>soso) && sol_timer_rearm!=0) {PORTC &= ~(1 << PIN1);sol_timer_rearm=0;} //desactivate solenoid after soso delay
  ads1 = (PINJ & (1<<PJ1)); // reads drop¹
  ads2 = (PINJ & (1<<PJ0)); // reads drop2
  ads3 = (PINH & (1<<PH1)); // reads drop3
  if (lds1 && !ads1 && deb[4]>=debmax) {score=score+drop_target_value;deb[4]=0;}
  if (lds2 && !ads2 && deb[5]>=debmax) {score=score+drop_target_value;deb[5]=0;}
  if (lds3 && !ads3 && deb[6]>=debmax) {score=score+drop_target_value;deb[6]=0;}
  if (!ads1 && !ads2 && !ads3) {
    // do something special ........ when all target down
    PORTC |= (1 << PIN1);
    sol_timer_rearm=millis();
    ++drop_counter;
    if (drop_counter>5) {
      score=score+drop_target_extra_value;
      drop_counter=0;
      strcpy(curMessage, "5 Rows of traget down ! - extra bonus !");
      DMD.displayReset();
    }
  }
  lds1=ads1;
  lds2=ads2;
  lds3=ads3;
}
;



void hole1 () {
  ahole1=(PINB & (1<<PB5)); //read hole1 sensor
  if (lhole1 && !ahole1 && deb[3]>=debmax) { // if sensor fall
    deb[3]=0;
    ballblocked1=1;
    score+=hole1_value;
    ++h1_counter;
    strcpy(curMessage, "Hole 1 hit");
    DMD.displayReset();
    if (block1) {
      --ballonplayfield;
      strcpy(curMessage, "Ball H1 bloqued !");
      DMD.displayReset();
      if (ballblocked1 && ballblocked2) {
      strcpy(curMessage, "Take ramp to multiball !");
      DMD.displayReset();
      }
      get_ball();
      block1=0;
      block2=0;
      ramp_counter=0;
      drop_counter=0;
      ft1_counter=0;
      ft2_counter=0;
      h1_counter=0;
    }
  }
  if (ballblocked1 && !block1) {
    if (sol_timer_h1==0) sol_timer_h1=millis();
    if (((millis()-sol_timer_h1)>700) && ((millis()-sol_timer_h1)<(soso*3)) && sol_timer_h1!=0) {PORTC |= (1 << PIN4);} //activate solenoid after 700 ms
    if (((millis()-sol_timer_h1)>(700+soso)) && sol_timer_h1!=0) {PORTC &= ~(1 << PIN4);ballblocked1=0;sol_timer_h1=0;} //desactivate solenoid after 700+soso ms 
  }
  lhole1=ahole1;
}


void hole2 () {
  ahole2=(PINB & (1<<PB6)); //read hole2 sensor
  if (lhole2 && !ahole2 && deb[2]>=debmax) { // if sensor fall
    deb[2]=0;
    ballblocked2=1;
    score+=hole2_value;
    ++h2_counter;
    strcpy(curMessage, "Hole 2 hit");
    DMD.displayReset();
    if (block2) {
      --ballonplayfield;
      strcpy(curMessage, "Ball H2 bloqued !");
      DMD.displayReset();
      if (ballblocked1 && ballblocked2) {
      strcpy(curMessage, "Take ramp to multiball !");
      DMD.displayReset();
      }
      get_ball();
      block1=0;
      block2=0;
      ramp_counter=0;
      drop_counter=0;
      ft1_counter=0;
      ft2_counter=0;
      h1_counter=0;
    }
  }
  if (ballblocked2 && !block2) {
    if (sol_timer_h2==0) sol_timer_h2=millis();
    if (((millis()-sol_timer_h2)>700) && ((millis()-sol_timer_h2)<(soso*3)) && sol_timer_h2!=0) {PORTC |= (1 << PIN3);} //activate solenoid after soso 700 ms
    if (((millis()-sol_timer_h2)>(700+soso)) && sol_timer_h2!=0) {PORTC &= ~(1 << PIN3);ballblocked2=0;sol_timer_h2=0;} //desactivate solenoid after soso 700+soso ms 
  }
  lhole2=ahole2;
}
 

void get_ball() {
  if (ballonplayfield==0 && ended==0 ) {
    activate_newball=1;
    ++ballonplayfield;
  }
  if((millis()-fire_sol_ball>1400) && activate_newball){PORTC |= (1 << PIN2);activate_newball=0;sol_timer_ball=millis();}
  if (((millis()-sol_timer_ball)>(soso+50)) && sol_timer_ball!=0) {PORTC &= ~(1 << PIN2);sol_timer_ball=0;} //desactivate solenoid after soso delay
}


uint8_t self_check(){
  if (!PINB & (1<<PB5)) {DMD.print("Hole 1 Sensor closed");return 1;} // hole1
  if (!PINB & (1<<PB6)) {DMD.print("Hole 2 Sensor closed");return 2;} // hole2
  if (!PINB & (1<<PB7)) {DMD.print("Drain Sensor closed");return 3;} // drain
  if (!PINJ & (1<<PJ1)) {DMD.print("Drop 1 Sensor closed");return 4;} // drop1
  if (!PINJ & (1<<PJ0)) {DMD.print("Drop 2 Sensor closed");return 5;} // drop2
  if (!PINH & (1<<PH1)) {DMD.print("Drop 3 Sensor closed");return 6;} // drop3
  if (!PINH & (1<<PH0)) {DMD.print("Fixed target 1 Sensor closed");return 7;} // ft1
  if (!PIND & (1<<PD3)) {DMD.print("Fixed target 1 Sensor closed");return 8;} // ft2
  if (!PIND & (1<<PD2)) {DMD.print("Bumper 1 Sensor closed");return 9;} // bump1
  if (!PIND & (1<<PD1)) {DMD.print("Bumper 2 Sensor closed");return 10;}//bump2
  if (!PIND & (1<<PD0)) {DMD.print("Slingshot 1 Sensor closed");return 11;}// sling1
  if (!PINA & (1<<PA0)) {DMD.print("Slingshot 2 Sensor closed");return 12;}// sling2
  if (!PINA & (1<<PA1)) {DMD.print("Ramp Sensor closed");return 13;}// ramp
  if (!PINA & (1<<PA2)) {DMD.print("Passage 1 Sensor closed");return 14;}//pass1
  if (!PINA & (1<<PA3)) {DMD.print("Passage 2 Sensor closed");return 15;}//pass2
  if (!PINA & (1<<PA4)) {DMD.print("Leftmost way Sensor closed");return 16;}// reads left loose sensor
  if (!PINA & (1<<PA5)) {DMD.print("Rightmost way Sensor closed");return 17;}// reads right loose sensor
  if (!PINA & (1<<PA6)) {DMD.print("Left way Sensor closed");return 18;}// reads left flip lane
  if (!PINA & (1<<PA7)) {DMD.print("Right way Sensor closed");return 19;}// reads right flip lane
return 0;

}

void reset_variables(){
  ended=0;
  sol_timer_rearm=0;
  sol_timer_ball=0;
  sol_timer_h1=0;
  sol_timer_h2=0;
  sol_timer_s1=0;
  sol_timer_s2=0;
  sol_timer_b1=0;
  sol_timer_b2=0;
  ballonplayfield=0;
  ball=0;
  saveball=0;
  block1=0;
  block2=0;
  lds1=0;
  lds2=0;
  lds3=0;
  lft1=0;
  lft2=0;
  lbmp1=0;
  lbmp2=0;
  lsling1=0;
  lsling2=0;
  lramp=0;
  lpass1=0; 
  pass1_light=0;
  lpass2=0; 
  pass2_light=0;
  lleft=0;
  lright=0;
  llflp=0;
  lrflp=0;
  bonus_mult=1;
  p_score=0;
  score=0;
  drop_counter=0;
  ramp_counter=0;
  bump_counter=0;
  ft1_counter=0;
  ft2_counter=0;
  h1_counter=0;
  h2_counter=0;
  left_loose_counter=0;
  right_loose_counter=0;
  left_flip_counter=0;
  right_flip_counter=0;

}

void fall_drain() {

      adrain=(PINB & (1<<PB7)); // reads PB7 -- drain sensor
      if (ldrain && !adrain && deb[1]>=debmax) {
        deb[1]=0;
      --ballonplayfield;
  
}

ldrain=adrain;
}


void DMDdisp(){

  if (DMD.displayAnimate())
  {
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    DMD.displayReset();
  }


}




void pwm_f1(){
  
   if (!f1_state) ++f1_c_off;
   if (f1_state) ++f1_c_on;
   if (f1_active && !f1_state && f1_c_off>f1_off_per) {f1_state=1;PORTL |= (1 << PIN7);f1_c_off=0;}
   if (f1_state && f1_c_on>f1_on_per) {f1_state=0;PORTL &= ~(1 << PIN7);f1_c_on=0;}
  
}

void pwm_f2(){
  
   if (!f2_state) ++f2_c_off;
   if (f2_state) ++f2_c_on;
   if (f2_active && !f2_state && f2_c_off>f2_off_per) {f2_state=1;PORTL |= (1 << PIN6);f2_c_off=0;}
   if (f2_state && f2_c_on>f2_on_per) {f2_state=0;PORTL &= ~(1 << PIN6);f2_c_on=0;}
  
}

void pwm_s1(){
  
   if (!s1_state) ++s1_c_off;
   if (s1_state) ++s1_c_on;
   if (s1_active && !s1_state && s1_c_off>s1_off_per) {s1_state=1;PORTC |= (1 << PIN0);s1_c_off=0;}
   if (s1_state && s1_c_on>s1_on_per) {s1_state=0;PORTC &= ~(1 << PIN0);s1_c_on=0;}
  
}

void pwm_s2(){
  
   if (!s2_state) ++s2_c_off;
   if (s2_state) ++s2_c_on;
   if (s2_active && !s2_state && s2_c_off>s2_off_per) {s2_state=1;PORTD |= (1 << PIN7);s2_c_off=0;}
   if (s2_state && s2_c_on>s2_on_per) {s2_state=0;PORTD &= ~(1 << PIN7);s2_c_on=0;}
  
}

