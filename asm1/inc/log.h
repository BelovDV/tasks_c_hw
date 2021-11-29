

#ifndef HEADER_LOG
#define HEADER_LOG

#define EXTRA_LOG

#ifdef EXTRA_LOG
#ifndef LOG_FILE
#define LOG_FILE log
#endif
void log_in();
void log_out();
void log_message(const char *file, int line, const char *format, ...);
#define LOG_IN                                                  \
	log_message(__FILE__, __LINE__, "%s", __PRETTY_FUNCTION__); \
	log_in();
#define LOG_OUT \
	log_out();
#define LOG(format, var) \
	log_message(__FILE__, __LINE__, "'%s' = " format "\n", #var, var);
#else

#endif

#endif