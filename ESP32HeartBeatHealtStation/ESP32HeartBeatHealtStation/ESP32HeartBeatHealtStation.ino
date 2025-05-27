/*
 Name:		ESP32HeartBeatHealthStation.ino
 Created:	22.05.2025 08:59:30
 Author:	admballuffvm
*/

/*
All needed libraries of the demo dev kit
*/
#include "Wire.h"
#include <heartRate.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <FT6336U.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>

/*
* project specific error codes
*/
#define ERR_IS_OK				0x00

/*
* project specific defines, e. g. port mapping
*/
#define LED_PORT_STATE			2

#define IIC_SCL					1
#define IIC_SDA					2

#define HEART_SENS_RATE_SIZE	4
#define HEART_SENS_MAX_VALUE	0xFF
#define HEART_SENS_MIN_VALUE	0x14

/*
* project specific global variables
*/
uint8_t sampleSpot = 0;
int32_t lastBeat = 0;
int32_t beatAvg = 0;
float beatsPerMinute = 0.0;

uint8_t samples[HEART_SENS_RATE_SIZE] = { 0x00, 0x00, 0x00, 0x00 };

/*
* project specific objects
*/
MAX30105 particleSensor;

/*
the setup function runs once when you press reset or power the board
*/

void setup()
{
	uint8_t errcode = ERR_IS_OK;

	Serial.begin(115200);
	Serial.println("|--- initializing components ---|");
	pinMode(LED_PORT_STATE, OUTPUT);
	Serial.println("init led_port_state ---> ok");

	Wire.begin(IIC_SDA, IIC_SCL);
	Serial.println("init wire iic_sdc iic_scl ---> ok");

	if (particleSensor.begin(Wire, I2C_SPEED_FAST) != ERR_IS_OK)
	{
		Serial.println("init particlesensor...");
		while (1);
	}
	particleSensor.setup();
	particleSensor.setPulseAmplitudeRed(0x0a);
	particleSensor.setPulseAmplitudeGreen(0);
	Serial.println("init particlesensor ---> ok");
}

void loop()
{
	int32_t irValue = particleSensor.getIR();
	if (checkForBeat(irValue) == true)
	{
		int32_t delta = millis() - lastBeat;
		lastBeat = millis();
		beatsPerMinute = (60 / (delta / 1000.0));
		if ((beatsPerMinute < HEART_SENS_MAX_VALUE) && (beatsPerMinute > HEART_SENS_MIN_VALUE))
		{
			samples[sampleSpot++] = (byte)beatsPerMinute;
			sampleSpot %= HEART_SENS_RATE_SIZE;
			beatAvg = 0;
			for (byte x = 0; x < HEART_SENS_RATE_SIZE; x++)
			{
				beatAvg += samples[x];
			}
			beatAvg /= HEART_SENS_RATE_SIZE;
		}
	}
	Serial.print("IR=");
	Serial.print(irValue);
	Serial.print(", BPM=");
	Serial.print(beatsPerMinute);
	Serial.print(", Avg BPM=");
	Serial.print(beatAvg);

	if (irValue < 50000)
	{
		Serial.print(" No finger?");
	}
	Serial.println();
}