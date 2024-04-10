#include "CommandLineParser.h"

#define COMMAND_HEADER "> "

#define INPUT_CODE_CANCEL 0x3
#define INPUT_CODE_BACKSPACE 0x8
#define INPUT_CODE_TAB 0x9

#define ESCAPE_CODE_CLEAR "\033c"
#define ESCAPE_CODE_BACKSAPCE "\x7f"

#define DEBUG_SHOW_UNKNOWN_CODE 0

typedef struct
{
    String name;
    CommandLineParser::Callback callback;
} Command;

Vector<Command> commandList;

void CommandLineParser::init()
{
    install("clear", onCommandClearScreen);
    install("help", onCommandHelp);
}

bool CommandLineParser::install(String name, Callback callback)
{
    trim(name);
    if (name.indexOf(' ') < 0 && !hasCommand(name))
    {
        Command command{name, callback};
        commandList.push_back(command);
        return true;
    }
    return false;
}

void CommandLineParser::run()
{
    static String command = "", params = "";
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
            if (name.indexOf(' ') < 0)
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

    int spaceIdx = inputStr.indexOf(' ');
    if (spaceIdx > 0 && spaceIdx < inputStr.length() - 1)
    {
        command = inputStr.substring(0, spaceIdx);
        params = inputStr.substring(spaceIdx + 1);
        trim(command);
        trim(params);
    }
    else
    {
        command = inputStr;
        params = "";
    }

    for (int i = 0; i < commandList.size(); i++)
    {
        if (commandList.at(i).name.equals(command))
        {
            commandList.at(i).callback(params);
            return;
        }
    }
    Serial.println("Unknown command '" + command + "'");
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

void CommandLineParser::onCommandClearScreen(String params)
{
    Serial.print(ESCAPE_CODE_CLEAR);
}

void CommandLineParser::onCommandHelp(String params)
{
    Serial.println(vector2String(getCommandList()));
}