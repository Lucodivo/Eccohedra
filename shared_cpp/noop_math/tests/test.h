#pragma  once

#include <gtest/gtest.h>

#include "noop_types.h"

#include "noop_math.h"
using namespace noop;

void printVec3(const vec3& v) {
  printf("[%4.2f, %4.2f, %4.2f]\n", v.x, v.y, v.z);
}

void printVec3(const char* name, const vec3& v) {
  printf("%s: [%4.2f, %4.2f, %4.2f]\n", name, v.x, v.y, v.z);
}

void printVec4(const vec4& v) {
  printf("[%4.2f, %4.2f, %4.2f, %4.2f]\n", v.x, v.y, v.z, v.w);
}

void printVec4(const char* name, const vec4& v) {
  printf("%s: [%4.2f, %4.2f, %4.2f, %4.2f]\n", name, v.x, v.y, v.z, v.w);
}

void printMat4(const char* name, const mat4& M) {
  printf("===%s===\n", name);
  for(u32 i = 0; i < 4; i++) {
    printVec4(M.col[i]);
  }
}

b32 printIfNotEqual(const vec4& v1, const vec4& v2) {
  b32 equal = v1 == v2;
  if(!equal) {
    printf("Two vec4s are not equal");
    printVec4("vec1", v1);
    printVec4("vec2", v2);
  }
  return equal;
}

b32 printIfNotEqual(const mat4& A, const mat4& B) {
  b32 equal = A == B;
  if(!equal) {
    printf("Two mat4s are not equal");
    printMat4("mat1", A);
    printMat4("mat2", B);
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