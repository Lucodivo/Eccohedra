#pragma  once

#include <gtest/gtest.h>

#include "noop_types.h"

#include "noop_math.h"
using namespace noop;

void printVec3(const vec3& v) {
  printf("[%4.2f, %4.2f, %4.2f]\n", v[0], v[1], v[2]);
}

void printVec3(const char* name, const vec3& v) {
  printf("%s: [%4.2f, %4.2f, %4.2f]\n", name, v[0], v[1], v[2]);
}

void printVec4(const vec4& v) {
printf("[%4.2f, %4.2f, %4.2f, %4.2f]\n", v[0], v[1], v[2], v[3]);
}

void printVec4(const char* name, const vec4& v) {
  printf("%s: [%4.8f, %4.8f, %4.8f, %4.8f]\n", name, v[0], v[1], v[2], v[3]);
}

void printMat4(const char* name, const mat4& M) {
  printf("===%s===\n", name);
  printVec4(vec4{M[0], M[4], M[8], M[12]});
  printVec4(vec4{M[1], M[5], M[9], M[13]});
  printVec4(vec4{M[2], M[6], M[10], M[14]});
  printVec4(vec4{M[3], M[7], M[11], M[15]});
}

b32 printIfNotEqual(const vec4& v1, const vec4& v2) {
  b32 equal = v1 == v2;
  if(!equal) {
    printf("Two vec4s are not equal\n");
    printVec4("actual", v1);
    printVec4("expected", v2);
  }
  return equal;
}

b32 printIfNotEqual(const mat4& A, const mat4& B) {
  b32 equal = A == B;
  if(!equal) {
    printf("Two mat4s are not equal\n");
    printMat4("actual", A);
    printMat4("expected", B);
  }
  return equal;
}

//#include <windows.h>
//
//enum ConsoleTextColor {
//  TEXT_ATT_GREEN_TEXT = 10,
//  TEXT_ATT_RED_TEXT = 12,
//  TEXT_ATT_WHITE_TEXT = 15,
//};
//
//void setConsoleTextColor(ConsoleTextColor color = TEXT_ATT_WHITE_TEXT) {
//  local_access HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//  SetConsoleTextAttribute(hConsole, color);
//}