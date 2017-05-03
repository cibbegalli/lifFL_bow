#include "kMeans.h"
#define MAX_n 1000000


//Function to choose the k words to a histogram
FeatureMatrix* computekMeans(FeatureMatrix* histogram, int k, int nFeaturesVectors, int maxIterations){
	FeatureMatrix* centroids = createFeatureMatrix(k);

    bool *isUsed = (bool*)calloc(nFeaturesVectors,sizeof(bool));
	for(int i=0; i<nFeaturesVectors; i++) {
		isUsed[i] = false;
	}
	// inicializando centroides com pontos distintos
	for(int i=0; i<k; i++) {
		while(true) {
			int randomIndex = rand() % nFeaturesVectors;
			if(isUsed[randomIndex] == false) {
				bool repeat = false;
				for(int j=0; j<i; j++) {
					if(vectorDifference(histogram->featureVector[j], histogram->featureVector[randomIndex]) == 0)
						repeat = true;
				}
				if(repeat == false) {
					isUsed[randomIndex] = true;
					centroids->featureVector[i] = copyFeatureVector(histogram->featureVector[randomIndex]);
					break;
				}
			}
		}
	}
	free(isUsed);

	int dist, min;
	
	int *elements = (int*) malloc(sizeof(int)*k);
	int *clusters = (int*) malloc(sizeof(int)*nFeaturesVectors);
	int *oldClusters = (int*) malloc(sizeof(int)*nFeaturesVectors);
	for(int i=0; i<nFeaturesVectors; i++) {
		clusters[i] = -1;
		oldClusters[i] = -2;
	}
	
	int iter = 0;
	bool done = false;

	while(iter < maxIterations && done == false){

		done = true;
		iter++;

		//Atribui para cada histograma, um cluster mais próximo
		for(int i=0; i<nFeaturesVectors; i++){
			dist = 0;
			min = MAX_n;
			for(int j=0; j<k; j++){		
				dist = pow(vectorDifference(histogram->featureVector[i],centroids->featureVector[j]),2);
				if (dist < min){
					min = dist;
					clusters[i]=j;
				}
			}
			if(clusters[i] != oldClusters[i]) {
				done = false;
			}
		}
		
		for(int i=0; i<nFeaturesVectors; i++) {
			oldClusters[i] = clusters[i];		
		}
		
		for(int i=0; i<k; i++){
			for(int j=0; j<histogram->featureVector[0]->size; j++)
				centroids->featureVector[i]->features[j] = 0.0;
			elements[i]=0;
		}

		//Atualiza centroides
		for(int i=0; i<nFeaturesVectors; i++){
			for(int j=0; j<histogram->featureVector[0]->size; j++){
				centroids->featureVector[clusters[i]]->features[j] += histogram->featureVector[i]->features[j];		
			}
			elements[clusters[i]]++;
		}
		for(int i=0; i<k; i++){
			for(int j=0; j<histogram->featureVector[0]->size; j++)
				if(elements[i] != 0) 
					centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/elements[i];
					//centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/(elements[i]+1);
		}		
	}

	printf("---------------------------break in iteration %d\n", iter);
	
	free(elements);
	free(clusters);
	free(oldClusters);
	
	return centroids;
}
