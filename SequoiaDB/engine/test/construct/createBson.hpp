#ifndef CREATEBSON_HPP_
#define CREATEBSON_HPP_

#include "core.hpp"



INT32 getObjData(const CHAR *key, CHAR *&str);

void getObjKey(CHAR *str);

void getObjIndexKey(CHAR *str);

void getObjRule(CHAR *str);

void getObjDelKey(CHAR *str);

void getObjIndexDelKey(CHAR *str);

#endif

