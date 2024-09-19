#ifndef WEB_DATA_ELEMENT_CHARACTER_MODEL_H
#define WEB_DATA_ELEMENT_CHARACTER_MODEL_H

#include "webDataElement.h"
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "utils.h"

class WebDataElementCharacterModel: public WebDataElement {
   public:
    int32_t getX();
    void setX(int32_t x);
    int32_t getY();
    void setY(int32_t y);

    String getCharacter();
    void setCharacter(String character);

    int32_t getFont();
    void setFont(int32_t font);

    int32_t getBackgroundColor();
    void setBackgroundColor(int32_t background);
    void setBackgroundColor(String background);

    void setColor(int32_t color);
    void setColor(String color);
    int32_t getColor();

    void parseData(const JsonObject& doc, int32_t defaultColor, int32_t defaultBackground) override;
    void draw(TFT_eSPI& display) override;

   private:
    int32_t m_x = 0;
    int32_t m_y = 0;
    String m_character;
    int32_t m_font = 2;
    int32_t m_color = -1;
    int32_t m_background = -1;
};
#endif
