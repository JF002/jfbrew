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

const char* temperatureSymbole = "C";
const char* valueOff = "0";
const char* valueOn = "1";
const char* topic_temperaturefridge = "/jfbrew/temperature/fridge";
const char* topic_temperatureBeer = "/jfbrew/temperature/beer";
const char* topic_temperatureRoom = "/jfbrew/temperature/room";
const char* topic_heaterState = "/jfbrew/relay/heater/state";
const char* topic_coolerState = "/jfbrew/relay/cooler/state";
const char* topic_coolerPwmValue = "/jfbrew/relay/cooler/pwm_value";
const char* topic_heaterPwmValue = "/jfbrew/relay/heater/pwm_value";
const char* topic_coolerPwmState = "/jfbrew/relay/cooler/pwm_state";
const char* topic_heaterPwmState = "/jfbrew/relay/heater/pwm_state";
const char* topic_fanState = "/jfbrew/relay/fan/state";
const char* topic_beerSetPoint = "/jfbrew/beer/setpoint";
const char* topic_coolerCommand = "/jfbrew/relay/cooler/command";
const char* topic_heaterCommand = "/jfbrew/relay/heater/command";
const char* topic_heaterPwmCommand = "/jfbrew/relay/heater/pwm_command";
const char* topic_coolerPwmCommand = "/jfbrew/relay/cooler/pwm_command";
const char* topic_heaterPwmConsign = "/jfbrew/relay/heater/pwm_consign";
const char* topic_coolerPwmConsign = "/jfbrew/relay/cooler/pwm_consign";
const char* topic_fanCommand = "/jfbrew/relay/fan/command";
const char* topic_temperatureFridgeStub = "/jfbrew/temperature/fridge/stub";
const char* topic_temperatureBeerStub = "/jfbrew/temperature/beer/stub";
const char* topic_temperatureRoomStub = "/jfbrew/temperature/room/stub";
constexpr size_t stringBufferSize = 128;
char stringBuffer[stringBufferSize];

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
    auto fridgeValue = app->FridgeTemperature();
    snprintf(stringBuffer, stringBufferSize, "%.2f%s", fridgeValue, temperatureSymbole);
    button0->SetText(stringBuffer);
    snprintf(stringBuffer, stringBufferSize, "%.2f", fridgeValue);
    mqtt.publish(topic_temperaturefridge, stringBuffer);

    auto beerValue = app->BeerTemperature();
    snprintf(stringBuffer, stringBufferSize, "%.2f%s", beerValue, temperatureSymbole);
    button1->SetText(stringBuffer);
    snprintf(stringBuffer, stringBufferSize, "%.2f", beerValue);
    mqtt.publish(topic_temperatureBeer, stringBuffer);

    auto roomValue = app->RoomTemperature();
    snprintf(stringBuffer, stringBufferSize, "%.2f%s", roomValue, temperatureSymbole);
    button2->SetText(stringBuffer);
    snprintf(stringBuffer, stringBufferSize, "%.2f", roomValue);
    mqtt.publish(topic_temperatureRoom, stringBuffer);

    if(app->HeaterRelayState() == Actuators::Relays::States::Open)
      mqtt.publish(topic_heaterState, valueOff);
    else
      mqtt.publish(topic_heaterState, valueOn);

    if(!button3->AreControlsEnabled()) {
      button3->SetText(Actuators::Relays::ToString(app->HeaterRelayState()));
    }

    if(app->CoolerRelayState() == Actuators::Relays::States::Open)
      mqtt.publish(topic_coolerState, valueOff);
    else
      mqtt.publish(topic_coolerState, valueOn);

    mqtt.publish(topic_coolerPwmValue, String(app->CoolerPwm()));
    mqtt.publish(topic_heaterPwmValue, String(app->HeaterPwm()));
    mqtt.publish(topic_coolerPwmState, String(app->IsCoolerPwmActivated()));
    mqtt.publish(topic_heaterPwmState, String(app->IsHeaterPwmActivated()));

    if(!button4->AreControlsEnabled()) {
      button4->SetText( Actuators::Relays::ToString(app->CoolerRelayState()));
    }

    if(app->FanRelayState() == Actuators::Relays::States::Open)
      mqtt.publish(topic_fanState, valueOff);
    else
      mqtt.publish(topic_fanState, valueOn);

    if(!button5->AreControlsEnabled()) {
      button5->SetText(Actuators::Relays::ToString(app->FanRelayState()));
    }

    uptimeHours = millis() / (60*60000);
    topBar->SetUptime(uptimeHours);

    snprintf(stringBuffer, stringBufferSize, "%02d:%02d:%02d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
    topBar->SetDateTime(stringBuffer);
  }
#if 1
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
#endif
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
  mqtt.subscribe(topic_coolerCommand);
  mqtt.subscribe(topic_heaterCommand);
  mqtt.subscribe(topic_fanCommand);

  mqtt.subscribe(topic_heaterPwmCommand);
  mqtt.subscribe(topic_coolerPwmCommand);
  mqtt.subscribe(topic_heaterPwmConsign);
  mqtt.subscribe(topic_coolerPwmConsign);

  if(config.IsTemperatureSensorStubbed()) {
    mqtt.subscribe(topic_temperatureFridgeStub);
    mqtt.subscribe(topic_temperatureBeerStub);
    mqtt.subscribe(topic_temperatureRoomStub);
  }

  mqtt.subscribe(topic_beerSetPoint);

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
    app->HeaterRelayState(buttonState);
    return false;
  });

  button3->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->HeaterRelayState();
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
    app->CoolerRelayState(buttonState);
    return false;
  });

  button4->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->CoolerRelayState();
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
    app->FanRelayState(buttonState);
    return false;
  });

  button5->SetCancelCallback([&buttonState](UpDownButton* w) {
    buttonState = app->FanRelayState();
    w->SetText(Actuators::Relays::ToString(buttonState).c_str());
    return true;
  });


  screen->Draw();
}

void mqttMessageReceived(String &topic, String &payload) {
  Serial.println("MQTT message received : " + topic + " - " + payload);
  if(topic == topic_coolerCommand) {
    if(payload == valueOff) {
      app->CoolerRelayState(Actuators::Relays::States::Open);
    } else if(payload == valueOn) {
      app->CoolerRelayState(Actuators::Relays::States::Closed);
    }
  } else if(topic == topic_heaterCommand) {
    if(payload == valueOff) {
      app->HeaterRelayState(Actuators::Relays::States::Open);
    } else if(payload == valueOn) {
      app->HeaterRelayState(Actuators::Relays::States::Closed);
    }
  } else if(topic == topic_fanCommand) {
    if(payload == valueOff) {
      app->FanRelayState(Actuators::Relays::States::Open);
    } else if(payload == valueOn) {
      app->FanRelayState(Actuators::Relays::States::Closed);
    }
  } else if(topic == topic_temperatureFridgeStub) {
    app->SetStubFridgeTemperature(payload.toFloat());
  } else if(topic == topic_temperatureBeerStub) {
    app->SetStubBeerTemperature(payload.toFloat());
  } else if(topic == topic_temperatureRoomStub) {
    app->SetStubRoomTemperature(payload.toFloat());
  } else if(topic == topic_coolerPwmCommand) {
    if(payload == valueOff) {
      app->ActivateCoolerPwm(false);
    } else if(payload == valueOn) {
      app->ActivateCoolerPwm(true);
    }
  } else if(topic == topic_heaterPwmCommand) {
    if(payload == valueOff) {
      app->ActivateHeaterPwm(false);
    } else if(payload == valueOn) {
      app->ActivateHeaterPwm(true);
    }
  } else if(topic == topic_heaterPwmConsign) {
    Serial.println("Heater PWM : " + String(payload.toInt()));
    app->HeaterPwm(payload.toInt());
  } else if(topic == topic_coolerPwmConsign) {
    app->CoolerPwm(payload.toInt());
  } else if(topic == topic_beerSetPoint) {
    app->BeerSetPoint(payload.toFloat());
  }

}
