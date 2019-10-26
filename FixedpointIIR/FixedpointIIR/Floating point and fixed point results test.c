// Both filters work fixed point and floating point
// Tested in Clion
// Inverse value works as well


#include <iostream>
//ADC_voltage:
#define MAX_VALUE_ADC   2047                               // only 11 bits are used
#define VCC         3.30
#define VREF        (((double) VCC) / 1.6)                 // maximal value is 2.06125 V
//DAC
#define MAX_VALUE_DAC 4095
#define CAL_DAC 1.000  // Calibration value DAC




int16_t state_x1, state_x2, state_y1, state_y2;
int16_t state_x2_1, state_x2_2, state_y2_1, state_y2_2;

int32_t y2;
int32_t y;


int32_t a11 = -28277;
int32_t a12 = 12246 ;
int32_t b10 = 88;
int32_t b11 = 176;
int32_t b12 = 88;


int32_t a21 = -30536 ;
int32_t a22 = 14533;
int32_t b20 = 95;
int32_t b21 = 190;
int32_t b22 = 95;

//float a11 = -1.725933395036940880;
//float a12 = 0.747447371907791203 ;
//float b10 =  0.005378494217712604;
//float b11 = 0.010756988435425208;
//float b12 = 0.005378494217712604;
//
//float a21 = -1.863800492075235350;
//float a22 =0.887032999652694887;
//float b20 = 0.005808126894364891;
//float b21 = 0.011616253788729781;
//float b22 = 0.005808126894364891;

int16_t res = 0;

int main() {

    for(int i = 1; i <= 2047; ++i) {
        res++;
        uint16_t  voltage;
        uint16_t BinaryValue;// contains value to write to DAC
		
		
		
        voltage = ((double)res * 1000 * VREF) / (MAX_VALUE_ADC + 1);
        //  printf("%u\n",iVoltage);

        y2 = y2 + ((b10 * voltage) + (b11 * state_x1) + (b12*state_x2) - (a11*state_y1) - (a12*state_y2));
        //printf("%d\n", y2);

        y2 = y2 >> 14;
        state_x2 = state_x1;             /* bump the states over */
        state_x1 = voltage;
        state_y2 = state_y1;
        state_y1 = y2;

        //2nd 2nd order
        y = y + (b20*y2) + (b21*state_x2_1) + (b22*state_x2_2) - (a21*state_y2_1) - (a22*state_y2_2);
        //printf("%d\n", y);

        state_x2_2 = state_x2_1;         /* bump the states over */
        state_x2_1 = y2;
        state_y2_2 = state_y2_1;
        state_y2_1 = y >> 14;

        double dY = y >> 14;
  //      printf("y: %f\n", dY);
        BinaryValue =  dY*((MAX_VALUE_DAC)/(VCC))*0.001*CAL_DAC ;				// Bitvalue
        printf("bin: %u\n", BinaryValue);
		BinaryValue = (~BinaryValue) & 0xFFF;
		printf("bin: %u\n", BinaryValue);

        y2 = y2 & 0x00003FFF;
        y = y & 0x00003FFF;
    }

    return 0;
}




//#include <iostream>
////ADC_voltage:
//#define MAX_VALUE_ADC   2047                               // only 11 bits are used
//#define VCC         3.30
//#define VREF        (((double) VCC) / 1.6)                 // maximal value is 2.06125 V
////DAC
//#define MAX_VALUE_DAC 4095
//#define CAL_DAC 1.000  // Calibration value DAC




//float state_x1, state_x2, state_y1, state_y2;
//float state_x2_1, state_x2_2, state_y2_1, state_y2_2;
//
//float y2;
//float y;
//
//float a11 = -1.725933395036940880;
//float a12 = 0.747447371907791203 ;
//float b10 =  0.005378494217712604;
//float b11 = 0.010756988435425208;
//float b12 = 0.005378494217712604;
//
//float a21 = -1.863800492075235350;
//float a22 =0.887032999652694887;
//float b20 = 0.005808126894364891;
//float b21 = 0.011616253788729781;
//float b22 = 0.005808126894364891;
//
//int16_t res = 0;
//
int main() {

    for(int i = 1; i <= 2047; ++i) {
        res++;
        double  voltage ;
        int16_t BinaryValue;// contains value to write to DAC

        voltage = (double)res * 1000 * VREF / (MAX_VALUE_ADC + 1);
      //  printf("%f\n",voltage);


        y2 = b10 * voltage + b11 * state_x1 + b12*state_x2 - a11*state_y1 - a12*state_y2;
        //printf("%d\n", y2);


        state_x2 = state_x1;             /* bump the states over */
        state_x1 = voltage;
        state_y2 = state_y1;
        state_y1 = y2;


        //2nd 2nd order
        y = b20*y2 + b21*state_x2_1 + b22*state_x2_2 - a21*state_y2_1 - a22*state_y2_2;

        state_x2_2 = state_x2_1;         /* bump the states over */
        state_x2_1 = y2;
        state_y2_2 = state_y2_1;
        state_y2_1 = y;


        BinaryValue =  y*((MAX_VALUE_DAC)/(VCC))*0.001*CAL_DAC ;				// Bitvalue
        printf("%d\n", BinaryValue);

    }

    return 0;
}