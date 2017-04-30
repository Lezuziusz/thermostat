#define DEBUG_MODE

#ifdef DEBUG_MODE
#define  dp(x)       Serial.print(x)
#define  dp2(x,y)    Serial.print(x,y)
#define  dpln(x)     Serial.println(x)
#define dpln2(x,y)  Serial.println(x,y)
#else
#define dp(x)
#define dpln(x)
#endif
