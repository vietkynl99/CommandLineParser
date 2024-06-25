#ifndef _COMMAND_LINE_PARSER_H_
#define _COMMAND_LINE_PARSER_H_

#include <Arduino.h>
#include <ArduinoVector.h>

typedef bool (*Callback)(String);
typedef struct
{
    String name;
    Callback callback;
    String description;
} Command;

class CommandLineParser
{
private:
    static Vector<Command> commandList;

public:
    static void init();
    static bool install(String name, Callback callback, String description = "");
    static void run();

private:
    static bool isSeparatorCharacter(char ch);
    static bool isAcceptedCharacter(char ch);
    static void trim(String &str);
    static bool isSingleName(const String &name);

    static Vector<String> getCommandList(String prefix = "");
    static String vector2String(Vector<String> &vector);
    static String getCommonName(Vector<String> &nameList);
    static bool hasCommand(const String &name);
    static bool getCommandByName(Command &command, String name);

    static bool onCommandClearScreen(String params);
    static bool onCommandHelp(String commandName);
};
#endif