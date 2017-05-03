//
// Created by deangeli on 3/27/17.
//

#ifndef _FEATUREVECTOR_H_
#define _FEATUREVECTOR_H_

#include "common.h"

typedef struct _featureVector {
    float* features;
    int size;
} FeatureVector;

/*
 * estrutura para armazenar varios features vectors
 * */
typedef struct _featureMatrix {
    FeatureVector **featureVector;
    int nFeaturesVectors;
} FeatureMatrix;


FeatureVector* createFeatureVector(int size);
FeatureVector* createFeatureVector(float* vec,int size);

/*
 * escreve uma arquivo txt onde caada linha no arquivo e um feature vector
 * */
void writeFeatureVector(FeatureVector* vector, FILE *fp);
void writeFeatureVectors(FeatureVector** vectors, int nVectors, char *filename);
void destroyFeatureVector(FeatureVector** vector);
void printFeatureVector(FeatureVector* featureVector);
FeatureVector* mergeFeatureVectors(FeatureVector* vector1,FeatureVector* vector2);
float vectorDifference(FeatureVector* vector1,FeatureVector* vector2);
float vectorDistance(FeatureVector* vector1,FeatureVector* vector2);
FeatureVector* copyFeatureVector(FeatureVector* featureVector);


/*
 * escreve uma arquivo txt onde caada linha no arquivo e um feature vector
 * */
FeatureMatrix* createFeatureMatrix(int nFeaturesVectors);
FeatureMatrix* createFeatureMatrix(int nFeaturesVectors,int vectorSize);
void writeFeatureMatrix(FeatureMatrix* featureMatrix, char *filename, int nFeaturesVectors);
void destroyFeatureMatrix(FeatureMatrix** featureMatrix);



void sortAt(FeatureVector featureVector, int lastIndex);

#endif //LIBFL_FEATUREVECTOR_H
