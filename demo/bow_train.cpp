#include "FL.h"
#include "constants.h"

char* mysubstr(char *str, int position, int length) {
	char *result = (char*)calloc(length+1, sizeof(char)); 
	int index = 0;
	while(index < length) {
		result[index] = str[position+index];
		index++;
	}
	result[index] = '\0';
	return result;
}

int imageLabel(char *filename) {

	char *name = strstr(filename, "obj");
	int l = -1, prev_l, nDigits = 1;
	do {
		prev_l = l;
		l = atoi(mysubstr(name, 3, nDigits));
		nDigits++;	
	} while(l != 0 && prev_l != l);

	if(prev_l == -1) {
		printf("error: not defined label image %s\n", filename);
		return 0;
	}

	return prev_l;
}


void printFeatureMatrix(FeatureMatrix *m, int nFeatures) {
	for(int i=0; i<nFeatures; i++) {
		printf("[%d]: {", i);
		for(int j=0; j<m->featureVector[0]->size; j++) {
			printf(" %f", m->featureVector[i]->features[j]);
		}
		printf(" }\n");
	}
}

void writeFeatureVector(FeatureVector* vec, char *filename){
    FILE *fp = fopen(filename,"w");
        for (int j = 0; j < vec->size; ++j) {
            fprintf(fp,"%f",vec->features[j]);
            fprintf(fp,"\n");
        }
}


int main(int argc, char **argv) {
	/*
    if (argc != 8){
        printf("Usage:\n<directory path [such as ../data/]>\n<dictionary file path [such as ../files/dic.txt]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<classifier file path [such as ../files/classifier.txt]>\nn class\nmaxIterations\n");
        exit(-1);
    }*/

    DirectoryManager* directoryManager = loadFilesFromDirBySuffix(TRAIN_DATA, ".ppm");
	int nFiles = (int)directoryManager->nfiles;
	char *filename = DICTIONARY_FILE;
	int patchSize = PATCH_SIZE;
	int binSize = BIN_SIZE;
	char *filenameClassifier = TRAIN_FILE;
	int k = N_CLASS;
	int maxIterations = MAX_ITERATIONS;

	Image* firstImage = readImage(directoryManager->files[0]->path);
    int patchX_axis = firstImage->nx/patchSize;
    int patchY_axis = firstImage->ny/patchSize;
    int numberPatchsPerImage = patchX_axis*patchY_axis;

	Image* imagePatch = extractSubImage(firstImage,0,0,patchSize,patchSize,true);
    FeatureVector* hist  = computeHistogramForFeatureVector(imagePatch,(float)binSize,true);

	int nFeatures = hist->size;
	
	destroyImage(&imagePatch);
	destroyImage(&firstImage);
	destroyFeatureVector(&hist);
	

	int dictionarySize = 0;


	//Verificando quantidade de palavras no dicionario
	char str[999];
	FILE * file = fopen(filename , "r");
	size_t len = 0;
	char *line = NULL;
	int nread;
	if (file) {
		while ((nread = getline(&line, &len, file)) != -1) {
			dictionarySize++;			
		}
		fclose(file);
	}
	printf("\tdictionarySize=%d\n", dictionarySize);


	FeatureMatrix *dictionary = createFeatureMatrix(dictionarySize);
	// Lendo arquivo para salvar dicionario em estrutura FeatureMatrix
	file = fopen(filename , "r");
	if (file) {

		for(int i=0; i<dictionarySize; i++) {
			dictionary->featureVector[i] = createFeatureVector(nFeatures);
		}
		int indexLine = 0;
		int indexFeature = 0;
		while (fscanf(file, "%s", str)!=EOF) {
			dictionary->featureVector[indexLine]->features[indexFeature] = atof(str);
			if(indexFeature == (nFeatures-1)) {
				indexLine++;		
				indexFeature = 0;		
			} else {
				indexFeature++;
			}
		}
		fclose(file);
	}

    printf("reading directory:\n");

	float dist, minDist;
	int indexWord;

	int *labels = (int*) malloc(sizeof(int)*nFiles);
    int *id_centroids = (int*)malloc(sizeof(int)*k);
    int *clusters = (int*)malloc(sizeof(int)*nFiles);
	FeatureMatrix *matrix = createFeatureMatrix(nFiles);

    for (int i = 0; i < nFiles; ++i) {

        printf("%s\n",directoryManager->files[i]->path);
        Image* currentImage = readImage(directoryManager->files[i]->path);

		FeatureMatrix *featuresCurrentImage = computeFeatureVectors(currentImage, patchSize, binSize);
		FeatureVector *histogram = createFeatureVector(dictionarySize);
		
		for(int j=0; j<dictionarySize; j++) {
			histogram->features[j] = 0;
		}
		
		for(int j=0; j<numberPatchsPerImage; j++) {
			minDist = MAX_n;
			indexWord = -1;
			for(int l=0; l<dictionarySize; l++) {
				dist = vectorDistance(featuresCurrentImage->featureVector[j],dictionary->featureVector[l]);
				if(dist < minDist) {
					minDist = dist;
					indexWord = l;
				}
			}
			
			histogram->features[indexWord]++;
		}

		labels[i] = imageLabel(directoryManager->files[i]->path);
        //printf("%d ", labels[i]);
		matrix->featureVector[i] = copyFeatureVector(histogram);
		
		destroyFeatureVector(&histogram);
		destroyImage(&currentImage);
		destroyFeatureMatrix(&featuresCurrentImage);
    }

	FeatureMatrix* classifier = computekMeans(matrix, k, nFiles, maxIterations, id_centroids, clusters);
    FeatureVector* label_centroids = createFeatureVector(k);
    for(int i=0; i<k; i++){
        label_centroids->features[i]=labels[id_centroids[i]];
    }

	writeFeatureMatrix(classifier, filenameClassifier, k);
	printFeatureMatrix(classifier, k);
    writeFeatureVector(label_centroids,LABEL_FILE);
	
	destroyFeatureMatrix(&matrix);
	destroyFeatureMatrix(&dictionary);
	destroyFeatureMatrix(&classifier);
    destroyDirectoryManager(&directoryManager);
    
    return 0;
}

