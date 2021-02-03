#pragma once

#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <ostream>

class IOption;
class Command;
class Parser;

struct IOption
{
	IOption() = default;

	IOption(const std::string& fullname, const std::function<void()>& callback)
	{
	}

	virtual int Proceed(int argc, char** argv) = 0;

	std::optional<std::string> m_sFullname;
	std::optional<std::string> m_sShortcut;
	std::optional<std::string> m_sDescription;
};

template <typename T>
int ProceedArgs(T& value, int argc, char** argv)
{
	throw std::runtime_error("undefined type");
}

template <>
int ProceedArgs(int& value, int argc, char** argv)
{
	assert(argc != 0);
	value = static_cast<int>(std::strtol(argv[0], NULL, 10));
	return 1;
}

template <>
int ProceedArgs(std::vector<std::string>& value, int argc, char** argv)
{
	int i = 0;
	for (; i < argc; ++i)
		// TODO: fix this :)
		if (argv[i][0] != '-')
			value.emplace_back(argv[i]);
		else
			break;

	return i;
}

template<typename T>
class Option : public IOption
{
public:
	Option(T& result, const std::string& fullname)
		: value(result)
	{
		m_sFullname = fullname;
	}

	Option(T& result, const char * fullname)
		: value(result)
	{
		m_sFullname = fullname;
	}

	Option& AddShortcut(const std::string& shortcut) {
		m_sShortcut = shortcut;
		return *this;
	}

	Option& AddDescription(const std::string& description) {
		m_sDescription = description;
		return *this;
	}

private:
	virtual int Proceed(int argc, char** argv)
	{
		return ProceedArgs<T>(value, argc, argv);
	}

private:
	T& value;
};

class Command
{
public:
	friend Parser;

public:
	explicit Command(std::string name)
		: name(std::move(name))
	{}

	template <typename T, typename ...Args>
	Option<T>& AddOption(T& value, Args... args)
	{
		m_dOptions.push_back(std::make_unique<Option<T>>(value, args...));
		return *dynamic_cast<Option<T>*>(m_dOptions.back().get());
	}

protected:
	inline bool MatchOption(const std::unique_ptr<IOption> & pOption, const char * sToken) const
	{
		if (pOption->m_sFullname.has_value() && !strcmp(pOption->m_sFullname.value().c_str(), sToken))
			return true;

		return pOption->m_sShortcut.has_value() && !strcmp(pOption->m_sShortcut.value().c_str(), sToken);
	}

	int ParseOptions(int argc, char** argv) const
	{
		int i = 0;
		for (; i < argc; ++i)
			for (const auto& option : m_dOptions)
				if (MatchOption(option, argv[i]))
				{
					i++;
					i += option->Proceed(argc - i, argv + i);
				}


		return i;
	}

	void PrintDescription() const
	{
		// TODO: add description
	}

protected:
	std::string name;
	std::vector<std::unique_ptr<IOption>> m_dOptions;
};

class Parser : public Command
{
public:
	explicit Parser(std::string name)
		: Command(std::move(name))
	{}

	Command& AddCommand(std::string name)
	{
		m_dCommands.emplace_back(std::move(name));
		return m_dCommands.back();
	}

	int Parse(int argc, char** argv)
	{
		int total = ParseOptions(argc, argv);

		// it may be command
		if (total != argc)
			for (const auto & tCommand : m_dCommands)
				if (!strcmp(tCommand.name.c_str(), argv[total]))
				{
					total++;
					total += tCommand.ParseOptions(argc - total, argv + total);
				}

		return total;
	}

	void PrintHelp(std::ostream& out)
	{
		out << "usage: " << name << "<options> [command] <command-options>";
		for (const auto & tCommand : m_dCommands)
			tCommand.PrintDescription();
	}

private:
	std::vector<Command> m_dCommands;
};
