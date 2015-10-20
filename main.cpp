#include "NOKIA_5110.h"
#define MAX_EFFECTS 5
#define MAX_MODIFICATION 50

// Current active effect
int effect_state = 0;

int effect_parameter_count = 4;
int active_parameter_select = 0;

extern float modified;
float in_sample = 0;

#include "mbed.h"
#include "effects.h"
#include <math.h>

//AnalogIn analog_in(PA_0);
AnalogOut analog_out(PA_4);
//Ticker timer;

// Interfacing User buttons to control effects pedal:

DigitalOut ledPin( PA_5 );         //Led used for testing purpose on Nucleo board LED2
DigitalIn buttonUp(D9, PullDown);  // Change active parameter
DigitalIn buttonDown(D4, PullDown);  // Change active parameter
DigitalIn buttonLeft(D3, PullDown);   // Next effect
DigitalIn buttonRight(D2, PullDown);  // Previous effect
DigitalIn buttonPlus(D5, PullDown);   // Button that will be used to save
DigitalIn buttonMinus(D8, PullDown);  // Button to clear effets

NokiaLcd *nokiaLCD;

Serial pc(USBTX, USBRX);

void button_control();
void update_lcd();
void callback();
void send(char c, int numb);
 
uint32_t samps[2] = {0,0};
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

static void MX_ADC1_Init(void);

static float adc_sample = 0;
static float modified_sample = 0;
static float effect_sample =0;

char buffer[20];

int main() {

	//setup
	MX_ADC1_Init();
    HAL_ADC_Start(&hadc1);

    init_effects();

    pc.baud(115200);
    pc.attach(&callback);

    LcdPins myPins;
	myPins.sce  = D15;
	myPins.rst  = D14;
	myPins.dc   = D10;
	myPins.mosi = D11;
	myPins.miso = NC;
	myPins.sclk = D13;

	wait(0.5);

	nokiaLCD = new NokiaLcd( myPins );
	nokiaLCD->InitLcd();

	nokiaLCD->ClearLcdMem();

	update_lcd();

    while(1)
    {
    	button_control();

    	adc_sample = HAL_ADC_GetValue(&hadc1) / 4096.0f ;
    	adc_sample += 0.0126953f;
    	if(adc_sample< 0.501 && adc_sample >  0.499)
    			adc_sample = 0.5;

    	// List of Effects
    	if(effect_state == 0)
    		modified_sample = clean(adc_sample);
    	else if (effect_state == 1)
    		modified_sample = tremolo(adc_sample);
    	else if (effect_state == 2)
    		modified_sample = tremolo2(adc_sample);
    	else if (effect_state == 3)
    		modified_sample = ez_distort(adc_sample);
    	else if (effect_state == 4)
    		modified_sample = ez_distort2(adc_sample);
    	else if (effect_state == 5)
    		modified_sample = flanger(adc_sample);
    	else if (effect_state == 6)
    		modified_sample = delay(adc_sample);

    	modified_sample -= 0.0126953f;
    	analog_out = modified_sample;

    }
}

void update_lcd()
{
    nokiaLCD->SetXY(0, 0);
    nokiaLCD->ClearLcdMem();

    char buf[MAX_MOD_COUNT][10];
    int i = 0;
    for(;i<MAX_MOD_COUNT; i++)
    {
    	itoa (mod[i],buf[i],10);
    }

    if(effect_state == 0)
    {
        nokiaLCD->DrawString("Clean");
        nokiaLCD->SetXY(0,1);
        nokiaLCD->DrawString("---------");

        nokiaLCD->SetXY(0,2);
        nokiaLCD->DrawString(" Volume: ");
        nokiaLCD->DrawString(buf[0]);
    }
    if (effect_state == 1)
    {
         nokiaLCD->DrawString("Tremolo 1");
         nokiaLCD->SetXY(0,1);
         nokiaLCD->DrawString("---------");

         nokiaLCD->SetXY(0,2);
         nokiaLCD->DrawString(" Fs: ");
         nokiaLCD->DrawString(buf[0]);
    }
    if (effect_state == 2)
    {
         nokiaLCD->DrawString("Tremolo 2");
         nokiaLCD->SetXY(0,1);
         nokiaLCD->DrawString("---------");

         nokiaLCD->SetXY(0,2);
         nokiaLCD->DrawString(" Fs: ");
         nokiaLCD->DrawString(buf[0]);
    }
    if(effect_state == 3)
    {
    	nokiaLCD->DrawString("Overdrive");
    	nokiaLCD->SetXY(0,1);
    	nokiaLCD->DrawString("---------");

	    nokiaLCD->SetXY(0,2);
	    nokiaLCD->DrawString(" Threshold: ");
	    nokiaLCD->DrawString(buf[0]);
    }
    if (effect_state == 4)
    {
         nokiaLCD->DrawString("Distortion");
         nokiaLCD->SetXY(0,1);
         nokiaLCD->DrawString("---------");

         nokiaLCD->SetXY(0,2); // Set active line
         nokiaLCD->DrawString(" Threshold: ");
         nokiaLCD->DrawString(buf[0]); // Show mod value
         nokiaLCD->SetXY(0,3);
         nokiaLCD->DrawString(" Gain: ");
         nokiaLCD->DrawString(buf[1]); // Show mod value

    }
    if(effect_state == 5)
    {
    	nokiaLCD->DrawString("Flanger");
    	nokiaLCD->SetXY(0,1);
    	 nokiaLCD->DrawString("---------");
  	    nokiaLCD->SetXY(0,2);
  	    nokiaLCD->DrawString(" Eto: ");
  	    nokiaLCD->DrawString(buf[0]);
    }
    // active_parameter_select indicates selected mod
    nokiaLCD->SetXY(0,active_parameter_select + 2);
    nokiaLCD->DrawString(">");
}

void send_effect_state()
{
	send('e', effect_state);
}

void send_mods_state()
{
	send('m', mod[0]);
	wait_us(1);
	send('q', mod[1]);
	wait_us(1);
	send('w', mod[2]);
	wait_us(1);
	send('e', mod[3]);
	wait_us(1);
	send('r', mod[4]);
}



void button_control()
{
     if (buttonRight == 1) {
         wait(0.002);
        if (buttonRight == 1) {
            while (buttonRight == 1); //wait until button is released

             ledPin = 1;
          //  LATBbits.LATB7 = ~LATBbits.LATB7;
            //effect_state += 1;
             set_modifications_default();

        	effect_state = (effect_state+1) % (MAX_EFFECTS);
            update_lcd();
            send_effect_state();
        }
    }
    if (buttonLeft == 1) {
        wait(0.02);
        if (buttonLeft == 1) {
            while (buttonLeft == 1); //wait until button is released

            ledPin = 0;

            effect_state -= 1;
            set_modifications_default();

            if(effect_state < 0)
            	effect_state = MAX_EFFECTS - 1;
            update_lcd();
            send_effect_state();
        }
    }

    if (buttonUp == 1) {
        wait(0.002);
        if (buttonUp == 1) {
            while (buttonUp == 1);

            active_parameter_select--;
            if(active_parameter_select<0)
            	active_parameter_select = effect_parameter_count-1;

            update_lcd();
            send_effect_state();

        }
    }


    if (buttonDown == 1) {
            wait(0.002);
            if (buttonDown == 1) {
                while (buttonDown == 1);

                active_parameter_select = (active_parameter_select + 1) % effect_parameter_count;

                update_lcd();
                send_effect_state();
            }
        }



    if (buttonPlus == 1) {
        wait(0.002);
        if (buttonPlus == 1) {
            while (buttonPlus == 1); //wait until button is released

            ledPin = 1;
            mod[active_parameter_select] += 1;

            if(mod[active_parameter_select] > MAX_MODIFICATION)
            	mod[active_parameter_select] = MAX_MODIFICATION;
            update_lcd();
            send_mods_state();
        }
    }

    if (buttonMinus == 1) {
        wait(0.002);
        if (buttonMinus == 1) {
            while (buttonMinus == 1); //wait until button is released

            ledPin = 0;
            mod[active_parameter_select] -= 1;

            if(mod[active_parameter_select] < 0)
            	mod[active_parameter_select] = 0;

            update_lcd();
            send_mods_state();
        }
    }
}

void MX_ADC1_Init(void)
{

	//DMA
	  /* DMA controller clock enable */
	  __DMA1_CLK_ENABLE();

	  /* DMA interrupt init */
	  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);




	 GPIO_InitTypeDef GPIO_InitStruct;

	__ADC12_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	hdma_adc1.Instance = DMA1_Channel1;
	hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc1.Init.MemInc = DMA_MINC_DISABLE;
	hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_adc1.Init.Mode = DMA_CIRCULAR;
	hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;

	HAL_DMA_Init(&hdma_adc1);

	__HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);



    /**Common config
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION12b;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = OVR_DATA_OVERWRITTEN;

  HAL_ADC_Init(&hadc1);




  ADC_ChannelConfTypeDef sConfig = {0};

  // Configure ADC channel
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;

  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset       = 0;
  sConfig.Channel = ADC_CHANNEL_1;

  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}

void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

void send(char c, int numb)
{
	pc.printf("%c%d\n", c, numb);
}

void callback()
{
	for(int i = 0; i < 5; i++)
		buffer[i] = 0;

    pc.gets(buffer, 10);

    char numbers[2];
    numbers[0] = buffer[1];
    numbers[1] = buffer[2];

    int number = 0;
    sscanf(numbers, "%d", &number);

    if(buffer[0] == 'e')
    {
    	effect_state = number;
    }

    if(buffer[0] == 'm')
    	mod[0] = number;
    if(buffer[0] == 'q')
    	mod[1] = number;
    if(buffer[0] == 'w')
    	mod[2] = number;
    if(buffer[0] == 'e')
    	mod[3] = number;
    if(buffer[0] == 'r')
    	mod[4] = number;

    if(buffer[0] == 'p')
    {
    	send_mods_state();
    	send_effect_state();
    }

	update_lcd();

}

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}
