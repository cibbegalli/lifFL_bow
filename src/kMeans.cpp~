#include "kMeans.h"

bool isBreakDifference(FeatureMatrix* old_centroids, FeatureMatrix* centroids, int k) {
	float diff = 0.0;
	for(int i=0; i<k; i++) {
		diff += vectorDifference(old_centroids->featureVector[i], centroids->featureVector[i]);
	}
	float minDiff = 0.001*k*centroids->featureVector[0]->size;
	if(diff < (minDiff + 0.0001))
		return true;
	return false;
}

//Function to choose the k words to a histogram
FeatureMatrix* computekMeans(FeatureMatrix* histogram, int k, int nFeaturesVectors, int maxIterations){
	FeatureMatrix* centroids = createFeatureMatrix(k);
	FeatureMatrix* old_centroids = createFeatureMatrix(k);
	for(int i=0; i<k; i++) {	
		old_centroids->featureVector[i] = createFeatureVector(histogram->featureVector[0]->size);
		for(int j=0; j<(int)histogram->featureVector[0]->size; ++j) {
			old_centroids->featureVector[i]->features[j] = 0.0;
		}	
	}

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
	for(int i=0; i<nFeaturesVectors; i++) {
		clusters[i] = -1;
	}
	
	int iter = 0;


	while(iter < maxIterations){
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
		}

		
		for(int i=0; i<k; i++){
			//for(int j=0; j<histogram->featureVector[0]->size; j++)
			//	centroids->featureVector[i]->features[j] = 0.0;
			elements[i]=0;
			destroyFeatureVector(&old_centroids->featureVector[i]);
			old_centroids->featureVector[i] = copyFeatureVector(centroids->featureVector[i]);
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
				//if(elements[i] != 0) 
					//centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/elements[i];
				centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/(elements[i]+1);
		}
		if(isBreakDifference(old_centroids, centroids, k))
			break;
	}

	printf("break kmeans in iteration %d\n", iter);
	
	free(elements);
	free(clusters);
	destroyFeatureMatrix(&old_centroids);
	
	return centroids;
}
