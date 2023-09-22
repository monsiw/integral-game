#include "BleKeyboard.h"
BleKeyboard bt("Wiatraczek1", "Espressif", 100);
#define pinHalla 25
#define pinPixeli 
#define millisDebounce 1

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

unsigned long prevMillisPlus;
unsigned long prevMillisMinus;
bool prevStan;
bool playOk;     //warunek odtwarzania
bool playStan;   //aktualny stan odtwarzania
int akumulator;  //zbiornik
int akumulatorMax = 10000;
int akumulatorMinus = 10;
int akumulatorProg = 100;
int millisAkumulatorMinus = 300;
int millisAkumulatorPlus;

/* device credentials */
char* deviceId = "monsiw";         //set your device id (will be the MQTT client username)
char* deviceSecret = "haslo"; //set your device secret (will be the MQTT client password)

/* device topics */
char* outTopic = "devices/<DEVICE-ID>/set"; //where physical updates are published
//char* outTopic = "python/mqtt";
char* inTopic = "devices/<DEVICE-ID>/get";  //where lelylan updates are received

/* Access settings */
byte server[] = {196, 168, 56, 1}; //MQTT server address
char* clientId = "<CLIENT-ID>"; //MQTT client id (random, max 23 bytes) //do wpisania

/* sample payload published to lelylan */
/* (to get the desired property-id go to the device settings and click on the type link) */
//char* payload = "{\"properties\":[{ \"id\": \"<PROPERTY-ID>\", \"value\": \"<VALUE>\" }]}";
int* payload = 0;

/* Ethernet configuration */
byte mac[] = {0xA0, 0xA0, 0xBA, 0xAC, 0xAE, 0x12}; //esp32
EthernetClient ethClient;

/* MQTT communication */
void callback(char* topic, byte* payload, unsigned int length); //subscription callback
PubSubClient client(server, 1883, callback, ethClient);         //mqtt client


void setup(){
	Serial.begin(19200);
	pinMode(pinHalla, INPUT);
	bt.begin();
	delay(500);
	Ethernet.begin(mac);
	Serial.print("Connected with IP: ");
	Serial.println(Ethernet.localIP());

	lelylanConnection(); //MQTT server connection
}

void loop(){
	if((prevMillisPlus+millisDebounce)<millis()){
		if(prevStan!=digitalRead(pinHalla)){
			akumulator++;
			prevStan=!prevStan;   
      //podczas zmiany stanu czujnika halla dodajemy do akumulator czyli zmiennej będącej przybliżeniem średniej prędkości obrotowej 
			millisAkumulatorPlus=millis()-prevMillisPlus;
			prevMillisPlus=millis();
		}
	}
	
	if((prevMillisMinus+millisAkumulatorMinus)<millis()&&(akumulator>akumulatorMinus)){ //czas pomiędzy kolejnymi impulsami
		akumulator=akumulator-akumulatorMinus; //co jakiś czas odejmowanie wartości od akumulatora, tak aby po zatrzymaniu wirnika pausować bajkę
		prevMillisMinus=millis();
	}

	playOk=akumulator>akumulatorProg; //warunek odtwarzania

	if (bt.isConnected()){
		if (playOk!=playStan){
			bt.write(KEY_MEDIA_PLAY_PAUSE);
			playStan=!playStan;
			Serial.println("klik");
			delay(1);
      payload=0;
      lelylanPublish(payload);
		}else{
      payload=1;
      lelylanPublish(payload);
    }
	}

	lelylanConnection();
}

/* MQTT server connection */
void lelylanConnection(){
	// add reconnection logics
	if (!client.connected()){
		// connection to MQTT server
		if (client.connect(clientId, deviceId, deviceSecret)){
			Serial.print("[OK] Connected with MQTT");
			lelylanSubscribe(0);  // topic subscription
			lelylanPublish(0);    // topic publishing
		}
	}
	client.loop();
}

/* MQTT publish */
void lelylanPublish(int* payload){
	client.publish(outTopic, payload);
}

/* MQTT subscribe */
void lelylanSubscribe(int* payload){
	client.subscribe(inTopic);
}

//debug to show the received message
void callback(char* topic, byte* payload, unsigned int length){
	Serial.print("Receiving subscribed message");
	Serial.println(topic);
	Serial.write(payload, length);
}

// const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
// const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
// const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
// const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
// const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
// const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
// const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
// const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
// const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1}; // Opens "My Computer" on Windows
// const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
// const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
// const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
// const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
// const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};