#include "FL.h"
#include "constants.h"

void writeLabels(char** label, char *filename, int nFeaturesVectors){
    FILE *fp = fopen(filename,"w");
    for (int i = 0; i < nFeaturesVectors; ++i) {
        fprintf(fp, "s\n", label[i]);
    }
    fclose(fp);
}

void printFeatureMatrixWithName(FeatureMatrix *m, int nFeatures, int numberPatchsPerImage, DirectoryManager* directoryManager) {
	for(int i=0; i<nFeatures; i++) {
		printf("[%d]: {", i);
		for(int j=0; j<m->featureVector[0]->size; j++) {
			printf(" %f", m->featureVector[i]->features[j]);
		}
		printf(" } ");
		printf("%s\n",directoryManager->files[i/numberPatchsPerImage]->path);
	}
}

//Printa centroids com os respectivos ids de histogram -> possivel identificar de qual imagem vem dado centróide
void printCentroids(FeatureMatrix *m, int nFeatures, int numberPatchsPerImage, DirectoryManager* directoryManager, int* id_centroids) {
    for(int i=0; i<nFeatures; i++) {
        printf("[%d]: {", i);
        for(int j=0; j<m->featureVector[0]->size; j++) {
            printf(" %f", m->featureVector[i]->features[j]);
        }
        printf(" } ");
        printf("%s ",directoryManager->files[id_centroids[i]/numberPatchsPerImage]->path);
        printf("%d\n", (int)(id_centroids[i]-(floor(id_centroids[i]/numberPatchsPerImage)*numberPatchsPerImage)+1));
    }
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
/*
char** givesLabel(int nFeatures, int* centroids, DirectoryManager* directoryManager, int numberPatchsPerImage){
    char **label;
    label = (char**)malloc(sizeof(char*)*nFeatures); // cada histograma passa a ter um label
    
    for(int i=0; i<nFeatures; i++){
        label[i]=(char*)malloc(sizeof(char)*32);
        label[i] = directoryManager->files[id_centroids[i]/numberPatchsPerImage]->path;
    }
    return label;
}

void modifyFileName(DirectoryManager* directoryManager, int patchSize, int binSize){ //
    Image* firstImage = readImage(directoryManager->files[0]->path);
    int patchX_axis = firstImage->nx/patchSize;
    int patchY_axis = firstImage->ny/patchSize;
    int numberPatchsPerImage = patchX_axis*patchY_axis;
    int numberPatchs = numberPatchsPerImage*directoryManager->nfiles;
    FeatureMatrix* featureMatrix = createFeatureMatrix(numberPatchs);
    //featureMatrix->nFeaturesVectors = numberPatchs;
    destroyImage(&firstImage);
    int k=0;
    
    //#pragma omp parallel for
    for (size_t fileIndex = 0; fileIndex < directoryManager->nfiles; ++fileIndex) {
        int id, img;
        char new_name[32];
        sscanf(directoryManager->files[fileIndex]->path, "../data/patchs/patch_%d_%d.ppm", id, img);
        sprintf(new_name, "../data/patchs/patch_%d_%d_label%d.ppm", id, img, label[]);
        rename(directoryManager->files[fileIndex]->path, new_name);
    }
    
    return featureMatrix;
}*/

int main(int argc, char **argv) {
	
    /*if (argc != 6){
        printf("Usage:\n<directory path [such as ../data/]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<maxIterations [such as 100]>\ndictionary file [such as ../data/dic.txt]\n");
        exit(-1);
    }*/

    DirectoryManager* directoryManager = loadFilesFromDirBySuffix(DICTIONARY_DATA, ".ppm");
	int nFiles = (int)directoryManager->nfiles;
	int patchSize = PATCH_SIZE;
	int binSize = BIN_SIZE;
	int maxIterations = MAX_ITERATIONS;
	char *filename = DICTIONARY_FILE;
    int *id_centroids, *clusters;
    char **label;

	Image* firstImage = readImage(directoryManager->files[0]->path);
    int patchX_axis = firstImage->nx/patchSize;
    int patchY_axis = firstImage->ny/patchSize;
    int numberPatchsPerImage = patchX_axis*patchY_axis;
    int numberPatchs = numberPatchsPerImage*nFiles;
    
	Image* imagePatch = extractSubImage(firstImage,0,0,patchSize,patchSize,true);
    FeatureVector* hist  = computeHistogramForFeatureVector(imagePatch,(float)binSize,true);
	//int nFeatures = hist->size;

	destroyImage(&imagePatch);
	destroyImage(&firstImage);
	destroyFeatureVector(&hist);

    FeatureMatrix* features = computeFeatureVectors(directoryManager, patchSize, binSize);
    // salva no nome do arquivo: qual é o id geral e a qual imagem pertence
    
	printf("Features\n");
	//printFeatureMatrixWithName(features, numberPatchs, numberPatchsPerImage, directoryManager);

	int k = ceil(numberPatchs*0.1);
    //int k = ceil(10);
    id_centroids = (int*)malloc(sizeof(int)*k); //Salva o id original de cada histograma para cada centroide
    clusters = (int*)malloc(sizeof(int)*numberPatchs); //Salva o id original de cada histograma para cada centroide
    
	FeatureMatrix *dictionary = computekMeans(features, k, numberPatchs, maxIterations, id_centroids, clusters);
    //label = givesLabel(dictionary->nFeatures, id_centroids, directoryManager, numberPatchsPerImage); // atribui label do arquivo para cada centroide
	//printf("Dictionary\n");
	printFeatureMatrix(dictionary, k);
    printCentroids(dictionary, k, numberPatchsPerImage, directoryManager, id_centroids); //pode ser otimizada pela função gives label
    
    destroyDirectoryManager(&directoryManager);
	destroyFeatureMatrix(&features);

	writeFeatureMatrix(dictionary, filename, k);
	printf("write dictionary in file %s\n", filename);
	destroyFeatureMatrix(&dictionary);
	
    free(id_centroids);
    return 0;
}

