#include "Main.h"

JNIEXPORT jint JNICALL Java_Main_fun
        (JNIEnv *env, jclass jObj, jint aVal, jint bVal){
    return aVal + bVal;
}
