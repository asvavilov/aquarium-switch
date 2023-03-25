#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "secure.h"

#define PIN_SW1 0
#define PIN_SW2 2

#define CMD_SET 1
#define CMD_GET 2

#define STATE_OFF 1
#define STATE_ON 2
#define STATE_DAY 3
#define STATE_EVENING 4
#define STATE_NIGHT 5

int numState2 = 0;
int onStates2 [] = { STATE_DAY, STATE_EVENING, STATE_NIGHT };

#define PORT 80
WiFiServer server(PORT);

void connectNet(uint32_t recon_delay = 1000)
{
	Serial.printf("Connecting to %s \n", SSID_NAME);
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID_NAME, SSID_PASS);
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(recon_delay);
	}
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID_NAME);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	server.begin();
	Serial.print("Open Telnet and connect to IP:");
	Serial.print(WiFi.localIP());
	Serial.print(" on port ");
	Serial.println(PORT);
}

void goState2(int state2)
{
	if (numState2 == 0)
	{
		digitalWrite(PIN_SW2, HIGH);
		numState2 = 1;
	}
	int i = numState2 - 1;
	while (state2 != onStates2[i])
	{
		delay(500);
		digitalWrite(PIN_SW2, LOW);
		delay(500);
		digitalWrite(PIN_SW2, HIGH);
		i++;
		if (i > 2)
		{
			i = 0;
		}
		numState2 = i + 1;
	}
}

void setup()
{
	Serial.begin(115200);

	pinMode(PIN_SW1, OUTPUT);
	digitalWrite(PIN_SW1, LOW);
	pinMode(PIN_SW2, OUTPUT);
	digitalWrite(PIN_SW2, LOW);

	connectNet();
}

void loop()
{
	WiFiClient client = server.accept();
	// echo -e "hello\n123" | nc -q 1 192.168.1.55 80
	// echo -e "\x1\x0\x5" | nc -q 1 192.168.1.55 80
	// echo -e "\x2" | nc -q 5 192.168.1.55 80
	if (client)
	{
		while (client.connected())
		{
			String ret = "";

			int cmd, state1, state2;

			while (client.available()>0)
			{
				cmd = client.read();

				if (cmd == CMD_SET)
				{
					state1 = client.read();
					state2 = client.read();

					if (state1 == STATE_OFF)
					{
						digitalWrite(PIN_SW1, LOW);
					}
					else if (state1 == STATE_ON)
					{
						digitalWrite(PIN_SW1, HIGH);
					}

					if (state2 == STATE_OFF)
					{
						digitalWrite(PIN_SW2, LOW);
						numState2 = 0;
					}
					else if (state2 == STATE_DAY || state2 == STATE_EVENING || state2 == STATE_NIGHT)
					{
						goState2(state2);
					}
					ret = String(state1) + "/" + String(state2);
				}
				else if (cmd == CMD_GET)
				{
					int sw1 = digitalRead(PIN_SW1);
					int sw2 = digitalRead(PIN_SW2);

					state1 = 0;
					state2 = 0;

					if (sw1 == LOW)
					{
						state1 = STATE_OFF;
					}
					else if (sw1 == HIGH)
					{
						state1 = STATE_ON;
					}

					if (sw2 == LOW)
					{
						state2 = STATE_OFF;
					}
					else if (sw2 == HIGH)
					{
						state2 = onStates2[numState2 - 1];
					}

					ret = String(state1) + "/" + String(state2);
				}
			}
			if (ret != "")
			{
				client.print(ret + "\n");
				Serial.println(ret);
			}
		}
		client.stop();
	}
}
