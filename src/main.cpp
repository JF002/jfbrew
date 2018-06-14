#include <M5Stack.h>

#include <WiFi.h>
#include <NTPClient.h>
#include <MQTT.h>

#include <Screen.h>
#include <ButtonInfoBar.h>
#include <StatusBar.h>
#include <Button.h>
#include <WidgetMosaic.h>
#include <AppScreen.h>
#include <UpDownButton.h>

//#include <MiniPID.h>

#include "Configuration.h"
#include "Application.h"

#include <string>
#include <time.h>
#include <sys/time.h>

using namespace Codingfield::UI;
using namespace Codingfield::Brew;

AppScreen* screen;
StatusBar* topBar;
ButtonInfoBar* bottomBar;
Codingfield::UI::Button* button0;
Codingfield::UI::Button* button1;
Codingfield::UI::Button* button2;
Codingfield::UI::UpDownButton* button3;
Codingfield::UI::UpDownButton* button4;
Codingfield::UI::UpDownButton* button5;
WidgetMosaic* mosaic;
Widget* focus;

Codingfield::UI::Button* buttonSetting;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, 7200); // Time offset = 2h
bool ntpSync = false;

WiFiClient net;
MQTTClient mqtt;

Configuration config;
Application* app;

Actuators::Relays::States buttonState = Actuators::Relays::States::Closed;

uint32_t loopCount = 0;

int32_t uptimeHours=0;
bool longPush = false;

void ConnectWifi();
void mqttMessageReceived(String &topic, String &payload);
void InitUI();

void setup() {
  const bool initLCD = true;
  const bool initSD = false;
  M5.begin(initLCD, initSD);
  M5.Speaker.mute();

  app = new Application(config);

  Serial.println("Init HW... ");
  app->Init();
  Serial.println("OK!");

  Serial.print("Init UI... ");
  InitUI();
  Serial.println("OK!");

  Serial.print("Connecting to Wifi... ");
  ConnectWifi();
  timeClient.update();
  Serial.println("OK!");
}

void loop() {
  M5.update();
  mqtt.loop();
  app->Update();

  if(!ntpSync) {
    if(timeClient.update()) {
      ntpSync = true;
    }
  }

  if((loopCount % 50) == 0) {
    String s;

    auto fridgeValue = app->FridgeTemperature();
    s = String(fridgeValue) + String("C");
    button0->SetText(s.c_str());
    Serial.println("TEMPERATURE0 : " + s);
    mqtt.publish("/jfbrew/temperature/fridge", String(fridgeValue));

    auto beerValue = app->BeerTemperature();
    s = String(beerValue) + String("C");
    button1->SetText(s.c_str());
    Serial.println("TEMPERATURE1 : " + s);
    mqtt.publish("/jfbrew/temperature/beer", String(beerValue));

    auto roomValue = app->RoomTemperature();
    s = String(roomValue) + String("C");
    button2->SetText(s.c_str());
    Serial.println("TEMPERATURE2 : " + s);
    mqtt.publish("/jfbrew/temperature/room", String(roomValue));

    if(app->HeaterState() == Actuators::Relays::States::Open)
      mqtt.publish("/jfbrew/relay/heater/state", "0");
    else
      mqtt.publish("/jfbrew/relay/heater/state", "1");

    if(!button3->AreControlsEnabled()) {
      s = Actuators::Relays::ToString(app->HeaterState()).c_str();
      button3->SetText(s.c_str());
    }

    if(app->CoolerState() == Actuators::Relays::States::Open)
      mqtt.publish("/jfbrew/relay/cooler/state", "0");
    else
      mqtt.publish("/jfbrew/relay/cooler/state", "1");

    if(!button4->AreControlsEnabled()) {
      s = Actuators::Relays::ToString(app->CoolerState()).c_str();
      button4->SetText(s.c_str());
    }

    if(app->FanState() == Actuators::Relays::States::Open)
      mqtt.publish("/jfbrew/relay/fan/state", "0");
    else
      mqtt.publish("/jfbrew/relay/fan/state", "1");

    if(!button5->AreControlsEnabled()) {
      s = Actuators::Relays::ToString(app->FanState()).c_str();
      button5->SetText(s.c_str());
    }

    uptimeHours = millis() / (60*60000);
    topBar->SetUptime(uptimeHours);


    char strftime_buf[64];
    snprintf(strftime_buf, 64, "%02d:%02d:%02d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
    topBar->SetDateTime(strftime_buf);
  }

  if((loopCount % 100) == 0) {
    auto rssi =WiFi.RSSI();
    if(rssi >= -55) {
      topBar->SetWifiStatus(StatusBar::WifiStatuses::Full);
    } else if(rssi >= -75) {
      topBar->SetWifiStatus(StatusBar::WifiStatuses::Medium);
    } else if(rssi >= -85) {
      topBar->SetWifiStatus(StatusBar::WifiStatuses::Weak);
    } else {
      topBar->SetWifiStatus(StatusBar::WifiStatuses::No_signal);
    }
  }

  if(M5.BtnA.wasPressed()) {
    focus->OnButtonAPressed();
  }

  if(M5.BtnB.pressedFor(1000)) {
    if(!longPush) {
      focus->OnButtonBLongPush();
      longPush = true;
    }
  }
  else if(M5.BtnB.wasReleased()) {
    if(!longPush) {
      focus->OnButtonBPressed();
    }
    else {
      longPush = false;
    }
  }

  if(M5.BtnC.wasPressed()) {
    focus->OnButtonCPressed();
  }
  screen->Draw();

  loopCount++;
  delay(10);
}

void ConnectWifi() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();

  WiFi.begin("Zataboy-2.4Ghz", "WWJNRZQV");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  Serial.println("Connecting to MQTT");
  mqtt.begin("192.168.1.109", net);
  while (!mqtt.connect("ESP32", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }
  mqtt.subscribe("/jfbrew/relay/cooler/command");
  mqtt.subscribe("/jfbrew/relay/heater/command");
  mqtt.subscribe("/jfbrew/relay/fan/command");

  if(config.IsTemperatureSensorStubbed()) {
    mqtt.subscribe("/jfbrew/temperature/fridge/stub");
    mqtt.subscribe("/jfbrew/temperature/beer/stub");
    mqtt.subscribe("/jfbrew/temperature/room/stub");
  }

  mqtt.onMessage(mqttMessageReceived);
  Serial.println("OK");

}

void InitUI() {
  topBar = new StatusBar();
  bottomBar = new ButtonInfoBar();
  mosaic = new WidgetMosaic(3, 2);
  screen = new AppScreen(Size(320, 240), BLACK, topBar, bottomBar, mosaic);
  focus = screen;

  button0 = new Codingfield::UI::Button(mosaic);
  button0->SetBackgroundColor(BLUE);
  button0->SetTextColor(WHITE);
  button0->SetText("--C");
  button0->SetTitle("Fridge");
  button1 = new Codingfield::UI::Button(mosaic);
  button1->SetBackgroundColor(ORANGE);
  button1->SetTextColor(BLACK);
  button1->SetText("--C");
  button1->SetTitle("Beer");
  button2 = new Codingfield::UI::Button(mosaic);
  button2->SetBackgroundColor(YELLOW);
  button2->SetTextColor(BLACK);
  button2->SetText("--C");
  button2->SetTitle("Room");
  button3 = new Codingfield::UI::UpDownButton(mosaic);
  button3->SetBackgroundColor(PURPLE);
  button3->SetTextColor(WHITE);
  button3->SetText("---");
  button3->SetTitle("Heater");
  button4 = new Codingfield::UI::UpDownButton(mosaic);
  button4->SetBackgroundColor(GREEN);
  button4->SetTextColor(WHITE);
  button4->SetText("---");
  button4->SetTitle("Cooler");
  button5 = new Codingfield::UI::UpDownButton(mosaic);
  button5->SetBackgroundColor(MAROON);
  button5->SetTextColor(WHITE);
  button5->SetText("---");
  button5->SetTitle("Fan");

  topBar->SetUptime(0);
  topBar->SetWifiStatus(StatusBar::WifiStatuses::No_signal);

  bottomBar->SetButtonAText("<");
  bottomBar->SetButtonBText("SELECT");
  bottomBar->SetButtonCText(">");

  mosaic->SetZoomOnSelectedCallback([focus, screen, bottomBar](Widget* widget, bool edit) {
    if(edit) {
      if(widget->IsEditable()){
        bottomBar->SetButtonAText("-");
        bottomBar->SetButtonBText("APPLY");
        bottomBar->SetButtonCText("+");
      } else {
        bottomBar->SetButtonAText("");
        bottomBar->SetButtonBText("BACK");
        bottomBar->SetButtonCText("");
      }
    } else {
      bottomBar->SetButtonAText("<");
      bottomBar->SetButtonBText("SELECT");
      bottomBar->SetButtonCText(">");
    }
  });

  // Button 3
  button3->SetUpCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button3->SetDownCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button3->SetApplyCallback([&buttonState](UpDownButton* w) {
    app->HeaterState(buttonState);
    return false;
  });

  button3->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->HeaterState();
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  // Button 4
  button4->SetUpCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button4->SetDownCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button4->SetApplyCallback([&buttonState](UpDownButton* w) {
    app->CoolerState(buttonState);
    return false;
  });

  button4->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->CoolerState();
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });


  // Button 5
  button5->SetUpCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button5->SetDownCallback([&buttonState](UpDownButton* w) {
    if(buttonState == Actuators::Relays::States::Closed)
      buttonState = Actuators::Relays::States::Open;
    else
      buttonState = Actuators::Relays::States::Closed;
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });

  button5->SetApplyCallback([&buttonState](UpDownButton* w) {
    app->FanState(buttonState);
    return false;
  });

  button5->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->FanState();
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });


  screen->Draw();
}

void mqttMessageReceived(String &topic, String &payload) {
  Serial.println("MQTT message received : " + topic + " - " + payload);
  if(topic == "/jfbrew/relay/cooler/command") {
    if(payload == "0") {
      app->CoolerState(Actuators::Relays::States::Open);
    } else if(payload == "1") {
      app->CoolerState(Actuators::Relays::States::Closed);
    }
  } else if(topic == "/jfbrew/relay/heater/command") {
    if(payload == "0") {
      app->HeaterState(Actuators::Relays::States::Open);
    } else if(payload == "1") {
      app->HeaterState(Actuators::Relays::States::Closed);
    }
  } else if(topic == "/jfbrew/relay/fan/command") {
    if(payload == "0") {
      app->FanState(Actuators::Relays::States::Open);
    } else if(payload == "1") {
      app->FanState(Actuators::Relays::States::Closed);
    }
  } else if(topic == "/jfbrew/temperature/fridge/stub") {
    app->SetStubFridgeTemperature(payload.toFloat());
  } else if(topic == "/jfbrew/temperature/beer/stub") {
    app->SetStubBeerTemperature(payload.toFloat());
  } else if(topic == "/jfbrew/temperature/room/stub") {
    app->SetStubRoomTemperature(payload.toFloat());
  }
}
