#include "Main.h"
#include "ArraySumExample.h"

JNIEXPORT jint JNICALL Java_Main_fun
        (JNIEnv *env, jclass jObj, jint aVal, jint bVal){
    return aVal + bVal;
}

JNIEXPORT jint JNICALL Java_ArraySumExample_calculatorSum
        (JNIEnv *env, jclass jObj, jintArray numbers){

    jint length = env->GetArrayLength(numbers);

    jint *elements = env->GetIntArrayElements(numbers, NULL);

    int sum = 0;
    for (int i = 0; i < length; i++)
        sum += elements[i];

    return sum;
}
