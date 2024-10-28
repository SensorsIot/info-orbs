#include <Arduino.h>
#include "core/wifiWidget.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h> // For ESP32 to store config in flash

#define CONFIG_PIN 0 // GPIO0 pin

Preferences preferences;
bool shouldSaveConfig = false;
bool res;

// Global variables
String c_timezone;
String c_weather_loc;
String c_stock_ticker;
String c_weather_units;
String c_format_24h;
String c_show_am_pm;
String c_show_seconds;

// WiFiManager and parameter setup
WiFiManager wm;
WiFiManagerParameter timezone_param("timezone", "Timezone (e.g., America/Vancouver)", TIMEZONE_API_LOCATION, 50);
WiFiManagerParameter weather_loc_param("weather_loc", "Weather Location (e.g., Victoria, BC)", WEATHER_LOCAION, 50);
WiFiManagerParameter stock_ticker_param("stocks", "Stock Ticker List (comma-separated)", STOCK_TICKER_LIST, 50);
WiFiManagerParameter weather_units_param("units", "Metric Units (1 for Metric, 0 for Imperial)", "1", 2);
WiFiManagerParameter format_24h_param("format_24h", "24-Hour Format (1 for 24H, 0 for 12H)", "0", 2);
WiFiManagerParameter show_am_pm_param("show_am_pm", "Show AM/PM Indicator (1 for Yes, 0 for No)", "0", 2);
WiFiManagerParameter show_seconds_param("show_seconds", "Show Seconds on Clock (1 for Yes, 0 for No)", "1", 2);

void saveConfigCallback()
{
    shouldSaveConfig = true;
}

void saveConfigToFlash()
{
    preferences.begin("wifi-config", false);
    preferences.putString("timezone", timezone_param.getValue());
    preferences.putString("weather_loc", weather_loc_param.getValue());
    preferences.putString("stocks", stock_ticker_param.getValue());
    preferences.putString("units", weather_units_param.getValue());
    preferences.putString("format_24h", format_24h_param.getValue());
    preferences.putString("show_am_pm", show_am_pm_param.getValue());
    preferences.putString("show_seconds", show_seconds_param.getValue());
    preferences.end();
    Serial.println("Configuration saved to flash.");
}

void loadConfigFromFlash()
{
    preferences.begin("wifi-config", true);
    String timezone = preferences.getString("timezone", TIMEZONE_API_LOCATION);
    String weather_loc = preferences.getString("weather_loc", WEATHER_LOCAION);
    String stocks = preferences.getString("stocks", STOCK_TICKER_LIST);
    String units = preferences.getString("units", "1");
    String format_24h = preferences.getString("format_24h", "0");
    String show_am_pm = preferences.getString("show_am_pm", "0");
    String show_seconds = preferences.getString("show_seconds", "1");
    preferences.end();

    timezone_param.setValue(timezone.c_str(), timezone.length());
    weather_loc_param.setValue(weather_loc.c_str(), weather_loc.length());
    stock_ticker_param.setValue(stocks.c_str(), stocks.length());
    weather_units_param.setValue(units.c_str(), units.length());
    format_24h_param.setValue(format_24h.c_str(), format_24h.length());
    show_am_pm_param.setValue(show_am_pm.c_str(), show_am_pm.length());
    show_seconds_param.setValue(show_seconds.c_str(), show_seconds.length());

    Serial.println("Settings:");
    Serial.println("Timezone: " + String(timezone_param.getValue()));
    Serial.println("Weather Location: " + String(weather_loc_param.getValue()));
    Serial.println("Stock Ticker List: " + String(stock_ticker_param.getValue()));
    Serial.printf("Metric Units: %s\n", strcmp(weather_units_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("24-Hour Format: %s\n", strcmp(format_24h_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("Show AM/PM Indicator: %s\n", strcmp(show_am_pm_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("Show Seconds on Clock: %s\n", strcmp(show_seconds_param.getValue(), "1") == 0 ? "Yes" : "No");

    c_timezone = String(timezone_param.getValue());
    c_weather_loc = String(weather_loc_param.getValue());
    c_stock_ticker = String(stock_ticker_param.getValue());
    c_weather_units = weather_units_param.getValue();
    c_format_24h = format_24h_param.getValue();
    c_show_am_pm = show_am_pm_param.getValue();
    c_show_seconds = show_seconds_param.getValue();

    Serial.println("Configuration loaded from flash.");
}

WifiWidget::WifiWidget(ScreenManager &manager) : Widget(manager) {}

WifiWidget::~WifiWidget() {}

void WifiWidget::setup()
{
    TFT_eSPI &display = m_manager.getDisplay();
    m_manager.selectAllScreens();
    display.fillScreen(TFT_BLACK);
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE);

    m_manager.selectScreen(0);
    display.drawCentreString("Connecting" + m_connectionString, 120, 80, 1);

    m_manager.selectScreen(1);
    display.drawCentreString("Connecting to", 120, 80, 1);
    display.drawCentreString("WiFi..", 120, 100, 1);
    display.drawCentreString(WIFI_SSID, 120, 130, 1);

    // Load stored settings
    loadConfigFromFlash();

    // Add parameters to WiFiManager
    wm.addParameter(&timezone_param);
    wm.addParameter(&weather_loc_param);
    wm.addParameter(&stock_ticker_param);
    wm.addParameter(&weather_units_param);
    wm.addParameter(&format_24h_param);
    wm.addParameter(&show_am_pm_param);
    wm.addParameter(&show_seconds_param);

    // Set config save callback
    wm.setSaveConfigCallback(saveConfigCallback);
    delay(3000);

    if (digitalRead(CONFIG_PIN) == LOW)
    {
        Serial.println("Starting configuration portal...");
        wm.startConfigPortal("AutoConnectAP", "password");
        if (shouldSaveConfig)
        {
            saveConfigToFlash();
            Serial.println("Saving");
        }
    }
    else
    {
        res = wm.autoConnect("AutoConnectAP", "password"); // password protected AP

        if (!res)
        {
            Serial.println("Failed to connect");
            ESP.restart();
        }
        else
        {
            Serial.println("**************Connected to WiFi");
        }
    }
    // Retrieve and print parameter values for verification
    Serial.println("Settings:");
    Serial.println("Timezone: " + String(timezone_param.getValue()));
    Serial.println("Weather Location: " + String(weather_loc_param.getValue()));
    Serial.println("Stock Ticker List: " + String(stock_ticker_param.getValue()));
    Serial.printf("Metric Units: %s\n", strcmp(weather_units_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("24-Hour Format: %s\n", strcmp(format_24h_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("Show AM/PM Indicator: %s\n", strcmp(show_am_pm_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.printf("Show Seconds on Clock: %s\n", strcmp(show_seconds_param.getValue(), "1") == 0 ? "Yes" : "No");
    Serial.println("End Init");
}

void WifiWidget::update(bool force)
{
    // force is currently an unhandled due to not knowing what behavior it would change

    if (WiFi.status() == WL_CONNECTED)
    {
        m_isConnected = true;
        m_connectionString = "Connected";
    }
    else
    {
        m_connectionTimer += 500;
        m_dotsString += ".";
        Serial.print(".");
        if (m_dotsString.length() > 3)
        {
            m_dotsString = "";
        }
        if (m_connectionTimer > m_connectionTimeout)
        {
            m_connectionFailed = true;
            connectionTimedOut();
        }
    }
}

void WifiWidget::draw(bool force)
{
    // force is currently an unhandled due to not knowing what behavior it would change

    if (!m_isConnected && !m_connectionFailed)
    {
        TFT_eSPI &display = m_manager.getDisplay();
        m_manager.selectScreen(0);
        display.fillRect(0, 100, 240, 100, TFT_BLACK);
        display.drawCentreString(m_dotsString, 120, 100, 1);
    }
    else if (m_isConnected && !m_hasDisplayedSuccess)
    {
        m_hasDisplayedSuccess = true;
        TFT_eSPI &display = m_manager.getDisplay();
        m_manager.selectScreen(0);
        display.fillScreen(TFT_BLACK);
        display.drawCentreString("Connected", 120, 100, 1);
        Serial.println();
        Serial.println("Connected to WiFi");
        m_isConnected = true;
    }
    else if (m_connectionFailed && !m_hasDisplayedError)
    {
        m_hasDisplayedError = true;
        TFT_eSPI &display = m_manager.getDisplay();
        m_manager.selectScreen(0);
        display.drawCentreString("Connection", 120, 80, 1);
        display.fillRect(0, 100, 240, 100, TFT_BLACK);
        display.drawCentreString(m_connectionString, 120, 100, 1);
    }
}

void WifiWidget::changeMode() {}

void WifiWidget::connectionTimedOut()
{
    switch (WiFi.status())
    {
    case WL_CONNECTED:
        m_connectionString = "Connected";
        break;
    case WL_NO_SSID_AVAIL:
        m_connectionString = "No SSID available";
        break;
    case WL_CONNECT_FAILED:
        m_connectionString = "Connection failed";
        break;
    case WL_IDLE_STATUS:
        m_connectionString = "Idle status";
        break;
    case WL_DISCONNECTED:
        m_connectionString = "Disconnected";
        break;
    default:
        m_connectionString = "Unknown";
        break;
    }

    // Serial.println("Connection timed out");
    // TFT_eSPI &display = m_manager.getDisplay();
    // m_manager.selectScreen(0);
    // display.drawCentreString("Connection", 120, 80, 1);
    // display.drawCentreString(m_connectionString, 120, 100, 1);
}