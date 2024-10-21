// generic colors
#define blackColor RGBToWord(0x00, 0x00, 0x00)
#define whiteColor RGBToWord(0xff, 0xff, 0xff)
#define darkGrey RGBToWord(0xaa, 0xaa, 0xaa) 
#define lightBlueColor RGBToWord(0x00, 0xff, 0xff) 
#define purpleColor RGBToWord(0xff, 0x00, 0xff)    

// theme colors
#define main1Color lightBlueColor
#define main2Color purpleColor
#define disableColor darkGrey 

//welcome image colors
#define welcomeUpperColor main1Color
#define welcomeLowerColor main2Color

// I/O colors
#define potUpperRightColor RGBToWord(0xff, 0x33, 0x99)  // Pink
#define potLowerLeftColor RGBToWord(0xff, 0xcc, 0x33)   // Yellow
#define potLowerRightColor RGBToWord(0x00, 0x99, 0xff)  // Navy Blue
#define in1Color RGBToWord(0x00, 0xff, 0x00) // Green
#define in2Color RGBToWord(0x00, 0xff, 0x00) // Green
#define out1Color whiteColor
#define out2Color whiteColor

// scopes Colors
#define chan0Color main1Color
#define chan1Color main2Color

// ADSR Colors (match Pot colors)
#define aColor potUpperRightColor
#define dColor potLowerLeftColor
#define sColor disableColor
#define rColor potLowerRightColor

// modules main colors
#define calColor main2Color //
#define SnHColor RGBToWord(0x00, 0xff, 0xff)   //
