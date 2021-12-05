/*

	Macroces for quick creating pretty log file
	LOG_IN - log function name and increase tab
	LOG_OUT - decrease tab
	LOG - print name of var and its value by format
	LOG_F - print name of var and use func for printing value

	TODO:
	Level of priority - to increase/decrease log file without
		commenting huge amount of strings
	Somehow reduce syntax - remove necessary of specifiing format
	Use more comfortable and less errorable syntax for printing functions
	Create vscode extension for collapsing or minimizing lines in file
	Increase possibilities of passing format and customizing line length

*/

#ifndef HEADER_LOG
#define HEADER_LOG

#ifdef EXTRA_LOG
#define LOG_MAX_STRING_NUMBER 1024
#ifndef LOG_FILE
#define LOG_FILE "log"
#endif
void log_in();
void log_out();
void log_message(const char *file, int line, const char *format, ...);
void log_func(const char *file, int line,
			  const char *name,
			  void (*printer)(void *, void *), void *var);
#ifdef LOG_SELF
#define LOG_IN                                                         \
	log_message(__FILE__, __LINE__, "%s log in", __PRETTY_FUNCTION__); \
	log_in();
#define LOG_OUT \
	log_out();  \
	log_message(__FILE__, __LINE__, "%s log out", __PRETTY_FUNCTION__);
#else
#define LOG_IN                                                  \
	log_message(__FILE__, __LINE__, "%s", __PRETTY_FUNCTION__); \
	log_in();
#define LOG_OUT \
	log_out();
#endif
#define LOG(format, var) \
	log_message(__FILE__, __LINE__, "'%-20.20s' = " format, #var, var);
#define LOG_F(func, arg)         \
	log_func(__FILE__, __LINE__, \
			 #arg, func, &(arg));
#else
#define LOG_IN
#define LOG_OUT
#define LOG(format, var)
#define LOG_F(func, var)
#endif

#endif