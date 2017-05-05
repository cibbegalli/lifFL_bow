#ifndef _KMEANS_H_
#define _KMEANS_H_

#include "common.h"
#include "featureVector.h"

#define MAX_n 1000000

FeatureMatrix* initializeCentroids(FeatureMatrix* c, int nbins);
FeatureVector* initializeCentroid(FeatureVector* c, int nbins);
FeatureVector* calculateFeatureVectorResult(FeatureVector* v1, FeatureVector* v2);
//Function to choose the k words to a histogram
FeatureMatrix* computekMeans(FeatureMatrix* histogram, int k, int nFeaturesVectors, int maxIterations, int *id_centroids);

#endif
