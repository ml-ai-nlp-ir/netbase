#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <string.h>
#include <vector>
#include <string>

#include "webserver.hpp"

#include "netbase.hpp"
#include "util.hpp"
#include "export.hpp"
#include "import.hpp"
#include "tests.hpp"
#include "query.hpp"
#include "init.hpp"

//#define USE_READLINE
// compile with -lreadline !
#ifdef USE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

using namespace std;
// static struct termios stored_settings;

void showHelp() {
	ps("available commands");
	ps("help :h or ?");
	ps("save :s or :w");
	ps("load :l [force]\tfetch the graph from disk or mem");
//	ps("load_files :lf");
	ps("import :i [<file>]");
	ps("export :e (all to csv)");
	//    ps("print :p");
	ps("delete :d <node|statement|id|name>");
//	ps("save and exit :x");
	ps("quit :q");
	ps("clear :cl");
	ps("limit <n>");
	ps("Type words, ids, or queries:");
	//    ps("all animals that have feathers");
	ps("all animals with feathers");
	ps("select * from dogs where terrier");
	ps("all city with countrycode ZW");
	ps("Population of Gehren");
}

//bool parse(string* data) {
static char* lastCommand;

Node *parseProperty(const char *data) {
	char *thing = (char *) malloc(1000);
	char *property = (char *) malloc(1000);
	if (contains(data, " of "))
		sscanf(data, "%s of %s", property, thing);
	else {
		//			sscanf(data,"%s.%s",thing,property);
		char** splat = splitStringC(data, '.');
		thing = splat[0];
		property = splat[1];
	}
	Node* found = has(getThe(thing), getAbstract(property));
	if (found == 0)
		found = has(getAbstract(thing), getAbstract(property));
	show(found);
}

bool parse(const char* data) {
	if (eq(data, null)) {
		return true;
	}
	if (eq(data, "")) {
		return true;
	}

	string arg = next(string(data));
	vector<char*> args = splitString(data, " "); // WITH 0==cmd!!!

	//		scanf ( "%s", data );
	if (eq(data, "exit"))return false;
	if (eq(data, "help") || eq(data, "?")) {
		showHelp();
		//        printf("type exit or word");
		return true;
	}

	if (eq(data, "more") && lastCommand) {
		defaultLimit *= 2;
		return parse(lastCommand);
	}
	if (eq(data, ":x")) {
		save();
		exit(1);
		return false;
	}
	lastCommand = clone(data);
	if (eq(data, ":w")) return save();
	if (eq(data, ":q")) {
		exit(1);
		return false;
	}
	if (eq(data, "q"))return false;
	if (eq(data, "x"))return false;
	if (eq(data, "export"))return export_csv();
	if (eq(data, ":cl"))return clearMemory();
	if (eq(data, "clear"))return clearMemory();
	if (eq(data, "./clear-shared-memory.sh"))return clearMemory();
	if (eq(data, ":e"))return export_csv();
	if (eq(data, "quit"))return false;
	if (eq(data, "quiet"))quiet = !quiet;

	//        if (eq(data, "collect abstracts")) {
	//            collectAbstracts2();
	//            return true;
	//        }
	if (startsWith(data, ":iy")) {
		import("yago");
		return true;
		//		importYago()
		//		  importAllYago();
	}
	if (startsWith(data, ":i") || startsWith(data, "import")) {
		if (arg.length() > 2) // && arg<0x7fff00000000
			import(arg.c_str());
		else
			importAll();
		return true;
	}
	if (startsWith(data, "limit")) {
		sscanf(data, "limit %d", &defaultLimit);
		return true;
	}
	if (eq(data, "load") || eq(data, ":l")) {
		load(false);
		return true;
	}
	if (eq(data, "load_files")||eq(data, ":lf") || eq(data, ":l!") || eq(data, "load!")) {
		load(true);
		return true;
	}
	if (eq(data, "save")) {
		save();
		return true;
	}
	if (eq(data, ":s")) {
		save();
		return true;
	}
	if (eq(data, ":c")) {
		showContext(currentContext());
		return true;
	}
	if (eq(data, ":t") || eq(data, "test") || eq(data, "test ") || eq(data, "tests")) {
		exitOnFailure = false;
		tests();
		return true;
	}
	if (eq(data, "debug") || eq(data, "debug on") || eq(data, "debug 1")) {
		debug = true;
		return true;
	}
	if (eq(data, "debug off") || eq(data, "no debug") || eq(data, "debug 0")) {
		debug = true;
		return true;
	}
	if (startsWith(data, ":d ") || startsWith(data, "delete ") || startsWith(data, "del ") || startsWith(data, "remove ")) {
		remove(next(data).c_str());
		return true;
	}

	if (startsWith(data, "path") || startsWith(data, ":p")) {
		Node* from = getAbstract(args.at(1));
		Node* to = getAbstract(args.at(2));
		shortestPath(from, to);
		return true;
	}

	if (startsWith(data, "has") || startsWith(data, ":h")) {
		Node* from = getAbstract(args.at(1));
		Node* to = getAbstract(args.at(2));
		memberPath(from, to);
		return true;
	}
	if (startsWith(data, "is")) {
		Node* from = getAbstract(args.at(1));
		Node* to = getAbstract(args.at(2));
		parentPath(from, to);
		return true;
	}
	if (startsWith(data, "select ")) {
		query(data);
		return true;
	}
	if (startsWith(data, "all ")) {
		query(data);
		return true;
	}
	if (startsWith(data, "these ")) {
		query(data);
		return true;
	}
	if (contains(data, "that ")) {
		query(data);
		return true;
	}
	if (contains(data, "who")) {// who loves jule
		query(data);
		return true;
	}

	if (startsWith(data, "the ") || startsWith(data, "my ")) {
		show(getThe(data)); // -> query
		return true;
	}
	if (startsWith(data, "an ")) {
		query(data);
		return true;
	}
	if (startsWith(data, "a ")) {
		query(data);
		return true;
	}
	if (startsWith(data, "any ")) {
		query(data);
		return true;
	}

	//        if(startsWith(data,"is ")){  check_statement(data);return true;}
	//        if(startsWith(data,"does ")){  check_statement(data);return true;}
	if (contains(data, " in ")) {
		query(data);
		return true;
	}
	if (contains(data, " of ") || contains(data, ".") && !contains(data, " ")) {
		parseProperty(data);
		//        query(data);
		return true;
	}


	if (contains(data, " with ")) {
		query(data);
		return true;
	}
	if (contains(data, " where ")) {
		query(data);
		return true;
	}
	if (contains(data, " from ")) {
		query(data);
		return true;
	}
	if (contains(data, " whose ")) {
		query(data);
		return true;
	}
	if (contains(data, " which ")) {
		query(data);
		return true;
	}
	if (contains(data, " without ")) {
		query(data);
		return true;
	}
	if (contains(data, "ing ")) {
		query(data);
		return true;
	}
	if (contains(data, " that ")) {
		query(data);
		return true;
	}
	if (contains(data, "who ")) {
		query(data);
		return true;
	}
	if (!isprint(data[0])) {
		return false;
	}
	if (splitString(data, " ").size() > 2) {
		learn(data); // SPO
		return true;
	}

	if (splitString(data, " ").size() == 2) {
		Node* from = getAbstract(args.at(0));
		Node* to = getAbstract(args.at(1));
		NodeVector all = memberPath(from, to);
		//		if(all==EMPTY)parentPath(from,to);
		//		if(all==EMPTY)shortestPath(from,to);
		return true;
	}

	//        query(string("all ")+data);// NO!
	//        query(data);// NOO!
	findWord(currentContext()->id, data);
	int i = atoi(data);
	if (i > 0) {
		if (i < 1000)showContext(i);
		showNr(currentContext()->id, i);
	}
}


#ifdef signal
// CATCH signal and try recovery!
#include <setjmp.h>
#include <execinfo.h>
jmp_buf try_context;
jmp_buf loop_context;
jmp_buf loop_context1;

void print_trace(const char * msg) {
	void *array[100];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 100);
	strings = backtrace_symbols(array, size);
	printf("%s Obtained %zd stack frames.\n", msg, size);
	for (i = 0; i < size; i++)
		printf("--> %s\n", strings[i]);
	free(strings);
}

void SIGINT_handler(int sig) {
	print_trace("SIGINT!\n");
	flush();
	longjmp(loop_context, 128);
}

void handler(int sig) {
	print_trace("EXC_BAD_ACCESS !\n");
	flush();
	longjmp(try_context, 128);
}
#endif
bool read_file = false;

void getline(char *buf) {
	int MAXLENGTH = 1000;
	const char* PROMPT = "netbase> ";
#ifdef RL_READLINE_VERSION // USE_READLINE
	if (!read_file)read_file = 1 + read_history(0);
	char *tmp= readline(PROMPT);
	if (strncmp(tmp, buf, MAXLENGTH)) add_history(tmp); // only add new content
	strncpy(buf, tmp, MAXLENGTH);
	buf[MAXLENGTH] = '\0';
	write_history(0);
//	append_history(1,0);
	//            free(tmp);
#else
	std::cout << PROMPT;
	std::cin.get(buf, MAXLENGTH);
	std::cin.ignore(); // delete CR
#endif
}

void console() {
	Node* n;
	int i;
	quiet = false;
	printf("\nNetbase C++ Version z.a\n");
	char* data = (char*) malloc(1000);
#ifdef signal
	setjmp(try_context); //recovery point
#endif
	while (true) {
//		clearAlgorithmHash();
		getline(data);
		parse(data);
	}
}

char* serve(const char* data) {
	if (parse(data))return "success"; // stdout.to_s
	else return "NOPE";
}
