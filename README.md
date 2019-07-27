# JFBrew

JFBrew is a DIY temperature controller for a beer fermentation chamber. 

It regulates the temperature inside the chamber (a modifier fridge, for example) so that the temperature of the wort (fermenting beer) is stable and controlled.

It uses 3 temperature sensors (beer, fridge and ambiant) and 2 relays (cooler and heater). The brain of JFBrew is a M5Stack board, based on the ESP32 processor.

More information: https://www.hackster.io/jf002/jfbrew-7c17ed

**Warning** : The source code contains hard-coded values (id of the temperature senors, for example) that are specific to my setup. 