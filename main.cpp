#include "parser.h"

#include <iostream>

struct tCommonOpts {
	int m_sHelp;
} g_tCommonOpts;

struct tCheckOpts {
	std::vector<std::string> m_dNames;
} g_tCheckOpts;

int main(int argc, char** argv) {
	Parser parser = Parser("sample");

	parser.AddOption(g_tCommonOpts.m_sHelp, "--help")
		.AddShortcut("-h")
		.AddDescription("Help option");

	auto check_cmd = parser.AddCommand("check");
	check_cmd.AddOption(g_tCheckOpts.m_dNames, "--index")
		.AddShortcut("-i")
		.AddDescription("list indexes");

	parser.PrintHelp(std::cout);

	int iLastToken = parser.Parse(argc, argv);
	if (iLastToken != argc)
	{
		std::cerr << "Error gets near '" << argv[iLastToken] << "'\n";
		exit(1);
	}

	return 0;
}