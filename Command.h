#pragma once

struct Command {
	long frameNumber;
	std::string commandName;

	Command(long number, std::string name);
};

struct CommandLineArguments {
	std::vector<std::string> parameters;
	std::vector<std::string> options;

	CommandLineArguments(int argc, char **argv);
	~CommandLineArguments();
	bool isOption(std::string option);
	std::string getOptionValue(std::string option);
	std::vector<int> getOptionValueAsVector(std::string option);
};

