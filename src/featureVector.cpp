//
// Created by deangeli on 3/27/17.
//
#include "featureVector.h"

FeatureVector* createFeatureVector(int size){
    FeatureVector* featureVector = (FeatureVector*)calloc(1,sizeof(FeatureVector));
    featureVector->size = size;
    featureVector->features = (float*)calloc((size_t)size, sizeof(float));
    return featureVector;
}
void destroyFeatureVector(FeatureVector** featureVector){
    free((*featureVector)->features);
    free((*featureVector));
    (*featureVector) = NULL;
}

void printFeatureVector(FeatureVector* featureVector){
    for (int i = 0; i < featureVector->size; ++i) {
        printf("%f ",featureVector->features[i]);
    }
    printf("\n");
}

FeatureVector* mergeFeatureVectors(FeatureVector* vector1,FeatureVector* vector2){
    FeatureVector *mergedVector = NULL;
    if(vector1 == NULL || vector2 == NULL){
        printf("vector1 or/and vector2 are NULL\n");
        return mergedVector;
    }
    int mergedSize = vector1->size + vector2->size;
    mergedVector = createFeatureVector(mergedSize);
    for (int i = 0; i < vector1->size; ++i) {
        mergedVector->features[i] = vector1->features[i];
    }

    for (int i = 0; i < vector2->size; ++i) {
        mergedVector->features[i+vector1->size] = vector2->features[i];
    }
    return mergedVector;
}

FeatureVector* createFeatureVector(float* vec,int size){
    FeatureVector* featureVector = (FeatureVector*)calloc(1,sizeof(FeatureVector));
    featureVector->size = size;
    featureVector->features = (float*)calloc((size_t)size,sizeof(float));
    for (int i = 0; i < size; ++i) {
        featureVector->features[i] = vec[i];
    }
    return featureVector;
}


void writeFeatureVectors(FeatureVector** vectors, int nVectors, char *filename){
    FILE *fp = fopen(filename,"w");
    for (int i = 0; i < nVectors; ++i) {
        FeatureVector* vec = vectors[i];
        for (int j = 0; j < vec->size; ++j) {
            fprintf(fp,"%f",vec->features[j]);
            if(!(j == vec->size-1)){
                fprintf(fp," ");
            }
        }
        fprintf(fp,"\n");
    }
}


float vectorDifference(FeatureVector* vector1,FeatureVector* vector2){
    if(vector1->size != vector2->size){
        printf("vectors mismatch dimensions\n");
        return -1.0;
    }
    float difference = 0;
    float diff;
    for (int i = 0; i < vector1->size; ++i) {
        diff = (vector1->features[i]-vector2->features[i]);
        if(diff < 0){
            diff *= -1;
        }

        difference += diff;
    }
    return difference;
}

float vectorDistance(FeatureVector* vector1,FeatureVector* vector2){
    if(vector1->size != vector2->size){
        printf("vectors mismatch dimensions\n");
        return -1.0;
    }
    float distance = 0;
    float diff;
    for (int i = 0; i < vector1->size; ++i) {
        diff = (vector1->features[i]-vector2->features[i]);
        distance += (diff*diff);
    }
    return sqrtf(distance);

}

FeatureVector* copyFeatureVector(FeatureVector* featureVector){
    return createFeatureVector(featureVector->features,featureVector->size);
}



FeatureMatrix* createFeatureMatrix(int nFeaturesVectors){
    FeatureMatrix* featureMatrix = NULL;
    featureMatrix = (FeatureMatrix*)calloc(1,sizeof(FeatureMatrix));
	//featureMatrix->nFeaturesVectors = nFeaturesVectors;
    featureMatrix->featureVector = (FeatureVector**)calloc((size_t)nFeaturesVectors,sizeof(FeatureVector*));
    return featureMatrix;
}

FeatureMatrix* createFeatureMatrix(int nFeaturesVectors,int vectorSize){
    FeatureMatrix* featureMatrix = NULL;
    featureMatrix = (FeatureMatrix*)calloc(1,sizeof(FeatureMatrix));
    featureMatrix->featureVector = (FeatureVector**)calloc((size_t)nFeaturesVectors,sizeof(FeatureVector*));
    for (int i = 0; i < vectorSize; ++i) {
        featureMatrix->featureVector[0] = createFeatureVector(vectorSize);
    }
    return featureMatrix;
}

void destroyFeatureMatrix(FeatureMatrix** featureMatrix){
    if(*featureMatrix == NULL){
        return ;
    }
    for (int i = 0; i < (*featureMatrix)->nFeaturesVectors; ++i) {
        destroyFeatureVector( &((*featureMatrix)->featureVector[i]) );
    }
    free((*featureMatrix)->featureVector);
    free((*featureMatrix));
    *featureMatrix = NULL;
}

void writeFeatureMatrix(FeatureMatrix* featureMatrix, char *filename, int nFeaturesVectors){
    FILE *fp = fopen(filename,"w");
    for (int i = 0; i < nFeaturesVectors; ++i) {
        FeatureVector* vec = featureMatrix->featureVector[i];
        for (int j = 0; j < vec->size; ++j) {
            fprintf(fp,"%f",vec->features[j]);
            if(!(j == vec->size-1)){
                fprintf(fp," ");
            }
        }
        fprintf(fp,"\n");
    }
}


//void sortAt(FeatureVector featureVector, int lastIndex){
//
//    for (int i = lastIndex-1; i >= 0 ; --i) {
//        if(featureVector.features[lastIndex] > featureVector.features[i]){
//            break;
//        }
//    }
//    //featureVector
//}
