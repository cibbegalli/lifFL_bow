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

int main(int argc, char **argv) {
 /*
    if (argc != 7){
        printf("Usage:\n<directory path [such as ../data/]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<classifier file path [such as ../files/classifier.txt]>\n<labels path [such as ../files/labels.txt]>\n<dictinary path [such as ../files/dic.txt]>\n");
        exit(-1);
    }*/
    
    DirectoryManager* directoryManager = loadDirectory(TEST_DATA, 1);
    int nFiles = (int)directoryManager->nfiles;
    int patchSize = PATCH_SIZE;
    int binSize = BIN_SIZE;
    char *filenameClassifier = TRAIN_FILE;
    char *filename = LABEL_FILE;
    char *filenameDic = DICTIONARY_FILE;
    
    Image* firstImage = readImage(directoryManager->files[0]->path);
    int patchX_axis = firstImage->nx/patchSize;
    int patchY_axis = firstImage->ny/patchSize;
    int numberPatchsPerImage = patchX_axis*patchY_axis;
    float accuracy = 0;
    
    Image* imagePatch = extractSubImage(firstImage,0,0,patchSize,patchSize,true);
    FeatureVector* hist  = computeHistogramForFeatureVector(imagePatch,(float)binSize,true);
    
    int nFeatures = hist->size;
    
    destroyImage(&imagePatch);
    destroyImage(&firstImage);
    destroyFeatureVector(&hist);
    
    int dictionarySize = 0;
    //Verificando quantidade de palavras no dicionario
    char str[999];
    FILE * file = fopen(filenameDic , "r");
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
    
    int classifierSize = 0;
    //Verificando quantidade de palavras no dicionario
    file = fopen(filenameClassifier, "r");
    len = 0;
    line = NULL;
    if (file) {
        while ((nread = getline(&line, &len, file)) != -1) {
            classifierSize++;
        }
        fclose(file);
    }
    printf("\tclassifierSize=%d\n", classifierSize);
    
    FeatureMatrix *classifier = createFeatureMatrix(classifierSize);
    // Lendo arquivo para salvar dicionario em estrutura FeatureMatrix
    file = fopen(filenameClassifier, "r");
    if (file) {
        for(int i=0; i<classifierSize; i++) {
            classifier->featureVector[i] = createFeatureVector(dictionarySize);
        }
        int indexLine = 0;
        int indexFeature = 0;
        while (fscanf(file, "%s", str)!=EOF) {
            classifier->featureVector[indexLine]->features[indexFeature] = atof(str);
            if(indexFeature == (dictionarySize-1)) {
                indexLine++;
                indexFeature = 0;		
            } else {
                indexFeature++;
            }
        }
        fclose(file);
    }
    
    FeatureVector *labels = createFeatureVector(classifierSize);
    file = fopen(filename, "r");
    if (file) {
        int indexFeature = 0;
        while (fscanf(file, "%s\n", str)!=EOF) {
            labels->features[indexFeature] = atof(str);
            indexFeature++;
        }
        fclose(file);
    }
    
    
    printf("reading directory:\n");
    
    float dist, minDist;
    int indexWord;
    
    for (int i = 0; i < nFiles; ++i) {
        
        printf("%s\n",directoryManager->files[i]->path);
        Image* currentImage = readImage(directoryManager->files[i]->path);
        
        FeatureMatrix *featuresCurrentImage = computeFeatureVectors(currentImage, patchSize, binSize);
        FeatureVector *histogram = createFeatureVector(dictionarySize);
        
        for(int j=0; j<dictionarySize; j++) {
            histogram->features[j] = 0;
        }
        //Para cada patch da imagem, identifica palavra visual mais próxima
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

        for(int j=0; j<classifierSize; j++) {
            minDist = MAX_n;
            indexWord = -1;
            dist = vectorDistance(histogram,classifier->featureVector[j]);
            if(dist < minDist) {
                minDist = dist;
                indexWord = j;
            }
        }
        
        int atual_class = labels->features[indexWord];
        int real_class = imageLabel(directoryManager->files[i]->path);
        
        printf("atual: %d real: %d\n", atual_class, real_class);
        
        if(real_class == atual_class)
            accuracy++;
        
        destroyFeatureVector(&histogram);
        destroyImage(&currentImage);
        destroyFeatureMatrix(&featuresCurrentImage);
    }
    
    accuracy = accuracy/nFiles;
    
    printf("Final Accuracy: %f.\n", accuracy);
    
    destroyFeatureVector(&labels);
    destroyFeatureMatrix(&dictionary);
    destroyFeatureMatrix(&classifier);
    destroyDirectoryManager(&directoryManager);
    
    return 0;
}

