#include "Command.h"
#include "utils.h"

Command::Command(long number, std::string name):
	frameNumber(number),
	commandName(name)
{
}

CommandLineArguments::CommandLineArguments(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		std::string parameter(argv[i]);
		int equalSignIndex = parameter.find("=");
		std::string option = parameter.substr(2, equalSignIndex - 2);
		std::string value = parameter.substr(equalSignIndex + 1, parameter.length() - equalSignIndex - 1);
		options.push_back(option);
		parameters.push_back(value);
	}
}
CommandLineArguments::~CommandLineArguments() {
	options = std::vector<std::string>();
	parameters = std::vector<std::string>();
}

bool CommandLineArguments::isOption(std::string option) {
	xForEach(iter, options) {
		if (*iter == option) {
			return true;
		}
	}

	return false;
}

std::string CommandLineArguments::getOptionValue(std::string option) {
	for (int i = 0; i < options.size(); i++) {
		if (options[i] == option) {
			return parameters[i];
		}
	}

	return "";
}

std::vector<int> CommandLineArguments::getOptionValueAsVector(std::string option) {
	std::vector<int> returnVector;

	for (int i = 0; i < options.size(); i++) {
		if (options[i] == option) {
			std::string input = parameters[i];
			std::istringstream ss(input);
			std::string token;

			while(std::getline(ss, token, ',')) {
				returnVector.push_back(atoi(token.c_str()));
				std::cout << "PARSED: " << token << '\n';
			}

			return returnVector;
		}
	}

	return returnVector;
}

