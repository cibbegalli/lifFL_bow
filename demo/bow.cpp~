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
	
    if (argc != 5){
        printf("Usage:\n<directory path [such as ../data/]>\n<patchSize [such as 8]>\n<binSize [such as 64]>\n<maxIterations [such as 100]>\n");
        exit(-1);
    }

    DirectoryManager* directoryManager = loadDirectory(argv[1], 1);
	int nFiles = (int)directoryManager->nfiles;
	int patchSize = atoi(argv[2]);
	int binSize = atoi(argv[3]);
	int maxIterations = atoi(argv[4]);

	printf("patchSize: %d\nbinSize: %d\n", patchSize, binSize);

	Image* firstImage = readImage(directoryManager->files[0]->path);
    int patchX_axis = firstImage->nx/patchSize;
    int patchY_axis = firstImage->ny/patchSize;
    int numberPatchsPerImage = patchX_axis*patchY_axis;
    int numberPatchs = numberPatchsPerImage*directoryManager->nfiles;

	printf("numberPatchsPerImage: %d\nnumberPatchs=%d\n", numberPatchsPerImage, numberPatchs);

	Image* imagePatch = extractSubImage(firstImage,0,0,patchSize,patchSize,true);
    FeatureVector* hist  = computeHistogramForFeatureVector(imagePatch,(float)binSize,true);

	int nFeatures = hist->size;
	printf("nFeatures: %d\n", nFeatures);
	
	destroyImage(&imagePatch);
	destroyFeatureVector(&hist);

	int k = ceil(numberPatchsPerImage*0.1);
	printf("kMeans k=%d\n", k);
    FeatureMatrix* dictionary = createFeatureMatrix(k*nFiles);
	
	printf("words in dictionary=%d\n", k*nFiles);

    printf("reading directory:\n");
    for (int i = 0; i < nFiles; ++i) {
        printf("%s\n",directoryManager->files[i]->path);
        Image* currentImage = readImage(directoryManager->files[i]->path);

		FeatureMatrix *featuresCurrentImage = computeFeatureVectors(currentImage, patchSize, binSize);
		printf("features:");		
		printFeatureMatrix(featuresCurrentImage, numberPatchsPerImage);
		printf("k clusters:");
		FeatureMatrix *kClusters = computekMeans(featuresCurrentImage, k, numberPatchsPerImage, maxIterations);
		printFeatureMatrix(kClusters, k);
		printf("\n");
		for(int j=0; j<k; ++j) {
			dictionary->featureVector[j+i*k] = copyFeatureVector(kClusters->featureVector[j]);
		}
		
		destroyImage(&currentImage);
		destroyFeatureMatrix(&kClusters);
		destroyFeatureMatrix(&featuresCurrentImage);
    }

    destroyDirectoryManager(&directoryManager);
	destroyFeatureMatrix(&dictionary);

    return 0;
}

