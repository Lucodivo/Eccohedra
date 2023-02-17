#pragma once

#include <time.h>

// TODO: Seems accurate but is his the best way to get time?
f64 getTime() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return f64((s64)now.tv_sec * NANOSECONDS_PER_SECOND + now.tv_nsec) / NANOSECONDS_PER_SECOND;
}

// Stopwatch In Seconds
struct StopWatch {
  f64 totalInSeconds;
  f64 lapInSeconds;

  f64 lastLapSystemTimeInSeconds;

  StopWatch() {
    totalInSeconds = 0.0;
    lastLapSystemTimeInSeconds = getTime();
    lapInSeconds = 0.0;
  }

  void resetLap() {
    lapInSeconds = 0;
    lastLapSystemTimeInSeconds = getTime();
  }

  void lap() {
    f64 t = getTime();
    lapInSeconds = t - lastLapSystemTimeInSeconds;
    lastLapSystemTimeInSeconds = t;
    totalInSeconds += lapInSeconds;
  }
};

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