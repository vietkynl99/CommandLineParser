#ifndef _COMMAND_LINE_PARSER_H_
#define _COMMAND_LINE_PARSER_H_

#include <Arduino.h>
#include <ArduinoVector.h>

class CommandLineParser
{
public:
    typedef void (*Callback)(String);

public:
    static void init();
    static bool install(String name, Callback callback);
    static void run();


private:
    static bool isSeparatorCharacter(char ch);
    static bool isAcceptedCharacter(char ch);
    static void trim(String &str);

    static Vector<String> getCommandList(String prefix = "");
    static String vector2String(const Vector<String> &Vector);
    static String getCommonName(const Vector<String> &nameList);
    static bool hasCommand(const String &name);

    static void onCommandClearScreen(String params);
    static void onCommandHelp(String params);
};
#endif