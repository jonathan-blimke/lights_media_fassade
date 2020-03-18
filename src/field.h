/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

typedef String (*FieldSetter)(String);
typedef String (*FieldGetter)();

const String NumberFieldType = "Number";
const String BooleanFieldType = "Boolean";
const String TextFieldType ="Text";
const String ArrayFieldType  ="Array";
const String ColorFieldType ="Color";

typedef struct Field
{
public:
  String name;
  String label;
  String type;
  uint8_t min;
  uint8_t max;
  FieldGetter getValue;
  FieldGetter getOptions;
  FieldSetter setValue;
};

typedef Field FieldList[];

Field getField(String name, FieldList fields, uint8_t count) {
  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (field.name == name)
    {
      return field;
    }
  }
  return Field();
}

String getFieldValue(String name, FieldList fields, uint8_t count) {
  Field field = getField(name, fields, count);
  if (field.getValue)
  {
    return field.getValue();
  }
  return String();
}

void writeFieldsToEEPROM(FieldList fields, uint8_t count) {
  uint8_t index = 0;

  EEPROM.write(index, 0);

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (!field.getValue && !field.setValue)
      continue;

    String value = field.getValue();
    byte v = value.toInt();
    EEPROM.write(index++, v);
  }
  EEPROM.commit();
}

String setFieldValue(String name, String value, FieldList fields, uint8_t count) {
  Field field = getField(name, fields, count);
  if (!field.setValue) {
    return "";
  }
  String result = field.setValue(value);

  String json = "{\"name\":\"" + name + "\",\"value\":" + result + "}";
  webSocketsServer.broadcastTXT(json);

  writeFieldsToEEPROM(fields, count);

  return result;
}

void loadFieldsFromEEPROM(FieldList fields, uint8_t count) {
  uint8_t byteCount = 1;

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (!field.setValue)
      continue;

    if (field.type == ColorFieldType)
    {
      byteCount += 3;
    }
    else
    {
      byteCount++;
    }
  }

  EEPROM.begin(count);
 
  if (EEPROM.read(0) == 255)
  {
    Serial.println("First run, or EEPROM erased, skipping settings load!");
    return;
  }

  uint8_t index = 0;

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];
    if (!field.setValue)
      continue;

    if (field.type == ColorFieldType)
    {
      String r = String(EEPROM.read(index++));
      String g = String(EEPROM.read(index++));
      String b = String(EEPROM.read(index++));
      field.setValue(r + "," + g + "," + b);
    }
    else
    {
      byte v = EEPROM.read(index++);
      field.setValue(String(v));
    }
  }
}

// returns field list as json
String getFieldsJson(FieldList fields, uint8_t count) {
  String json = "[";

  for (uint8_t i = 0; i < count; i++)
  {
    Field field = fields[i];

    json += "{\"name\":\"" + field.name + "\",\"label\":\"" + field.label + "\",\"type\":\"" + field.type + "\"";

    if (field.getValue)
    {
      if (field.type == ColorFieldType || field.type == "String")
      {
        json += ",\"value\":\"" + field.getValue() + "\"";
      }
      else
      {
        json += ",\"value\":" + field.getValue();
      }
    }

    if (field.type == NumberFieldType)
    {
      json += ",\"min\":" + String(field.min);
      json += ",\"max\":" + String(field.max);
    }

    if (field.getOptions)
    {
      json += ",\"options\":[";
      json += field.getOptions();
      json += "]";
    }

    json += "}";

    if (i < count - 1)
      json += ",";
  }

  json += "]";

  return json;
}
