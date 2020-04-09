# instacount

Arduino sketch to display the followers count for any user

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
 * Breakout board fro ESP12E
 * 3,3V voltage regulator

### Connections
ESP12E D6 to MAX7219 CLK
ESP12E D7 to MAX7219 CS
ESP12E D8 to MAX7219 DIN


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
