# instacount

Sketch and hardware solution to display the followers count for any user

## Quick start
 * Connect to _instacount_ hotspot -> A captive portal for configuration will open
 * Insert your wifi details, the target Instagram account and your Instagram credentials. Adjust the other parameters to taste
 * Once saved the device will try to connect to the wifi network, if successful it will try to fetch the followers count using one of the provided Instagram credentials, if successful you are good to go, if not ..
 
## Troubleshooting
 * The device doesnt show anything on the display
   * Sometimes these chinese displays are unresponsive. Unplug and replug several times until the digits light up
 * The device never goes out of config
   * The wifi credentials are not correct, recheck them
 * The device always shows 0 followers
   * The Instagram credentials are not correct, recheck them. 
   * Try to use those credentials in a browser to see if they are challenged or banned by Instagram
 * The device shows 0 followers when powered-up
   * I'm working on this
 * When I reconfigure the device, the Instagram credentials are not persisted
   * They are, but they are not showed for security reasons! If the module is set to config mode without you knowing it, and your neighbour connects to the config page s/he will see your plain text password

## Description and usage
Polling updated display to show the count of followers of a given Instagram account.

Sadly, Instagram has no public API to retrieve the followers count for a given user. Because of that, instacount uses web scrapping methods to retrieve this information. Although it works, it is not the best solution as Instagram controls the number of requests made to its servers and can deny the access to resources for a given user. As of April 2020 the quota is 500 requests/user*day, meaning that a request can be made each 2.88 minutes

To overcome this limitation, instacount uses two concepts combined:
 * Poll more often at daytime and less often at night.
 * Use several accounts to be able to make requests as quick as possible.

Last, Instagram handles suspicious activities by issuing a challenge. If a given account is challenged, it wont be able to login and thus retrieve the count. To overcome this just login to the account from a web browser and response to the challenge. As instacount wont warn about this, to detect and be able to solve this, it is best to use your own accounts with instacount (and not dummy accounts)

## Features
 * Login based approach to enable a multi-account strategy
 * Configurable via wifi captive portal
 * Countdown until next request shown via display dots
 * Smart cooldown mechanism to temporarily disable the accounts that got too many requests
 * .dxf for case routing/printing also included!

## Hardware design
### BOM
 * ESP12E board
 * MAX7219 8-digit 8-segments module
 * Breakout board for ESP12E
 * 3,3V voltage regulator (remove the 0 Ohm resistor from the breakout board!)

### Connections
```
ESP12E GPIO13 to MAX7219 CLK
ESP12E GPIO12 to MAX7219 CS
ESP12E GPIO14 to MAX7219 DIN
```

## Build environment
 * PlatformIO: Quick-install with this *nosudo* oneliner `python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"; export PATH=$PATH:~/.platformio/penv/bin`

## Libraries
 * WifiManager
 * LedControl
 * DoubleResetDetector
 * ArduinoJSON

## Building
 * Go to the project folder and type `pio run` to build
 * Or `pio run -t upload` to upload
