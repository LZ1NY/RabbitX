
#include <SPI.h>
#include <RFM22.h>

char  chars[50][6] = {{2,2,2,2,2,0},{1,2,2,2,2,0}, {1,1,2,2,2,0},{1,1,1,2,2,0},{1,1,1,1,2,0},{1,1,1,1,1,0},{2,1,1,1,1,0},{2,2,1,1,1,0},{2,2,2,2,1,0},{2,2,2,2,1,0},
{0},  //:
{0},  //;
{0},  //<
{2,1,1,1,2,0},  //=
{0},  //>
{0  },  //?
{0},  //@
{1,2,0},       //a
{2,1,1,1,0},   //b
{2,1,2,1,0},   //c
{2,1,1,0},     //d
{1,0},         //e
{1,1,2,1,0},   //f
{2,2,1,0},     //g
{1,1,1,1,0},   //h
{1,1,0},       //i
{1,2,2,2,0},   //j
{2,1,2,0},     //k
{1,2,1,1,0},                //l
{2,2,0},                       //m
{2,1,0},                          //n
{2,2,2,0},                           //o
{1,2,2,1,0},                              //p
{2,2,1,2,0},                                 //q
{1,2,1,0},                           //r
{1,1,1,0},                              //s
{2,0},                                 //t
{1,1,2,0},                         //u
{1,1,1,2,0},                             //v
{1,2,2,0},                                //w
{2,1,1,2,0},                                   //x
{2,1,2,2,0},                                      //y
{2,2,1,1,0}           //z
}      ;

char buffer[10]  ;
char send_buffer[50];
void rtty_txbit (int bit)  ;
int one_tit=60;
boolean send_cw;
char *mode = "C" ;
 int led = 13;
//Setup radio on SPI with NSEL on pin 10
rfm22 radio1(10);
//--------------------------------------------------------------------------------------
void setupRadio(){
    digitalWrite(5, LOW);
    delay(1000);
    rfm22::initSPI();
    radio1.init();
    radio1.write(0x71, 0x00); // unmodulated carrier
    //This sets up the GPIOs to automatically switch the antenna depending on Tx or Rx state, only needs to be done at start up
    radio1.write(0x0b,0x12);
    radio1.write(0x0c,0x15);
    radio1.write(0x6D, 0x04);   // turn tx low power 11db
    //radio1.setFrequency(434.2005);
     }     
//--------------------------------------------------------------------------------------
void setup(void)  {
   Serial.begin(9600);
   setupRadio();
   Serial.print("Init Ends\n");  
  
   }

//--------------------------------------------------------------------------------------
void cw_char(char c)      {
          if ( c == 32 )   {delay(10*one_tit); return;}
          if ( c<48  or c>91)   return;    
          
  uint8_t ind = uint8_t(c) - 0x30;
   
  byte cw_cnt =0 ;
  char *defs;
  Serial.print(uint8_t(c));   Serial.print("-");
  defs = &chars[ind][0];
  send_cw = true;
  while(send_cw){   switch (defs[cw_cnt]){
                    case 0:
                          delay(3*one_tit);
                          send_cw=false;
                          return;   
                    case 1:
                          tit();
                          break;
                    case 2:
                          taa();
                        break;
                     default:
                           {send_cw=false; delay(10*one_tit); return;  }
                            }
                            
                      cw_cnt++;      
    
   }   }   
//--------------------------------------------------------------------------------------
  void rtty_char(char c) 
{ 
	/* Simple function to sent each bit of a char to 
	** rtty_txbit function. 
	** NB The bits are sent Least Significant Bit first
	** All chars should be preceded with a 0 and 
	** proceded with a 1. 0 = Start bit; 1 = Stop bit
	** ASCII_BIT = 7 or 8 for ASCII-7 / ASCII-8     	*/
	int i;
	rtty_txbit (0); // Start bit
	// Send bits for for char LSB first	
	for (i=0;i<8;i++)
	{
		if (c & 1) rtty_txbit(1); 
			else rtty_txbit(0);	
		c = c >> 1;
	}
	rtty_txbit (1); // Stop bit
  rtty_txbit (1); // Stop bit

}
 
void rtty_txbit (int bit)
{  	if (bit)    
                radio1.setFrequency(434.200); 
               // radio1.write(0x73,0x03);
                
		else          
               radio1.setFrequency(434.2005);
                //radio1.write(0x73,0x00); // Low
                
	     delayMicroseconds(19500); // 10000 = 100 BAUD 20150       was 19500
      //delay(200)    ;
  }


//--------------------------------------------------------------------------------------






//--------------------------------------------------------------------------------------
float GetTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
}
//--------------------------------------------------------------------------------------
void txstring (char * string)
{
  if  (mode=="R") {tx_on();    delay(2000);}
	char c;
	c = *string++;
	while ( c != '\0')
	{
    mode="R";
		if  (mode=="C") cw_char(c);
    if  (mode=="R") rtty_char(c)    ;    
		c = *string++;
    
          }    
          }


//--------------------------------------------------------------------------------------
void loop(void){
    //tx_on(); delay(100); tx_off();
    
    float t =   GetTemp();
    dtostrf(t, 5,1, buffer) ;
    Serial.print(buffer);
    Serial.print("\n" );
    //txstring("ABCDEFGHIJK")  ;
    //txstring("LZ1NY=TEMP") ;
    //strcpy(send_buffer, "abcd" )  ;
    //strcpy(*(&send_buffer+2), "1234" )  ;
     sprintf(send_buffer, "$$$$LZ1NY T=%s\r", buffer)    ;
    
    txstring(send_buffer) ;
    //txstring("\n\r") ;
    delay(1000);
   }

//--------------------------------------------------------------------------------------


void tx_on(void) {radio1.write(0x07, 0x08);}
void tx_off(void){radio1.write(0x07, 0x01);}
void tit(void){  tx_on(); delay(one_tit); tx_off();delay(one_tit); }
void taa(void){  tx_on(); delay(3*one_tit); tx_off();delay(one_tit); }

 //-------------------------------------------
 void setup_test(void)  {                        pinMode(led, OUTPUT);    }
 void loop_test(void){
             

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(10);               // wait for a second

 }
