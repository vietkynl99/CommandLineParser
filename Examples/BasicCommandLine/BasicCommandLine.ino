// Available commands:
// help : show available commands
// clear : clear screen
// test : run testFunc function

#include <CommandLineParser.h>

void testFunc(String params)
{
	Serial.println("testFunc: " + params);
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
