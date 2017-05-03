#include "FL.h"

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
	
    if (argc != 6){
        printf("Usage:\n<directory path [such as ../data/]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<maxIterations [such as 100]>\ndictionary file [such as ../data/dic.txt]\n");
        exit(-1);
    }

    DirectoryManager* directoryManager = loadDirectory(argv[1], 1);
	int nFiles = (int)directoryManager->nfiles;
	int patchSize = atoi(argv[2]);
	int binSize = atoi(argv[3]);
	int maxIterations = atoi(argv[4]);
	char *filename = argv[5];

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
	//printf("Features\n");
	//printFeatureMatrix(features, numberPatchs);

	int k = ceil(numberPatchs*0.1);	
	
	FeatureMatrix *dictionary = computekMeans(features, k, numberPatchs, maxIterations);
	//printf("Dictionary\n");	
	//printFeatureMatrix(dictionary, k);

    destroyDirectoryManager(&directoryManager);
	destroyFeatureMatrix(&features);

	writeFeatureMatrix(dictionary, filename, k);
	printf("write dictionary in file %s\n", filename);
	destroyFeatureMatrix(&dictionary);
	
    return 0;
}

