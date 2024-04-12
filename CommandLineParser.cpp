#include "CommandLineParser.h"

#define COMMAND_HEADER "> "

#define INPUT_CODE_CANCEL 0x3
#define INPUT_CODE_BACKSPACE 0x8
#define INPUT_CODE_TAB 0x9
#define INPUT_CODE_ESC 0x1B

#define ESCAPE_CODE_CLEAR "\033c"
#define ESCAPE_CODE_BACKSAPCE "\x7f"

#define DEBUG_SHOW_UNKNOWN_CODE 0

Vector<Command> CommandLineParser::commandList;

void CommandLineParser::init()
{
    install("clear", onCommandClearScreen, "clear\t: clear screen");
    install("help", onCommandHelp, "help\t: show available commands\r\nhelp [command]\t: show command's description");
}

bool CommandLineParser::install(String name, Callback callback, String description)
{
    trim(name);
    if (isSingleName(name) && !hasCommand(name))
    {
        trim(description);
        Command command{name, callback, description};
        commandList.push_back(command);
        return true;
    }
    return false;
}

void CommandLineParser::run()
{
    static String inputStr = "";
    static bool handled = true;
    static bool hasPrevTab = false;

    if (handled)
    {
        handled = false;
        hasPrevTab = false;
        inputStr = "";
        Serial.print(COMMAND_HEADER);
    }

    while (Serial.available())
    {
        char ch = Serial.read();
        if (ch == INPUT_CODE_CANCEL)
        {
            handled = true;
            Serial.println("^C");
            return;
        }
        else if (ch == INPUT_CODE_BACKSPACE)
        {
            if (inputStr.length() > 0)
            {
                inputStr.remove(inputStr.length() - 1);
                Serial.print(ESCAPE_CODE_BACKSAPCE);
            }
            return;
        }
        else if (ch == INPUT_CODE_TAB)
        {
            String name = inputStr;
            trim(name);
            if (isSingleName(name))
            {
                Vector<String> list = getCommandList(name);
                if (list.size() > 0)
                {
                    String substring = getCommonName(list);
                    if (substring.length() > 0)
                    {
                        int index = substring.indexOf(name);
                        if (index >= 0)
                        {
                            String suffix = substring.substring(name.length() + index);
                            if (suffix.length() > 0)
                            {
                                inputStr += suffix;
                                Serial.print(suffix);
                            }
                            else if (hasPrevTab)
                            {
                                Serial.println();
                                Serial.print(" " + vector2String(list));
                                Serial.println();
                                Serial.print(COMMAND_HEADER + inputStr);
                            }
                        }
                    }
                }
            }
            hasPrevTab = true;
            return;
        }
        else if (ch == INPUT_CODE_ESC)
        {
            delay(1);
            if (Serial.peek() == '[')
            {
                Serial.read();
                delay(1);
                char ch = Serial.peek();
                // Arrow keys
                if (ch == 'D' || ch == 'C' || ch == 'A' || ch == 'B')
                {
                    Serial.read();
                }
            }
            return;
        }
        else if (isSeparatorCharacter(ch))
        {
            handled = true;
            Serial.println();
            break;
        }
        else if (isAcceptedCharacter(ch))
        {
            inputStr += ch;
            Serial.print(ch);
        }
#if DEBUG_SHOW_UNKNOWN_CODE
        else
        {
            Serial.print("{0x");
            Serial.print(ch, HEX);
            Serial.print("}");
        }
#endif
        // delay(1);
    }

    if (!handled)
    {
        return;
    }

    trim(inputStr);
    if (inputStr.length() == 0)
    {
        return;
    }

    String commandName = "", commandParams = "";
    int spaceIdx = inputStr.indexOf(' ');
    if (spaceIdx > 0 && spaceIdx < inputStr.length() - 1)
    {
        commandName = inputStr.substring(0, spaceIdx);
        commandParams = inputStr.substring(spaceIdx + 1);
        trim(commandName);
        trim(commandParams);
    }
    else
    {
        commandName = inputStr;
        commandParams = "";
    }

    for (int i = 0; i < commandList.size(); i++)
    {
        Command command = commandList.at(i);
        if (command.name.equals(commandName))
        {
            if (!command.callback(commandParams))
            {
                Serial.println("Invalid usage of command '" + command.name + "'");
                Serial.println("Show command's description: help " + command.name);
            }
            return;
        }
    }
    Serial.println("Unknown command '" + commandName + "'");
}

bool CommandLineParser::isSeparatorCharacter(char ch)
{
    return ch == '\n' || ch == '\r';
}

bool CommandLineParser::isAcceptedCharacter(char ch)
{
    return ch >= ' ' && ch <= '~';
}

void CommandLineParser::trim(String &str)
{
    while (str.length() > 0 && (str.indexOf(' ') == 0 || str.indexOf('\r') == 0 || str.indexOf('\n') == 0))
    {
        str = str.substring(1);
    }
    while (str.length() > 0 && (str.lastIndexOf(' ') == str.length() - 1 || str.lastIndexOf('\r') == str.length() - 1 || str.lastIndexOf('\n') == str.length() - 1))
    {
        str = str.substring(0, str.length() - 1);
    }
}

bool CommandLineParser::isSingleName(const String &name)
{
    return name.indexOf(' ') < 0;
}

Vector<String> CommandLineParser::getCommandList(String prefix)
{
    Vector<String> result;
    for (int i = 0; i < commandList.size(); i++)
    {
        if (prefix.length() == 0 || commandList.at(i).name.startsWith(prefix))
        {
            result.push_back(commandList.at(i).name);
        }
    }
    return result;
}

String CommandLineParser::vector2String(const Vector<String> &Vector)
{
    String result = "";
    for (int i = 0; i < Vector.size(); i++)
    {
        result += Vector.at(i);
        if (i != Vector.size() - 1)
        {
            result += " ";
        }
    }
    return result;
}

String CommandLineParser::getCommonName(const Vector<String> &nameList)
{
    if (nameList.size() == 0)
    {
        return "";
    }
    if (nameList.size() == 1)
    {
        return nameList.at(0);
    }

    int keyIndex = 0;
    for (int i = 0; i < nameList.size(); i++)
    {
        if (nameList.at(i).length() < nameList.at(keyIndex).length())
        {
            keyIndex = i;
        }
    }

    String result = "";
    for (int i = 0; i < nameList.at(keyIndex).length(); i++)
    {
        for (int j = 0; j < nameList.size(); j++)
        {
            if (nameList.at(j).charAt(i) != nameList.at(0).charAt(i))
            {
                return result;
            }
        }
        result += nameList.at(keyIndex).charAt(i);
    }
    return result;
}

bool CommandLineParser::hasCommand(const String &name)
{
    for (int i = 0; i < commandList.size(); i++)
    {
        if (commandList.at(i).name.equals(name))
        {
            return true;
        }
    }
    return false;
}

bool CommandLineParser::getCommandByName(Command &command, String name)
{
    for (int i = 0; i < commandList.size(); i++)
    {
        if (commandList.at(i).name.equals(name))
        {
            command = commandList.at(i);
            return true;
        }
    }
    return false;
}

bool CommandLineParser::onCommandClearScreen(String params)
{
    if (params.length() > 0)
    {
        return false;
    }
    Serial.print(ESCAPE_CODE_CLEAR);
    return true;
}

bool CommandLineParser::onCommandHelp(String commandName)
{
    if (!isSingleName(commandName))
    {
        return false;
    }

    if (commandName.length() == 0)
    {
        Serial.print(F("Available commands: "));
        Serial.println(vector2String(getCommandList()));
        Serial.println(F("Show command's description: help [command]"));
        return true;
    }

    Command command;
    if (!getCommandByName(command, commandName))
    {
        Serial.println("Unknown command: " + commandName);
        return true;
    }
    if (command.description.length() == 0)
    {
        Serial.println("Command '" + commandName + "' has no description");
        return true;
    }
    Serial.println(command.description);
    return true;
}