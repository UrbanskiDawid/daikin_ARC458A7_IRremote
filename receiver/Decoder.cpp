#include <Arduino.h>



class Decoder
{
  //============================================================================+
  bool HEAD[24]       ={ 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1 };
  bool TYPE_A[16]     ={ 0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0 };
  bool TYPE_B[16]     ={ 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1 };
  
  bool BODY_TYPEA[24] ={ 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,1,1,0 };
  bool BODY_BRIGH[24] ={ 0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,0,0,1,0 };
  bool BODY_TIMER[24] ={ 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,1,0,0,0 };
  bool BODY_MODE [24] ={ 1,1,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,1,0,1,0,0 };
  bool BODY_ANTI [24] ={ 1,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,1,1,0,0,0 };
  bool BODY_TURBO[24] ={ 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,1,0,0,0 };
  bool BODY_FAN  [24] ={ 0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,1,0,0 };
  bool BODY_AUTO [24] ={ 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,0,0,1,0,0,0 };
  bool BODY_LOCK [24] ={ 0,0,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,1,0,0,1,0,0 };
  bool BODY_ONOFF[24] ={ 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,0,0,0 };
  //============================================================================+


  bool isOK;
  unsigned long *buffer;

  //this converst part1 or part2 into bool array
  //msg #64 of (mark, space)
  //notes:
  // space is ~1600 long
  // mark0  is ~400  long (encoded as 0)
  // mark1  is ~1300 long (encoded as 1)
  // first mark has value of ~1700~
  bool convertMEMORY(unsigned long *msg,bool *OUT)
  {
    unsigned outI=0;
    bool ret=true;      

    for(unsigned i=0; i<64*2;i+=2)
    {
      unsigned long space = msg[i+0];
      if(space<300 || space>510)     { ret=false; }
      
      unsigned long mark = msg[i+1];
      if(mark >  350 && mark <  600) { OUT[outI]=false; }
      else
      if(mark > 1200 && mark < 1500) { OUT[outI]=true;  }     
      else                           { ret=false; }
  
      outI++;
    }
    return ret;
  }
  
  bool compare(bool *A,bool *B, unsigned startA,unsigned len)
  {
     unsigned Bi=0;
     for(unsigned i=startA; i<startA+len; i++,Bi++)
     {
        if(A[i]!=B[Bi]) return false;        
     }
     return true;
  }

public:
  enum eButton{
    unknown,
    BRIGH,
    TIMER,
    MODE ,
    ANTI ,
    TURBO,
    FAN  ,
    AUTO ,
    LOCK ,
    ONOFF
  }button;

  Decoder(unsigned long *buf,unsigned len)
  {
      buffer=buf;

      isOK=true;

      //check len
      if( len!=264 ) {Serial.print("error len"); Serial.print(len); Serial.println(" expected 264"); isOK=false; return;}

      //MESSAGE_PART1
      //check fist  (here is start of 1 msg)
      if( (buffer[0]  <3300 && buffer[0]  >3600 )
          ||
          (buffer[1]  <1600 && buffer[1]  >1900)) {Serial.print("error fist"); isOK=false; return;}

      //MESSAGE_SILENCE
      //check delay between msg1 & msg2
      if( buffer[131]<29800 || buffer[131]>30100) {Serial.print("error delay"); isOK=false; return;}

      //MESSAGE_PART2
      //check second (here is start of 2 msg)
      if( (buffer[132]<3300 && buffer[132]>3600)
           || 
          (buffer[133]<1600 && buffer[133]>1900)) {Serial.print("error second"); isOK=false; return;}

      unsigned long *part1=&buffer[2];
      unsigned long *part2=&buffer[134];
      
      bool MSG[64];

      convertMEMORY(part1,MSG);
      if(!compare(MSG,HEAD,          0,24)) {Serial.print(" error: part1-head"); isOK=false;}
      if(!compare(MSG,TYPE_A,       24,16)) {Serial.print(" error: part1-type"); isOK=false;}
      if(!compare(MSG,BODY_TYPEA,24+16,24)) {Serial.print(" error: part1-body"); isOK=false;}
      
      convertMEMORY(part2,MSG);
      if(!compare(MSG,HEAD,          0,24)) {Serial.print(" error: part1-head"); isOK=false;}
      if(!compare(MSG,TYPE_B,       24,16)) {Serial.print(" error: part1-type"); isOK=false;}

      if(compare(MSG,BODY_BRIGH,    40,24)) {button=BRIGH;}
      else
      if(compare(MSG,BODY_TIMER,    40,24)) {button=TIMER;}
      else
      if(compare(MSG,BODY_MODE,     40,24)) {button=MODE;}
      else
      if(compare(MSG,BODY_ANTI,     40,24)) {button=ANTI;}
      else
      if(compare(MSG,BODY_TURBO,    40,24)) {button=TURBO;}
      else
      if(compare(MSG,BODY_FAN,      40,24)) {button=FAN;}
      else
      if(compare(MSG,BODY_AUTO,     40,24)) {button=AUTO;}
      else
      if(compare(MSG,BODY_LOCK,     40,24)) {button=LOCK;}
      else
      if(compare(MSG,BODY_ONOFF,    40,24)) {button=ONOFF;}
      else                                  {button=unknown; 
                                            Serial.print(" error: part2 body");isOK=false;}

      if(!isOK) {Serial.println();}
  }
  bool OK() {return isOK;}
  
  void printButton()
  {
    switch(button)//TODO: array of string
    {
      case BRIGH: Serial.println("BRIGHT"); break;
      case TIMER: Serial.println("TIMER");  break;
      case MODE:  Serial.println("MODE");   break;
      case ANTI:  Serial.println("ANTI");   break;
      case TURBO: Serial.println("TURBO");  break;
      case FAN:   Serial.println("FAN");    break;
      case AUTO:  Serial.println("AUTO");   break;
      case LOCK:  Serial.println("LOCK");   break;
      case ONOFF: Serial.println("ONOFF");  break;
      default:    Serial.println("?????");  break;
    }
  }

  void printRaw()
  {
    Serial.print("head: "); Serial.print(buffer[0]); 
    Serial.print(" timeout: "); Serial.print(buffer[264-1]);
    Serial.print(" (space "); Serial.print(buffer[131]); Serial.print(","); Serial.print(buffer[132]); Serial.print(")");
    Serial.println();
    unsigned long *part1=&buffer[1];
    unsigned long *part2=&buffer[133];

    bool MSG[64];
    convertMEMORY(part1,MSG);    Serial.print("|"); for(int i=0;i<64;i++) { if(i==24 || i==24+16) {Serial.print("|");} Serial.print(MSG[i]);}Serial.println("| ");  
    convertMEMORY(part2,MSG);    Serial.print("|"); for(int i=0;i<64;i++) { if(i==24 || i==24+16) {Serial.print("|");} Serial.print(MSG[i]);}Serial.println("| ");
  }

  void printBuffer()
  {
    __printBuffer(0);
    Serial.print("[131] "); Serial.print(buffer[131]);  Serial.println("(SPACE)");
    __printBuffer(132);
  }
  
  void __printBuffer(unsigned i)
  {
      Serial.print("[");  Serial.print(i);   Serial.print("] ");
      Serial.print(buffer[i+0]); 
      Serial.print(" ");
      Serial.print(buffer[i+1]);
      Serial.print("\t\t");      
      if(buffer[i+0] >  3300 && buffer[i+0] <  3800) { Serial.print("space"); }
      else                                           { Serial.print("spaceError"); }
      Serial.print("\t");
      if(buffer[i+1] >  1600 && buffer[i+1] <  1800) { Serial.print("markS"); }
      else                                           { Serial.print("markError"); }
      Serial.println();

       for(unsigned j=2;j<=64*2+2;j+=2)
       {
          unsigned long *space = buffer[j+i+0];
          unsigned long *mark = buffer[j+i+1];
          
          
          Serial.print("[");  Serial.print(j+i);   Serial.print("] ");
          Serial.print((int)space);
          Serial.print(" ");
          Serial.print((int)mark);
          Serial.print("\t\t");          

          
          if(space>300 && space<510) {Serial.print("space"); }
          else                       {Serial.print("spaceError");}
          
          Serial.print("\t");
          
          if(mark >  350 && mark <  600) { Serial.print("mark0"); }
          else
          if(mark > 1200 && mark < 1500) { Serial.print("mark1");  }
          else                           { Serial.print("markError");  }

          Serial.println();
       }
  }
};
