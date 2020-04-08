#define XSTR(s)	 STR(s)
#define STR(s)	 #s

#define DISPLAY_DATA_IN				15
#define DISPLAY_CLK						12
#define DISPLAY_LOAD					13
#define MAX7219_COUNT					1

#define DRD_TIMEOUT 					5						// Number of seconds after reset during which a subseqent reset will be considered a double reset. This sketch uses drd.stop() rather than relying on the timeout
#define DRD_ADDRESS						0						// RTC Memory Address for the DoubleResetDetector to use

#define INSTA_SCHEMA					"https://"
#define INSTA_HOST						"www.instagram.com"
#define BASE_PATH						  "/"
#define LOGIN_PATH						"/accounts/login/ajax/"
#define INSTA_PORT						443
#define INSTA_API_POSTFIX			"/?__a=1"
//#define MAX_REQUESTS_DAY_PER_ACCOUNT	9999
#define MAX_REQUESTS_DAY_PER_ACCOUNT	500
#define CHROME_WIN_UA 				"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.87 Safari/537.36"
#define STORIES_UA						"Instagram 123.0.0.21.114 (iPhone; CPU iPhone OS 11_4 like Mac OS X; en_US; en-US; scale=2.00; 750x1334) AppleWebKit/605.1.15"
#define COOLDOWN_MIN  				6*60*60*1000
#define COOLDOWN_INC  				3

#define DEFAULT_AP						"instacount"
#define DEFAULT_PW						"mendiolas"
#define DEFAULT_NIGHT_START		0
#define DEFAULT_NIGHT_END			8
#define DEFAULT_NIGHT_SPACING	3600

#define HTML_CSS_STYLES \
"<style>" \
".alert {" \
"  margin: 10px;" \
"  padding: 20px;" \
"  opacity: 1;" \
"  transition: opacity 0.6s;" \
"  margin-bottom: 15px;" \
"  font-family: -apple-system,BlinkMacSystemFont,\"Segoe UI\",Roboto,\"Helvetica Neue\",Arial,sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\";" \
"  box-sizing: border-box;" \
"  border: 1px solid transparent;" \
"  border-radius: .25rem;" \
"  line-height: 1.5;" \
"}" \
".alert.info {" \
"  color: #004085;" \
"  background-color: #cce5ff;" \
"  border-color: #b8daff;" \
"}" \
".alert.light {" \
"  color: #818182;" \
"  background-color: #fefefe;" \
"  border-color: #fdfdfe;" \
"  margin: 2px;" \
"  padding: 2px;" \
"}" \
"</style>"

#define HTML_GLOBAL_HELP \
"<div class=\"alert info\">" \
"  <strong>Instacount instructions</strong>" \
"  <p>The configuration of instacount is made through a captive wifi portal. At first, since no configuration is yet made, instacount will expose the captive portal by default. Just connect to it and modify the parameters to fit your needs. If at some point you want to reconfigure instacount, just press the reset button twice, and the captive portal will showup again. Whenever the led stays on for 10 seconds or so after boot, the config portal will be on and -conFiG- will be shown in the display . Mind that if the configuration cannot be applied the captive portal will show up again.</p>" \
"  <p>Sadly, Instagram has no public API to retrieve the followers count for a given user. Because of that, instacount uses web scrapping methods to retrieve this information. Although it works, it is not the best solution since Instagram  controls the number of requests made to its servers and can deny the access to resources for a given user. As of April 2020 the quota is 500 requests/user*day, meaning that a request can be made each 2.88 minutes</p>" \
"  <p>To overcome this limitation, instacount uses two concepts combined:" \
"    <ul>" \
"    <li>Poll more often at daytime and less often at night.</li>" \
"    <li>Use several accounts to be able to make requests as quick as possible.</li>" \
"    </ul>" \
"  </p>" \
"  <p>The display dots will be lightened in order to show the amount of time left until the next request.</p>" \
"</div>"

#define HTML_PROFILE_HELP\
"<div class=\"alert light\">" \
"  Insert the Instagram profile to be polled for followers count" \
"</div>"

#define HTML_ACCOUNTS_HELP\
"<div class=\"alert light\">" \
"  Insert at least one account/password. The format is: <br>" \
"  user1:passwd1;user2:passwd2;...<br>" \
"  Previously introduced credentials are not shown for security reasons. The field is limited to 100 characters." \
"</div>"

#define HTML_NIGHT_HELP\
"<div class=\"alert light\">" \
"  Insert the start and finish hours for the night, and the number of seconds you want instacount to wait between requests. <br>" \
"  Remember, the less usage at night means the more at day!" \
"</div>"

#define HTML_REQUESTS_HELP\
"<div class=\"alert light\">" \
"  This should be set to "XSTR(MAX_REQUESTS_DAY_PER_ACCOUNT)" but if you suffer a lot of cooldowns you'll probably need to decrease this value" \
"</div>"

// Digicert intermediate (valid until ‎‎October ‎22,‎2028 1:00:00 PM) x509 encoded as DER
#define DIGICERT_SHA2_HIGH_ASSURANCE_SERVER_CA {0x30,0x82,0x03,0xC5,0x30,0x82,0x02,0xAD,0xA0,0x03,0x02,0x01,0x02,0x02,0x10,0x02,0xAC,0x5C,0x26,0x6A,0x0B,0x40,0x9B,0x8F,0x0B,0x79,0xF2,0xAE,0x46,0x25,0x77,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,0x30,0x6C,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x15,0x30,0x13,0x06,0x03,0x55,0x04,0x0A,0x13,0x0C,0x44,0x69,0x67,0x69,0x43,0x65,0x72,0x74,0x20,0x49,0x6E,0x63,0x31,0x19,0x30,0x17,0x06,0x03,0x55,0x04,0x0B,0x13,0x10,0x77,0x77,0x77,0x2E,0x64,0x69,0x67,0x69,0x63,0x65,0x72,0x74,0x2E,0x63,0x6F,0x6D,0x31,0x2B,0x30,0x29,0x06,0x03,0x55,0x04,0x03,0x13,0x22,0x44,0x69,0x67,0x69,0x43,0x65,0x72,0x74,0x20,0x48,0x69,0x67,0x68,0x20,0x41,0x73,0x73,0x75,0x72,0x61,0x6E,0x63,0x65,0x20,0x45,0x56,0x20,0x52,0x6F,0x6F,0x74,0x20,0x43,0x41,0x30,0x1E,0x17,0x0D,0x30,0x36,0x31,0x31,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x5A,0x17,0x0D,0x33,0x31,0x31,0x31,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x5A,0x30,0x6C,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x15,0x30,0x13,0x06,0x03,0x55,0x04,0x0A,0x13,0x0C,0x44,0x69,0x67,0x69,0x43,0x65,0x72,0x74,0x20,0x49,0x6E,0x63,0x31,0x19,0x30,0x17,0x06,0x03,0x55,0x04,0x0B,0x13,0x10,0x77,0x77,0x77,0x2E,0x64,0x69,0x67,0x69,0x63,0x65,0x72,0x74,0x2E,0x63,0x6F,0x6D,0x31,0x2B,0x30,0x29,0x06,0x03,0x55,0x04,0x03,0x13,0x22,0x44,0x69,0x67,0x69,0x43,0x65,0x72,0x74,0x20,0x48,0x69,0x67,0x68,0x20,0x41,0x73,0x73,0x75,0x72,0x61,0x6E,0x63,0x65,0x20,0x45,0x56,0x20,0x52,0x6F,0x6F,0x74,0x20,0x43,0x41,0x30,0x82,0x01,0x22,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x82,0x01,0x0F,0x00,0x30,0x82,0x01,0x0A,0x02,0x82,0x01,0x01,0x00,0xC6,0xCC,0xE5,0x73,0xE6,0xFB,0xD4,0xBB,0xE5,0x2D,0x2D,0x32,0xA6,0xDF,0xE5,0x81,0x3F,0xC9,0xCD,0x25,0x49,0xB6,0x71,0x2A,0xC3,0xD5,0x94,0x34,0x67,0xA2,0x0A,0x1C,0xB0,0x5F,0x69,0xA6,0x40,0xB1,0xC4,0xB7,0xB2,0x8F,0xD0,0x98,0xA4,0xA9,0x41,0x59,0x3A,0xD3,0xDC,0x94,0xD6,0x3C,0xDB,0x74,0x38,0xA4,0x4A,0xCC,0x4D,0x25,0x82,0xF7,0x4A,0xA5,0x53,0x12,0x38,0xEE,0xF3,0x49,0x6D,0x71,0x91,0x7E,0x63,0xB6,0xAB,0xA6,0x5F,0xC3,0xA4,0x84,0xF8,0x4F,0x62,0x51,0xBE,0xF8,0xC5,0xEC,0xDB,0x38,0x92,0xE3,0x06,0xE5,0x08,0x91,0x0C,0xC4,0x28,0x41,0x55,0xFB,0xCB,0x5A,0x89,0x15,0x7E,0x71,0xE8,0x35,0xBF,0x4D,0x72,0x09,0x3D,0xBE,0x3A,0x38,0x50,0x5B,0x77,0x31,0x1B,0x8D,0xB3,0xC7,0x24,0x45,0x9A,0xA7,0xAC,0x6D,0x00,0x14,0x5A,0x04,0xB7,0xBA,0x13,0xEB,0x51,0x0A,0x98,0x41,0x41,0x22,0x4E,0x65,0x61,0x87,0x81,0x41,0x50,0xA6,0x79,0x5C,0x89,0xDE,0x19,0x4A,0x57,0xD5,0x2E,0xE6,0x5D,0x1C,0x53,0x2C,0x7E,0x98,0xCD,0x1A,0x06,0x16,0xA4,0x68,0x73,0xD0,0x34,0x04,0x13,0x5C,0xA1,0x71,0xD3,0x5A,0x7C,0x55,0xDB,0x5E,0x64,0xE1,0x37,0x87,0x30,0x56,0x04,0xE5,0x11,0xB4,0x29,0x80,0x12,0xF1,0x79,0x39,0x88,0xA2,0x02,0x11,0x7C,0x27,0x66,0xB7,0x88,0xB7,0x78,0xF2,0xCA,0x0A,0xA8,0x38,0xAB,0x0A,0x64,0xC2,0xBF,0x66,0x5D,0x95,0x84,0xC1,0xA1,0x25,0x1E,0x87,0x5D,0x1A,0x50,0x0B,0x20,0x12,0xCC,0x41,0xBB,0x6E,0x0B,0x51,0x38,0xB8,0x4B,0xCB,0x02,0x03,0x01,0x00,0x01,0xA3,0x63,0x30,0x61,0x30,0x0E,0x06,0x03,0x55,0x1D,0x0F,0x01,0x01,0xFF,0x04,0x04,0x03,0x02,0x01,0x86,0x30,0x0F,0x06,0x03,0x55,0x1D,0x13,0x01,0x01,0xFF,0x04,0x05,0x30,0x03,0x01,0x01,0xFF,0x30,0x1D,0x06,0x03,0x55,0x1D,0x0E,0x04,0x16,0x04,0x14,0xB1,0x3E,0xC3,0x69,0x03,0xF8,0xBF,0x47,0x01,0xD4,0x98,0x26,0x1A,0x08,0x02,0xEF,0x63,0x64,0x2B,0xC3,0x30,0x1F,0x06,0x03,0x55,0x1D,0x23,0x04,0x18,0x30,0x16,0x80,0x14,0xB1,0x3E,0xC3,0x69,0x03,0xF8,0xBF,0x47,0x01,0xD4,0x98,0x26,0x1A,0x08,0x02,0xEF,0x63,0x64,0x2B,0xC3,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,0x03,0x82,0x01,0x01,0x00,0x1C,0x1A,0x06,0x97,0xDC,0xD7,0x9C,0x9F,0x3C,0x88,0x66,0x06,0x08,0x57,0x21,0xDB,0x21,0x47,0xF8,0x2A,0x67,0xAA,0xBF,0x18,0x32,0x76,0x40,0x10,0x57,0xC1,0x8A,0xF3,0x7A,0xD9,0x11,0x65,0x8E,0x35,0xFA,0x9E,0xFC,0x45,0xB5,0x9E,0xD9,0x4C,0x31,0x4B,0xB8,0x91,0xE8,0x43,0x2C,0x8E,0xB3,0x78,0xCE,0xDB,0xE3,0x53,0x79,0x71,0xD6,0xE5,0x21,0x94,0x01,0xDA,0x55,0x87,0x9A,0x24,0x64,0xF6,0x8A,0x66,0xCC,0xDE,0x9C,0x37,0xCD,0xA8,0x34,0xB1,0x69,0x9B,0x23,0xC8,0x9E,0x78,0x22,0x2B,0x70,0x43,0xE3,0x55,0x47,0x31,0x61,0x19,0xEF,0x58,0xC5,0x85,0x2F,0x4E,0x30,0xF6,0xA0,0x31,0x16,0x23,0xC8,0xE7,0xE2,0x65,0x16,0x33,0xCB,0xBF,0x1A,0x1B,0xA0,0x3D,0xF8,0xCA,0x5E,0x8B,0x31,0x8B,0x60,0x08,0x89,0x2D,0x0C,0x06,0x5C,0x52,0xB7,0xC4,0xF9,0x0A,0x98,0xD1,0x15,0x5F,0x9F,0x12,0xBE,0x7C,0x36,0x63,0x38,0xBD,0x44,0xA4,0x7F,0xE4,0x26,0x2B,0x0A,0xC4,0x97,0x69,0x0D,0xE9,0x8C,0xE2,0xC0,0x10,0x57,0xB8,0xC8,0x76,0x12,0x91,0x55,0xF2,0x48,0x69,0xD8,0xBC,0x2A,0x02,0x5B,0x0F,0x44,0xD4,0x20,0x31,0xDB,0xF4,0xBA,0x70,0x26,0x5D,0x90,0x60,0x9E,0xBC,0x4B,0x17,0x09,0x2F,0xB4,0xCB,0x1E,0x43,0x68,0xC9,0x07,0x27,0xC1,0xD2,0x5C,0xF7,0xEA,0x21,0xB9,0x68,0x12,0x9C,0x3C,0x9C,0xBF,0x9E,0xFC,0x80,0x5C,0x9B,0x63,0xCD,0xEC,0x47,0xAA,0x25,0x27,0x67,0xA0,0x37,0xF3,0x00,0x82,0x7D,0x54,0xD7,0xA9,0xF8,0xE9,0x2E,0x13,0xA3,0x77,0xE8,0x1F,0x4A}