#include "FL.h"


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

int main(int argc, char **argv) {
	
    if (argc != 6){
        printf("Usage:\n<directory path [such as ../data/]>\n<dictionary file path [such as ../files/dic.txt]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<matriz file path [such as ../files/matriz.txt]>\n");
        exit(-1);
    }

    DirectoryManager* directoryManager = loadDirectory(argv[1], 1);
	int nFiles = (int)directoryManager->nfiles;
	char *filename = argv[2];
	int patchSize = atoi(argv[3]);
	int binSize = atoi(argv[4]);
	char *filenameMatriz = argv[5];

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
	if (file) {
		
		while (fscanf(file, "%s", str)!=EOF) {
			dictionarySize++;
		}
		fclose(file);
	}
	
	FeatureMatrix *dictionary = createFeatureMatrix(dictionarySize);
	// Lendo arquivo para salvar dicionario em estrutura FeatureMatrix
	file = fopen(filename , "r");
	if (file) {
		int indexLine = 0;
		while (fscanf(file, "%s", str)!=EOF) {
			dictionary->featureVector[indexLine] = createFeatureVector(nFeatures);
			char * pch;
			pch = strtok (str," ");
			int indexFeature = 0;
			while (pch != NULL) {
				dictionary->featureVector[indexLine]->features[indexFeature] = atof(pch);
				pch = strtok (NULL, " ");
				indexFeature++;
			}
			indexLine++;
		}
		fclose(file);
	}

	
    printf("reading directory:\n");

	float dist, minDist;
	int indexWord;

	int *labels = (int*) malloc(sizeof(int)*nFiles);
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
		matrix->featureVector[i] = copyFeatureVector(histogram);
		
		destroyFeatureVector(&histogram);
		destroyImage(&currentImage);
		destroyFeatureMatrix(&featuresCurrentImage);
    }
	writeFeatureMatrix(matrix, filenameMatriz, nFiles);

	destroyFeatureMatrix(&matrix);
    destroyDirectoryManager(&directoryManager);
	destroyFeatureMatrix(&dictionary);

    return 0;
}

