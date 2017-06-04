/*
 * requires: https://github.com/cyborg5/IRLib2
 */

#include <IRLibRecvPCI.h> // Recommend only use IRLibRecvPCI or IRLibRecvLoop for best results

#if (RECV_BUF_LENGTH < 140)
  #error "please edit IRLibGlobals.h and set RECV_BUF_LENGTH to more than 140"
#endif


IRrecvPCI myReceiver(2);//pin number for the receiver
uint16_t A[RECV_BUF_LENGTH];
uint16_t B[RECV_BUF_LENGTH];


#define OUT_LEN 64
bool OUT[OUT_LEN];
bool *OUThead = OUT+0; //len 24
bool *OUTtype = OUT+24;//len 16
bool *OUTbody = OUT+40;//len 24

//*NAME*                  *HEAD*                            |    * TYPE*                        |                *BODY*
//START: 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0, | 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,1,1,0
//BRIGH: 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,0,0,1,0
//TIMER: 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 1,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,1,0,1,0,0,0  
//MODE:  1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 1,1,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,1,0,1,0,0
//ANTI:  1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 1,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,1,1,0,0,0
//TURBO: 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,1,1,0,0,0
//FAN:   1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,0,0,0,1,0,0
//AUTO:  1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,0,0,1,0,0,0
//LOCK:  1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 0,0,1,0,1,0,0,0, 0,0,0,0,0,0,0,0, 1,0,1,0,0,1,0,0
//POWER: 1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1, | 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1, | 1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,1,0,0,1,0,0,0

bool HEAD[24]={  1,0,0,0,1,0,0,0, 0,1,0,1,1,0,1,1, 1,1,1,0,1,0,0,1 };

bool TYPE_A[16]={ 0,0,0,0,1,1,1,1, 0,0,0,0,0,0,0,0 };
bool TYPE_B[16]={ 0,0,0,0,0,0,0,0, 1,1,1,1,0,0,0,1 };

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



int get(uint16_t *out)
{
  while(!myReceiver.getResults()){}

  int len = recvGlobal.recvLength;
  for(bufIndex_t i=0;i<recvGlobal.recvLength;i++)    out[i]=recvGlobal.recvBuffer[i];
  
  myReceiver.enableIRIn();      //Restart receiver
  return len;
}

inline bool eq(int16_t a,int16_t b,uint16_t delta){ return (abs(a-b) < delta);}
inline bool isSpace(uint16_t &v) { return eq(v, 350,  50); }//300-400
inline bool isBit0(uint16_t &v)  { return eq(v, 520, 100); }//420-620
inline bool isBit1(uint16_t &v)  { return eq(v,1300, 150); }//1150-1450

bool comapre(bool *a, bool *b, int len)
{
  for(int i=0;i<len;i++)
  {
     if(a[i]!=b[i])  return false;
  }
  return true;
}

bool getOut(uint16_t *buf,int len)
{
    if(len!=132) {
      Serial.println("ERROR: getOut wrong len"); 
      return false;
    }
    
    int ID=0;
    for(bufIndex_t i=3;i<len-1;i+=2)  // WTF?! why -1 (last has wrong value!)
    {
      uint16_t space = buf[i+0];
      uint16_t value = buf[i+1];
     
      if(!isSpace(space)) { 
        Serial.print("ERROR: getOut bit: ");
        Serial.print(i,DEC); 
        Serial.print(" is not a space");
        Serial.println();
        return false;        
      }

      if(isBit0(value))   { OUT[ID++]=false; continue;}
      if(isBit1(value))   { OUT[ID++]=true;  continue;}

      Serial.print("ERROR: getOut wrong Bit");
      Serial.print(i,DEC);
      Serial.print(" value="); 
      Serial.println(value,DEC);
      
      return false;
    }

    if(!comapre(OUThead,HEAD,24))
    {
      Serial.println("ERROR: getOut: wrong head!");
      return false;
    }

    return true;
}

void setup() {
  Serial.begin(9600);  
  myReceiver.enableIRIn(); // Start the receiver
}

void loop() {

  Serial.println();

  Serial.println("INFO: wait for A...");
  int la= get(A);
  long aEndTime = millis();
  Serial.println("INFO: wait for B...");
  int lb= get(B);

  long AandBtimeDelta = (millis()-aEndTime);
  if(AandBtimeDelta>90 && AandBtimeDelta<120)
  {
    Serial.print("INFO: done time between A&B: " );
    Serial.print(AandBtimeDelta,DEC);
    Serial.print("mili sec");
    Serial.println();
  }else{
    Serial.print("ERROR: time between A&B: " );
    Serial.print(AandBtimeDelta,DEC);
    Serial.print(" should be ~100 mili sec");
    Serial.println();
  }

  if(la==lb)
  {
    //CHECK A
    if(!getOut(A,la))                   { Serial.println("ERROR: A getOut failed!");  return;}
    if(!comapre(OUTtype,TYPE_A,16))     { Serial.println("ERROR: wrong A type!"); return;}
    if(!comapre(OUTbody,BODY_TYPEA,24)) { Serial.println("ERROR: wrong A body!"); return;}
    //============================================================================================

    //CHECK B
    if(!getOut(B,lb))                   { Serial.println("ERROR: B getOut failed!");  return;}
    if(!comapre(OUTtype,TYPE_B,16))     { Serial.println("ERROR: wrong B type!"); return;}
    //============================================================================================
    
    Serial.print("MESSAGE:");
    for(int i=24+16;i<OUT_LEN;i++)
    {
       if(OUT[i]) Serial.print("1,");
       else       Serial.print("0,");
    }
    Serial.print(" ");

    if(comapre(OUTbody,BODY_BRIGH,24)) Serial.print("BRIGHT");
    else
    if(comapre(OUTbody,BODY_TIMER,24)) Serial.print("TIMER");
    else
    if(comapre(OUTbody,BODY_MODE,24))  Serial.print("MODE");
    else
    if(comapre(OUTbody,BODY_ANTI,24))  Serial.print("ANTI");
    else
    if(comapre(OUTbody,BODY_TURBO,24)) Serial.print("TURBO");
    else
    if(comapre(OUTbody,BODY_FAN,24))   Serial.print("FAN");
    else
    if(comapre(OUTbody,BODY_AUTO,24))  Serial.print("AUTO");
    else
    if(comapre(OUTbody,BODY_LOCK,24))  Serial.print("LOCK");
    else
    if(comapre(OUTbody,BODY_ONOFF,24)) Serial.print("ONOFF");
    else                               Serial.print("?????");
    //==============================================
    
    Serial.println();
  }else{
    Serial.println("ERROR: len(A)!=len(B) !");
  }
          
  
}

