#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include "program_options_lite.h"

using namespace std;

namespace df {
namespace program_options_lite {

Options::~Options()
{
	for(Options::NamesPtrList::iterator it = opt_list.begin(); it != opt_list.end(); it++) {
		delete *it;
	}
}

void Options::addOption(OptionBase *opt)
{
	Names* names = new Names();
	names->opt = opt;
	string& opt_string = opt->opt_string;

	size_t opt_start = 0;
	for (size_t opt_end = 0; opt_end != string::npos;) {
		opt_end = opt_string.find_first_of(',', opt_start);
		bool force_short = 0;
		if (opt_string[opt_start] == '-') {
			opt_start++;
			force_short = 1;
		}
		string opt_name = opt_string.substr(opt_start, opt_end - opt_start);
		if (force_short || opt_name.size() == 1) {
			names->opt_short.push_back(opt_name);
			opt_short_map[opt_name].push_back(names);
		}
		else {
			names->opt_long.push_back(opt_name);
			opt_long_map[opt_name].push_back(names);
		}
		opt_start += opt_end + 1;
	}
	opt_list.push_back(names);
}

/* Helper method to initiate adding options to Options */
OptionSpecific Options::addOptions() {
	return OptionSpecific(*this);
}

static void setOptions(Options::NamesPtrList& opt_list, const string& value)
{
	/* multiple options may be registered for the same name:
	 *   allow each to parse value */
	for (Options::NamesPtrList::iterator it = opt_list.begin(); it != opt_list.end(); ++it) {
		(*it)->opt->parse(value);
	}
}

/* format help text for a single option:
 * using the formatting: "-x, --long",
 * if a short/long option isn't specified, it is not printed
 */
static void doHelpOpt(ostream& out, const Options::Names& entry)
{
	if (!entry.opt_short.empty()) {
		out << "-" << entry.opt_short.front();
		if (!entry.opt_long.empty()) {
			out << ", ";
		}
	}
	else {
		out << "    ";
	}

	if (!entry.opt_long.empty()) {
		out << "--" << entry.opt_long.front();
	}
}

/* format the help text */
void doHelp(ostream& out, Options& opts)
{
	/* first pass: work out the longest option name */
	unsigned max_width = 0;
	for(Options::NamesPtrList::iterator it = opts.opt_list.begin(); it != opts.opt_list.end(); it++) {
		ostringstream line(ios_base::out);
		doHelpOpt(line, **it);
		max_width = max(max_width, (unsigned) line.tellp());
	}

	unsigned opt_width = min(max_width+2, 28u) + 2;

	for(Options::NamesPtrList::iterator it = opts.opt_list.begin(); it != opts.opt_list.end(); it++) {
		ostringstream line(ios_base::out);
		line << "  ";
		doHelpOpt(line, **it);
		size_t currlength = line.tellp();
		if (currlength > opt_width) {
			/* if option was too long, split onto next line */
			line << endl;
			line << &("                              "[30 - opt_width]);
		}
		else {
			line << &("                              "[30 - opt_width + currlength]);
		}
		line << (*it)->opt->opt_desc;
		cout << line.str() << endl;
	}
}

bool storePair(Options& opts, bool allow_long, bool allow_short, const string& name, const string& value)
{
	bool found = false;
	Options::NamesMap::iterator opt_it;
	if (allow_long) {
		opt_it = opts.opt_long_map.find(name);
		if (opt_it != opts.opt_long_map.end()) {
			found = true;
		}
	}

	/* check for the short list */
	if (allow_short && !(found && allow_long)) {
		opt_it = opts.opt_short_map.find(name);
		if (opt_it != opts.opt_short_map.end()) {
			found = true;
		}
	}

	if (!found) {
		/* not found */
		cerr << "Unknown option: `" << name << "' (value:`" << value << "')" << endl;
		return false;
	}

	setOptions((*opt_it).second, value);
	return true;
}

bool storePair(Options& opts, const string& name, const string& value)
{
	return storePair(opts, true, true, name, value);
}

/**
 * returns number of extra arguments consumed
 */
unsigned parseGNU(Options& opts, unsigned argc, const char* argv[])
{
	/* gnu style long options can take the forms:
	 *  --option=arg
	 *  --option arg
	 */
	string arg(argv[0]);
	size_t arg_opt_start = arg.find_first_not_of('-');
	size_t arg_opt_sep = arg.find_first_of('=');
	string option = arg.substr(arg_opt_start, arg_opt_sep - arg_opt_start);

	unsigned extra_argc_consumed = 0;
	if (arg_opt_sep == string::npos) {
		/* no argument found => argument in argv[1] (maybe) */
		/* xxx, need to handle case where option isn't required */
#if 0
		/* commented out, to return to true GNU style processing
		 * where longopts have to include an =, otherwise they are
		 * booleans */
		if (argc == 1)
			return 0; /* run out of argv for argument */
		extra_argc_consumed = 1;
#endif
		if(!storePair(opts, true, false, option, "1")) {
			return 0;
		}
	}
	else {
		/* argument occurs after option_sep */
		string val = arg.substr(arg_opt_sep + 1);
		storePair(opts, true, false, option, val);
	}

	return extra_argc_consumed;
}

unsigned parseSHORT(Options& opts, unsigned argc, const char* argv[])
{
	/* short options can take the forms:
	 *  --option arg
	 *  -option arg
	 */
	string arg(argv[0]);
	size_t arg_opt_start = arg.find_first_not_of('-');
	string option = arg.substr(arg_opt_start);
	/* lookup option */

	/* argument in argv[1] */
	/* xxx, need to handle case where option isn't required */
	if (argc == 1)
		return 0; /* run out of argv for argument */
	storePair(opts, false, true, option, string(argv[1]));

	return 1;
}

void scanArgv(Options& opts, unsigned argc, const char* argv[])
{
	for(unsigned i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			i += parseGNU(opts, argc - i, &argv[i]);
			continue;
		}
		if (argv[i][0] == '-') {
#if 0
			i += parsePOSIX(opts, argc - i, &argv[i]);
#else
			i += parseSHORT(opts, argc - i, &argv[i]);
#endif
			continue;
		}
	}
}

void scanLine(Options& opts, string& line)
{
	/* strip any leading whitespace */
	size_t start = line.find_first_not_of(" \t\n\r");
	if (start == string::npos) {
		/* blank line */
		return;
	}
	if (line[start] == '#') {
		/* comment line */
		return;
	}
	/* look for first whitespace or ':' after the option end */
	size_t option_end = line.find_first_of(": \t\n\r",start);
	string option = line.substr(start, option_end - start);

	/* look for ':', eat up any whitespace first */
	start = line.find_first_not_of(" \t\n\r", option_end);
	if (start == string::npos) {
		/* error: badly formatted line */
		return;
	}
	if (line[start] != ':') {
		/* error: badly formatted line */
		return;
	}

	/* look for start of value string -- eat up any leading whitespace */
	start = line.find_first_not_of(" \t\n\r", ++start);
	if (start == string::npos) {
		/* error: badly formatted line */
		return;
	}

	/* extract the value part, which may contain embedded spaces
	 * by searching for a word at a time, until we hit a comment or end of line */
	size_t value_end = start;
	do {
		if (line[value_end] == '#') {
			/* rest of line is a comment */
			value_end--;
			break;
		}
		value_end = line.find_first_of(" \t\n\r", value_end);
		/* consume any white space, incase there is another word.
		 * any trailing whitespace will be removed shortly */
		value_end = line.find_first_not_of(" \t\n\r", value_end);
	} while (value_end != string::npos);
	/* strip any trailing space from value*/
	value_end = line.find_last_not_of(" \t\n\r", value_end);

	string value;
	if (value_end >= start) {
		value = line.substr(start, value_end +1 - start);
	}
	else {
		/* error: no value */
		return;
	}

	/* store the value in option */
	storePair(opts, true, false, option, value);
}

void scanFile(Options& opts, istream& in)
{
	do {
		string line;
		getline(in, line);
		scanLine(opts, line);
	} while(!!in);
}

/* for all options in @opts@, set their storage to their specified
 * default value */
void setDefaults(Options& opts)
{
	for(Options::NamesPtrList::iterator it = opts.opt_list.begin(); it != opts.opt_list.end(); it++) {
		(*it)->opt->setDefault();
	}
}

void parseConfigFile(Options& opts, const string& filename)
{
	ifstream cfgstream(filename.c_str(), ifstream::in);
	if (!cfgstream) {
		cerr << "Failed to open config file: `" << filename << "'" << endl;
		exit(EXIT_FAILURE);
	}
	scanFile(opts, cfgstream);
}

};
};
