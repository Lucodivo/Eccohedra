#pragma once

#include <time.h>

// TODO: Seems accurate but is his the best way to get time?
f64 getTime() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return f64((s64)now.tv_sec * NANOSECONDS_PER_SECOND + now.tv_nsec) / NANOSECONDS_PER_SECOND;
}

// stopwatch
struct StopWatch {
  f64 totalElapsed;
  f64 lastFrame;
  f64 deltaSeconds;
};

StopWatch createStopWatch() {
  StopWatch stopWatch;
  stopWatch.totalElapsed = 0.0;
  stopWatch.lastFrame = getTime();
  stopWatch.deltaSeconds = 0.0;
  return stopWatch;
}

void updateStopWatch(StopWatch* stopWatch) {
  f64 t = getTime();
  stopWatch->deltaSeconds = t - stopWatch->lastFrame;
  stopWatch->lastFrame = t;
  stopWatch->totalElapsed += stopWatch->deltaSeconds;
}

// 32 bit boolean flags
void clearFlags(b32* flags) {
  *flags = 0;
}

void setFlags(b32* flags, b32 desiredFlags){
  *flags |= desiredFlags;
}

void overrideFlags(b32* flags, b32 desiredFlags) {
  *flags = desiredFlags;
}

b32 flagIsSet(b32 flags, b32 checkFlag) {
  return flags & checkFlag;
}

b32 flagsAreSet(b32 flags, b32 checkFlags) {
  return (flags & checkFlags) == checkFlags;
}

char* cStrAllocateAndCopy(const char* cStr) {
  char* returnCStr = new char[strlen(cStr) + 1];
  strcpy(returnCStr, cStr);
  return returnCStr;
}

b32 fileReadable(const char* filename) {
  if (FILE* file = fopen(filename, "r")) {
    fclose(file);
    return true;
  }
  return false;
}

b32 empty(const char* cStr) {
  return cStr[0] == '\0';
}