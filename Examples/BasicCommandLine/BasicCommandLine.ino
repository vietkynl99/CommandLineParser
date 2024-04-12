// Available commands:
// help : show available commands
// clear : clear screen
// test : run testFunc function

#include <CommandLineParser.h>

bool testFunc(String params)
{
	Serial.println("testFunc: " + params);
	return true;
}

void setup()
{
	delay(1000);
	Serial.begin(115200);

	CommandLineParser::init();
	CommandLineParser::install("test", testFunc);
}

void loop()
{
	CommandLineParser::run();
}
