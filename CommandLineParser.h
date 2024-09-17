#ifndef _COMMAND_LINE_PARSER_H_
#define _COMMAND_LINE_PARSER_H_

#include <Arduino.h>
#include <VVector.h>

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
    static Stream *mStream;
    static VVector<Command> commandList;

public:
    static void init(Stream *stream = nullptr);
    static bool install(String name, Callback callback, String description = "");
    static void run();
    static String process(Stream *stream, String &inputStr);

private:
    static bool isSeparatorCharacter(char ch);
    static bool isAcceptedCharacter(char ch);
    static void trim(String &str);
    static bool isSingleName(const String &name);

    static VVector<String> getCommandList(String prefix = "");
    static String vector2String(VVector<String> &vector);
    static String getCommonName(VVector<String> &nameList);
    static bool hasCommand(const String &name);
    static bool getCommandByName(Command &command, String name);

    static bool onCommandClearScreen(String params);
    static bool onCommandHelp(String commandName);
};
#endif