#include "constants.h"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
// Additional libraries needed by WiFiManager
#include <DNSServer.h>
//Local DNS Server used for redirecting all rs to the configuration portal
#include <ESP8266WebServer.h>
//Local WebServer used to serve the configuration portal


// To use this sketch in an ESP8266 board you have to follow the instructions at
// https://github.com/esp8266/Arduino#installing-with-boards-manager


#include <LedControl.h>
// To control an MAX7219 enabled display
// https://github.com/debuti/LedControl
// More advanced library based on wayoda's LedControl
// https://github.com/wayoda/LedControl

#include <DoubleResetDetector.h>
// For entering Config mode by pressing reset twice
// Available on the library manager (DoubleResetDetector)
// https://github.com/datacute/DoubleResetDetector

#include <ArduinoJson.h>
// Required for the YouTubeApi and used for the config file
// Available on the library manager (ArduinoJson)
// https://github.com/bblanchon/ArduinoJson

#include <WiFiManager.h>
// For configuring the Wifi credentials without re-programing
// Availalbe on library manager (WiFiManager)
// https://github.com/tzapu/WiFiManager

#include "FS.h"
// For storing configurations
// Already included in the include path by the esp8266 core
// https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html


/** Typedefs **/
typedef struct {
  char* k;
  char* v;
} cookie_t;

typedef struct {
  char*         usr;
  char*         pwd;
  uint8_t       cookies_nb;
  cookie_t**    cookies;
  char*         csrftoken;
  unsigned long cooldown_until;
  unsigned long lasttry;
  unsigned long logins_nb;
  unsigned long ok_nb;
  unsigned long ko_nb;
} account_t;

typedef struct {
  uint8_t     start;
  uint8_t     end;
  uint16_t    spacing;
} night_t;

typedef struct {
  char*         profile;
  unsigned long max_requests_day_per_account;
  night_t       night;
  night_t       day;
  uint8_t       accounts_nb;
  account_t**   accounts;
} config_t;


/** Globals **/
WiFiClientSecure client;
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
LedControl lc = LedControl(DISPLAY_DATA_IN, DISPLAY_CLK, DISPLAY_LOAD, MAX7219_COUNT);

char ap[] = DEFAULT_AP;
char pw[] = DEFAULT_PW;

const unsigned char caCert[] = DIGICERT_SHA2_HIGH_ASSURANCE_SERVER_CA;
const unsigned int caCertLen = (sizeof(caCert) / sizeof(caCert[0]));
const char* host = INSTA_HOST;
const int httpsPort = INSTA_PORT;
const char url_postfix[] = INSTA_API_POSTFIX;

bool shouldSaveConfig  = false;                         // flag for saving data
bool enteredConfigMode = false;                         // flag for state machine

unsigned long api_lasttime = 0;                         //last time api request has been done

config_t config = {
  .profile                      = NULL,
  .max_requests_day_per_account = MAX_REQUESTS_DAY_PER_ACCOUNT,
  .night = {
    .start                      = DEFAULT_NIGHT_START,
    .end                        = DEFAULT_NIGHT_END,
    .spacing                    = DEFAULT_NIGHT_SPACING
  },
  .day = {
    .start                      = DEFAULT_NIGHT_END,
    .end                        = DEFAULT_NIGHT_START,
    .spacing                    = 0
  },
  .accounts_nb                  = 0,
  .accounts                     = NULL
};



void printConfig(config_t* config) {
  Serial.println("config {");
  Serial.println(String("  profile : ")+config->profile);
  Serial.println(String("  maxreqs : ")+config->max_requests_day_per_account);
  Serial.println(String("  night {"));
  Serial.println(String("    start :\t")+config->night.start);
  Serial.println(String("    end :\t")+config->night.end);
  Serial.println(String("    spacing :\t")+config->night.spacing);
  Serial.println(String("  }"));
  Serial.println(String("  day {"));
  Serial.println(String("    start :\t")+config->day.start);
  Serial.println(String("    end :\t")+config->day.end);
  Serial.println(String("    spacing :\t")+config->day.spacing);
  Serial.println(String("  }"));
  Serial.println(String("  accounts[")+config->accounts_nb+"] {");
  for (int i=0; i<config->accounts_nb; i++) {
    Serial.println(String("    account {"));
    Serial.println(String("      usr :\t\t")+config->accounts[i]->usr);
    Serial.print(String("      pwd :\t\t"));
    for (int j=0; j<strlen(config->accounts[i]->pwd); j++) Serial.print('*');
    Serial.println();
    Serial.println(String("      cool :\t\t")+config->accounts[i]->cooldown_until);
    Serial.println(String("      lasttry :\t\t")+config->accounts[i]->lasttry);
    Serial.println(String("      logins_nb :\t")+config->accounts[i]->logins_nb);
    Serial.println(String("      ok_nb :\t\t")+config->accounts[i]->ok_nb);
    Serial.println(String("      ko_nb :\t\t")+config->accounts[i]->ko_nb);
    Serial.println(String("      csrftoken :\t")+(config->accounts[i]->csrftoken?config->accounts[i]->csrftoken:"NULL"));
    Serial.println(String("      cookies[")+config->accounts[i]->cookies_nb+"] {");
    for (int j=0; j<config->accounts[i]->cookies_nb; j++)
      Serial.println(String("        ") + config->accounts[i]->cookies[j]->k + " : " + config->accounts[i]->cookies[j]->v);
    Serial.println(String("      }"));
    Serial.println(String("    }"));
  }
  Serial.println(String("  }"));
  Serial.println("}");
}

/**
 * Frees the memory associated to the dynamic object
 **/
bool freeConfig(config_t* config) {
  free(config->profile);
  for (int idx = 0; idx < config->accounts_nb; idx++) {
    free(config->accounts[idx]->usr);
    free(config->accounts[idx]->pwd);
    free(config->accounts[idx]->csrftoken);
    for (int j=0; j<config->accounts[idx]->cookies_nb; j++) {
      free(config->accounts[idx]->cookies[j]->k);
      free(config->accounts[idx]->cookies[j]->v);
      free(config->accounts[idx]->cookies[j]);
    }
    free(config->accounts[idx]);
  }
  free(config->accounts);
}

/**
 * Checks if the config makes sense
 **/
bool checkConfig(config_t* config) {
  return ((strcmp(config->profile, "") > 0) && \
          (config->accounts_nb > 0) && \
          (config->accounts != NULL));
}

/**
 * Loads the configuration from storaged JSON into dynamic object
 **/
bool loadConfig(config_t* config) {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  StaticJsonDocument<500> json;
  DeserializationError err = deserializeJson(json, configFile);
  if (err) {
    Serial.println("Failed to parse config file");
    return false;
  }

  if (json["profile"]) config->profile = strdup(json["profile"]);
  if (json["max_requests_day_per_account"]) config->max_requests_day_per_account = json["max_requests_day_per_account"];
  if (json["accounts"]) {
    config->accounts = (account_t**)malloc(sizeof(account_t*)*json["accounts"].size());
    for (int idx = 0; idx < json["accounts"].size(); idx++) {
      config->accounts[idx] = (account_t*)malloc(sizeof(account_t));
      config->accounts[idx]->usr = strdup(json["accounts"][idx]["usr"]);
      config->accounts[idx]->pwd = strdup(json["accounts"][idx]["pwd"]);
      config->accounts[idx]->cookies_nb = 0;
      config->accounts[idx]->cookies = NULL;
      config->accounts[idx]->cooldown_until = 0;
      config->accounts[idx]->lasttry = 0;
      config->accounts[idx]->logins_nb = 0;
      config->accounts[idx]->ok_nb = 0;
      config->accounts[idx]->ko_nb = 0;
      config->accounts[idx]->csrftoken = NULL;
      config->accounts_nb++;
    }
  }
  if (json["night"]) {
    config->night.start = json["night"]["start"];
    config->night.end = json["night"]["end"];
    config->night.spacing = json["night"]["spacing"];
    unsigned long night_dur = 60*60*(config->night.end>config->night.start ?
                                      config->night.end-config->night.start :
                                      config->night.end-(config->night.start-24));
    unsigned long day_dur = 24*60*60-night_dur;
    config->day.start = json["night"]["end"];
    config->day.end = json["night"]["start"];
    config->day.spacing = day_dur/(config->max_requests_day_per_account*config->accounts_nb-night_dur/config->night.spacing);
    Serial.print("Day spacing: ");
    Serial.println(config->day.spacing);
  }
  Serial.println("Config loaded");
  return true;
}

/**
 *
 **/
bool saveConfig(WiFiManagerParameter* profileWMP,
                WiFiManagerParameter* max_requests_day_per_accountWMP,
                WiFiManagerParameter* accountsWMP,
                WiFiManagerParameter* night_startWMP,
                WiFiManagerParameter* night_endWMP,
                WiFiManagerParameter* night_spacingWMP) {
  StaticJsonDocument<500> json;
  json["profile"] = profileWMP->getValue();
  json["max_requests_day_per_account"] = max_requests_day_per_accountWMP->getValue();

  char *ptr;
  char* input = strdup(accountsWMP->getValue());
  char* freeme = input;
  JsonArray accounts = json["accounts"].to<JsonArray>();
  while ((ptr = strsep(&input, ";")) != NULL) {
    char* pwd = strstr((const char *)ptr, ":") + 1;
    char* usr = ptr;
    *(pwd - 1) = '\0';
    JsonObject nested = accounts.createNestedObject();
    nested["usr"] = usr;
    nested["pwd"] = pwd;
  }
  free(freeme);

  json["night"]["start"] = night_startWMP->getValue();
  json["night"]["end"] = night_endWMP->getValue();
  json["night"]["spacing"] = night_spacingWMP->getValue();

  /* Open and overwrite */
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(json, configFile);
  Serial.println("Saved JSON");
  return true;
}

/**
 * Callback when your device enters configuration mode on failed WiFi connection attempt
 **/
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.print("Entered config mode: ");
  Serial.println(WiFi.softAPIP());
  // We don't want the next time the board resets to be considered a double reset so we remove the flag
  drd.stop();
  enteredConfigMode = true;
}

/**
 * Callback notifying us of the need to save config
 **/
void saveConfigCallback () {
  shouldSaveConfig = true;
}

/**
 * Setup routine
 **/
void setup() {
  /* Start the UART */
  Serial.begin(115200);
  Serial.println("");
  Serial.print("Initializing. Available heap: ");
  Serial.println(ESP.getFreeHeap());

  /* Start the display */
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  /* Start the led */
  pinMode(LED_BUILTIN, OUTPUT);
  /* Turn the LED on (Note that LOW is the voltage level */
  digitalWrite(LED_BUILTIN, LOW);

  /* Start the storage */
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount FS");
    return;
  }
  /* Load configs from flash */
  loadConfig(&config);

  /* Start wifimanager */
  WiFiManager wifiManager;
  char tmp[10];
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.setCustomHeadElement(HTML_CSS_STYLES);

  WiFiManagerParameter globalHelp(HTML_GLOBAL_HELP);
  wifiManager.addParameter(&globalHelp);

  WiFiManagerParameter profileWMP("profile", "Profile", config.profile, 50);  wifiManager.addParameter(&profileWMP);
  WiFiManagerParameter profileWMPHelp(HTML_PROFILE_HELP);  wifiManager.addParameter(&profileWMPHelp);
  WiFiManagerParameter accountsWMP("accounts", "Accounts", "", 100);  wifiManager.addParameter(&accountsWMP);
  WiFiManagerParameter accountsWMPHelp(HTML_ACCOUNTS_HELP);  wifiManager.addParameter(&accountsWMPHelp);
  snprintf(tmp, 10, "%d", config.night.start);
  WiFiManagerParameter night_startWMP("night_start", "Night starts at", tmp, 2);  wifiManager.addParameter(&night_startWMP);
  snprintf(tmp, 10, "%d", config.night.end);
  WiFiManagerParameter night_endWMP("night_end", "Night ends at", tmp, 2);  wifiManager.addParameter(&night_endWMP);
  snprintf(tmp, 10, "%d", config.night.spacing);
  WiFiManagerParameter night_spacingWMP("night_spacing", "Night spacing", tmp, 5);  wifiManager.addParameter(&night_spacingWMP);
  WiFiManagerParameter nightWMPHelp(HTML_NIGHT_HELP);  wifiManager.addParameter(&nightWMPHelp);
  snprintf(tmp, 10, "%d", config.max_requests_day_per_account);
  WiFiManagerParameter max_requests_day_per_accountWMP("max_requests_day_per_account", "Max requests per day and account", tmp, 10);  wifiManager.addParameter(&max_requests_day_per_accountWMP);
  WiFiManagerParameter max_requests_day_per_accountWMPHelp(HTML_REQUESTS_HELP);  wifiManager.addParameter(&max_requests_day_per_accountWMPHelp);

  lc.setString(0, "-conFiG-", 0);
  /** Check for double reset **/
  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    wifiManager.startConfigPortal(ap, pw);
  } else {
    Serial.println("No Double Reset Detected");
    wifiManager.autoConnect(ap, pw);
  }

  /* Turn the LED off by making the voltage HIGH */
  digitalWrite(LED_BUILTIN, HIGH);

  /* Config must be saved afterwards as you have no access to the results from the callback */
  if (shouldSaveConfig && enteredConfigMode) {
    saveConfig(&profileWMP, &max_requests_day_per_accountWMP, &accountsWMP, &night_startWMP, &night_endWMP, &night_spacingWMP);
    freeConfig(&config);
    loadConfig(&config);
  }

  /* Force Config mode if the needed info is not in place */
  if (checkConfig(&config)) {
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
      delay(5);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());

    /* Synchronize time using SNTP. This is necessary to verify that the TLS certificates offered by the server are currently valid. */
    Serial.print("Setting time using SNTP. ");
    configTime(0, 0, "pool.ntp.org");
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);  // Spain timezone
    delay(1000);
    time_t tnow = time(nullptr);
    Serial.print("Current time: ");
    Serial.print(String(ctime(&tnow)));

    // Load root certificate in DER format into WiFiClientSecure object
    bool res = client.setCACert(caCert, caCertLen);
    if (!res) {
      Serial.println("Failed to load root CA certificate!");
      while (true) {
        yield();
      }
    }
  }
  else {
    /* The config mode is forced by not stopping the drd */
    Serial.println("Forcing Config Mode and resetting");
    WiFi.disconnect();
    delay(500);
    ESP.restart();
    delay(5000);
  }

  lc.setString(0, "        ", 0);
  drd.stop();
}

#define HANDLE_RETURN_CODE {                     \
  if (line.startsWith(String("HTTP/1.1 "))) {    \
    line.remove(0, strlen("HTTP/1.1 "));         \
    int end = line.indexOf(' ');                 \
    returncode = line.substring(0, end).toInt(); \
  }                                              \
}

#define HANDLE_SET_COOKIE {                                           \
  if (line.startsWith(String("Set-Cookie: "))) {                      \
    line.remove(0, strlen("Set-Cookie: "));                           \
    int sep = line.indexOf('=');                                      \
    int end = line.indexOf(';');                                      \
    String key = line.substring(0, sep);                              \
    String value = line.substring(sep+1, end);                        \
    if (key == "csrftoken") {                                         \
      free(account->csrftoken);                                       \
      account->csrftoken = strdup(value.c_str());                     \
    }                                                                 \
    bool found = false;                                               \
    for (int idx=0; idx<account->cookies_nb; idx++) {                 \
      if (key == account->cookies[idx]->k) {                          \
        free(account->cookies[idx]->v);                               \
        account->cookies[idx]->v=strdup(value.c_str());               \
        found = true;                                                 \
      }                                                               \
    }                                                                 \
    if (!found) {                                                     \
      account->cookies =                                              \
        (cookie_t**)realloc(account->cookies,                         \
                            sizeof(cookie_t*)*account->cookies_nb+1); \
      account->cookies[account->cookies_nb] =                         \
        (cookie_t*)malloc(sizeof(cookie_t));                          \
      account->cookies[account->cookies_nb]->k =                      \
        strdup(key.c_str());                                          \
      account->cookies[account->cookies_nb]->v =                      \
        strdup(value.c_str());                                        \
      account->cookies_nb++;                                          \
    }                                                                 \
  }                                                                   \
}

void logout(account_t* account) {
  /* Delete all the cookies! */
  while (account->cookies_nb) {
    account->cookies_nb--;
    free(account->cookies[account->cookies_nb]->k);
    free(account->cookies[account->cookies_nb]->v);
    free(account->cookies[account->cookies_nb]);
    account->cookies = (cookie_t**)realloc(account->cookies, sizeof(cookie_t*)*account->cookies_nb);
  }
}

bool getCSRF(account_t* account) {
  Serial.print("Getting CSRF for ");
  Serial.println(host);

  /* Connect to remote server */
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }

  /* Verify validity of server's certificate */
  if (!client.verifyCertChain(host)) {
    Serial.println("certificate verification failed!");
    return false;
  }

  /* Get the initial cookies */
  client.print(String("GET / HTTP/1.1\r\n") +
               "Host: " + host + "\r\n" +
               "User-Agent: " + STORIES_UA + "\r\n" +
               "Connection: close\r\n\r\n");

  long returncode = 0;
  while (client.connected() || client.available()) {
   if (client.available()) {
    String line = client.readStringUntil('\n');

    HANDLE_RETURN_CODE
    HANDLE_SET_COOKIE
   }
  }

  return (returncode == 200);
}


bool login(account_t* account) {
  Serial.print("Login to ");
  Serial.println(host);

  /* Connect to remote server */
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }

  /* Verify validity of server's certificate */
  if (!client.verifyCertChain(host)) {
    Serial.println("certificate verification failed!");
    return false;
  }

  client.print(String("POST ") + LOGIN_PATH + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " + STORIES_UA + "\r\n" +
               "X-CSRFToken: " + account->csrftoken + "\r\n" +
               "Content-Type: " + "application/x-www-form-urlencoded" + "\r\n" +
               "Content-Length: " + String(strlen("username=") +
                                           strlen(account->usr) +
                                           strlen("&password=") +
                                           strlen(account->pwd)) + "\r\n" +
               "Connection: close\r\n");
  for (int idx=0; idx<account->cookies_nb; idx++)
    client.print(String("Cookie: ")+account->cookies[idx]->k+"="+account->cookies[idx]->v+"\r\n");
  client.print(String("\r\n") +
               "username="+account->usr+"&password="+account->pwd);

  long returncode = 0;
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');

      HANDLE_RETURN_CODE
      HANDLE_SET_COOKIE

      if ((returncode == 200) && (line.indexOf(String("\"authenticated\": true")) != -1)) {
        account->logins_nb++;
        return true;
      }
    }
  }

  return false;
}


/**
   Connects to instagram and returns the count
 **/
long followersCount(account_t* account) {
  // Connect to remote server
  Serial.print("Connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return -1;
  }

  // Verify validity of server's certificate
  if (!client.verifyCertChain(host)) {
    Serial.println("certificate verification failed!");
    return -1;
  }

  /* Make the request */
  client.print(String("GET ") + "/" + config.profile + INSTA_API_POSTFIX + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " + CHROME_WIN_UA + "\r\n" +
               "X-CSRFToken: " + account->csrftoken + "\r\n" +
               "Connection: close\r\n");
  for (int idx=0; idx<account->cookies_nb; idx++)
    client.print(String("Cookie: ")+account->cookies[idx]->k+"="+account->cookies[idx]->v+"\r\n");
  client.print(String("\r\n"));

  account->lasttry = millis();

  long returncode = 0;
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');

      HANDLE_RETURN_CODE
      HANDLE_SET_COOKIE

      /* Too many requests, lets cooldown this account */
      if (returncode == 429) {
        account->cooldown_until = millis() + COOLDOWN_MIN;
        break;
      }

      int pos = line.indexOf(String("\"edge_followed_by\":{\"count\":"));
      if ((pos != -1) && (returncode == 200)) {
        int start = pos + strlen("\"edge_followed_by\":{\"count\":");
        int end = line.indexOf('\"', start);
        account->ok_nb++;
        return line.substring(start, end).toInt();
      }
    }
  }

  /* Requested data not found, force a login */
  Serial.print("Request failed with code ");
  Serial.println(returncode);
  account->ko_nb++;
  logout(account);
  return -1;
}

long count = 0;

account_t* selectAccount() {
  account_t** available = NULL;
  uint available_nb = 0;
  for (int i=0; i<config.accounts_nb; i++) {
    if (config.accounts[i]->cooldown_until < millis()) {
      available = (account_t**)realloc(available, (available_nb+1) * sizeof(account_t*));
      available[available_nb]=config.accounts[i];
      available_nb++;
    }
  }

  account_t* selected = NULL;
  if (available_nb > 0) {
    unsigned long older = ULONG_MAX;
    for (int i=0; i<available_nb; i++) {
      if (available[i]->lasttry < older) {
        older = available[i]->lasttry;
        selected = available[i];
      }
    }
  }
  else {
    unsigned long min = ULONG_MAX;
    for (int i=0; i<config.accounts_nb; i++) {
      unsigned long rem = config.accounts[i]->cooldown_until - millis();
      if (rem - min) {
        min = rem;
        selected = config.accounts[i];
      }
    }
    lc.setString(0, "cooldo_n", 0);
    delay(min);
    lc.setString(0, "        ", 0);
  }

  free(available);
  return selected;
}

account_t* doOrGetLogin(account_t* account){
  if (account->cookies_nb > 0)
    return account;
  else {
    if (getCSRF(account) && login(account)) {
      return account;
    }
  }
  return NULL;
}

void loop() {
  bool night = false;
  time_t tnow = time(nullptr);
  struct tm *tnowr = gmtime(&tnow);
  uint8_t percent = 0;
  uint8_t progress = 0;

  if (config.night.start < config.night.end) {
    if (config.night.start <= tnowr->tm_hour and tnowr->tm_hour < config.night.end) night = true;
  }
  else {
    if (config.night.start <= tnowr->tm_hour or tnowr->tm_hour < config.night.end) night = true;
  }

  if ((night && millis() - api_lasttime > config.night.spacing * 1000) ||
      (!night && millis() - api_lasttime > config.day.spacing * 1000)){
    Serial.println("---");
    api_lasttime = millis();
    Serial.print("Free HEAP: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("Date: ");
    Serial.print(String(ctime(&tnow)));

    if (account_t* account = doOrGetLogin(selectAccount())) {
        long result =  followersCount(account);
        if (result > 0)
          count = result;
    }

    printConfig(&config);

    Serial.println("---");
  }

  percent = (millis() - api_lasttime)*100 / (1000 * (night ? config.night.spacing : config.day.spacing));
  progress = (percent < 100/9 ? 0b11111111 :
              (percent < 200/9 ? 0b01111111 :
               (percent < 300/9 ? 0b00111111 :
                (percent < 400/9 ? 0b00011111 :
                 (percent < 500/9 ? 0b00001111 :
                  (percent < 600/9 ? 0b00000111 :
                   (percent < 700/9 ? 0b00000011 :
                    (percent < 800/9 ? 0b00000001 : 0b00000000))))))));
  lc.setNumber(0, count, progress);
}
