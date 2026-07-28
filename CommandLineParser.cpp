#include "CommandLineParser.h"

#define COMMAND_HEADER "> "

#define INPUT_CODE_CANCEL 0x3
#define INPUT_CODE_BACKSPACE 0x8
#define INPUT_CODE_TAB 0x9
#define INPUT_CODE_ESC 0x1B

#define ESCAPE_CODE_CLEAR "\033c"
#define ESCAPE_CODE_BACKSAPCE "\x7f"

#define DEBUG_SHOW_UNKNOWN_CODE 0

Stream *CommandLineParser::mStream = &Serial;
VVector<Command> CommandLineParser::commandList;
String CommandLineParser::mInputStr = "";
bool CommandLineParser::mHandled = true;
bool CommandLineParser::mHasPrevTab = false;

void CommandLineParser::init(Stream *stream)
{
    if (stream)
    {
        mStream = stream;
    }
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
    while (mStream->available())
    {
        handleByte(mStream->read());
    }
}

void CommandLineParser::handleByte(char ch)
{
    if (mHandled)
    {
        mHandled = false;
        mHasPrevTab = false;
        mInputStr = "";
        mStream->print(COMMAND_HEADER);
    }

    if (ch == INPUT_CODE_CANCEL)
    {
        mHandled = true;
        mStream->println("^C");
        return;
    }
    else if (ch == INPUT_CODE_BACKSPACE)
    {
        if (mInputStr.length() > 0)
        {
            mInputStr.remove(mInputStr.length() - 1);
            mStream->print(ESCAPE_CODE_BACKSAPCE);
        }
        return;
    }
    else if (ch == INPUT_CODE_TAB)
    {
        String name = mInputStr;
        bool endWithSpace = name[name.length() - 1] == ' ';
        trim(name);
        if (isSingleName(name))
        {
            VVector<String> list = getCommandList(name);
            if (list.size() > 0 && !(list.size() == 1 && name.equals(list.at(0))))
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
                            mInputStr += suffix;
                            mStream->print(suffix);
                            if (list.size() == 1 && !endWithSpace)
                            {
                                mInputStr += ' ';
                                mStream->print(' ');
                            }
                        }
                        else if (mHasPrevTab)
                        {
                            mStream->println();
                            mStream->print(" " + vector2String(list));
                            mStream->println();
                            mStream->print(COMMAND_HEADER + mInputStr);
                        }
                    }
                }
            }
        }
        mHasPrevTab = true;
        return;
    }
    else if (ch == INPUT_CODE_ESC)
    {
        delay(1);
        if (mStream->peek() == '[')
        {
            mStream->read();
            delay(1);
            char ch2 = mStream->peek();
            // Arrow keys
            if (ch2 == 'D' || ch2 == 'C' || ch2 == 'A' || ch2 == 'B')
            {
                mStream->read();
            }
        }
        return;
    }
    else if (isSeparatorCharacter(ch))
    {
        mHandled = true;
        mStream->println();
        process(mStream, mInputStr);
        return;
    }
    else if (isAcceptedCharacter(ch))
    {
        mInputStr += ch;
        mStream->print(ch);
    }
#if DEBUG_SHOW_UNKNOWN_CODE
    else
    {
        mStream->print("{0x");
        mStream->print(ch, HEX);
        mStream->print("}");
    }
#endif
}

String CommandLineParser::process(Stream *stream, String &inputStr)
{
    String message = "";
    trim(inputStr);
    if (inputStr.length() == 0)
    {
        return message;
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
                message = "Invalid usage of command '" + command.name + "'\nShow command's description: help " + command.name;
                if (stream)
                {
                    stream->println(message);
                }
            }
            return message;
        }
    }
    message = "Unknown command '" + commandName + "'";
    if (stream)
    {
        stream->println(message);
    }
    return message;
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

VVector<String> CommandLineParser::getCommandList(String prefix)
{
    VVector<String> result;
    for (int i = 0; i < commandList.size(); i++)
    {
        if (prefix.length() == 0 || commandList.at(i).name.startsWith(prefix))
        {
            result.push_back(commandList.at(i).name);
        }
    }
    return result;
}

String CommandLineParser::vector2String(VVector<String> &vector)
{
    String result = "";
    for (int i = 0; i < vector.size(); i++)
    {
        result += vector.at(i);
        if (i != vector.size() - 1)
        {
            result += ", ";
        }
    }
    return result;
}

String CommandLineParser::getCommonName(VVector<String> &nameList)
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
    mStream->print(ESCAPE_CODE_CLEAR);
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
        VVector<String> commands = getCommandList();
        mStream->print(F("Available commands: "));
        mStream->println(vector2String(commands));
        mStream->println(F("Show command's description: help [command]"));
        return true;
    }

    Command command;
    if (!getCommandByName(command, commandName))
    {
        mStream->println("Unknown command: " + commandName);
        return true;
    }
    if (command.description.length() == 0)
    {
        mStream->println("Command '" + commandName + "' has no description");
        return true;
    }
    mStream->println(command.description);
    return true;
}