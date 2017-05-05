#include "kMeans.h"

bool isBreakDifference(FeatureMatrix* old_centroids, FeatureMatrix* centroids, int k) {
	float diff = 0.0;
	for(int i=0; i<k; i++) {
		diff += vectorDifference(old_centroids->featureVector[i], centroids->featureVector[i]);
	}
	float minDiff = 0.0000000001*k*centroids->featureVector[0]->size;
	if(diff < (minDiff + 0.0000000005))
		return true;
	return false;
}

//Function to choose the k words to a histogram
FeatureMatrix* computekMeans(FeatureMatrix* histogram, int k, int nFeaturesVectors, int maxIterations, int* id_centroids){
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
                    id_centroids[i] = randomIndex; // Armazena id inicial de cada centroid (saber a qual classe ele pertence) 
					break;
				}
			}
		}
	}

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

		//Old_centroids = centroids; centroids e elements zerados
		for(int i=0; i<k; i++){
			elements[i]=0;
			destroyFeatureVector(&old_centroids->featureVector[i]);
			old_centroids->featureVector[i] = copyFeatureVector(centroids->featureVector[i]);
			for(int j=0; j<histogram->featureVector[0]->size; j++)
				centroids->featureVector[i]->features[j] = 0.0;
		}
		
		//Atualiza centroides
		for(int i=0; i<nFeaturesVectors; i++){
			for(int j=0; j<histogram->featureVector[0]->size; j++){//Soma de todos os pontos de cada cluster
				centroids->featureVector[clusters[i]]->features[j] += histogram->featureVector[i]->features[j];		
			}
			elements[clusters[i]]++; //Quantidade de elementos em cada cluster
		}
		
		for(int i=0; i<nFeaturesVectors; i++) {
			isUsed[i] = false;
		}
		
		for(int i=0; i<k; i++){
			
			//centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/(elements[i]+1);
			if(elements[i] != 0) {
				for(int j=0; j<histogram->featureVector[0]->size; j++)
					centroids->featureVector[i]->features[j] = centroids->featureVector[i]->features[j]/elements[i]; //Tira ponto médio em cada cluster.
    
                //Para cada cluster, calcular o elemento mais próximo da média
                    dist = 0;
                    min = MAX_n;
                    for(int j=0; j<nFeaturesVectors; j++){
                        dist = pow(vectorDifference(histogram->featureVector[j],centroids->featureVector[i]),2);
                        if (dist < min){
                            min = dist;
                            //clusters[i]=j;
                            id_centroids[i] = j;
                        }
                    }
                    for(int s=0; s<histogram->featureVector[0]->size; s++) //Atribui o elemento mais próximo da média como centróide
                        centroids->featureVector[i]->features[s] = histogram->featureVector[id_centroids[i]]->features[s];
			} else {
				while(true) {
					int randomIndex = rand() % nFeaturesVectors;
					if(isUsed[randomIndex] == false) {
						isUsed[randomIndex] = true;
						centroids->featureVector[i] = copyFeatureVector(histogram->featureVector[randomIndex]);
                        id_centroids[i] = randomIndex;
						break;
					}
				}
			}
			
		}
		//if(isBreakDifference(old_centroids, centroids, k))
			//break;
	}
	
	printf("break kmeans in iteration %d\n", iter);
	
	free(isUsed);
	free(elements);
	free(clusters);
	destroyFeatureMatrix(&old_centroids);
	
	return centroids;
}
