#ifndef TEST_H
#define TEST_H


#define LOGPATH "../tcp/log/log.txt"
#define TIMESTAMPPATH "../tcp/log/timestamp.txt"

extern void process();

extern void LogTest(const char *s,...);

extern void Loginfo(const char *s, ...);

#endif