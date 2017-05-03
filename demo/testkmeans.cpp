#include "FL.h"
//#include "kMeans.h"

int main(int argc, char **argv) {
    char             ext[10],*pos;
    if (argc != 2) {
        printf("histogram <image.[pgm,ppm]>\n");
        exit(-1);
    }

    pos = strrchr(argv[1],'.') + 1;
    sscanf(pos,"%s",ext);
	
	printf("----%s------", argv[1]);
    if (strcmp(ext,"pgm")==0){
        GrayImage *grayImage = readGrayImage(argv[1]);
        destroyGrayImage(&grayImage);
    } else { /* ppm */
        ColorImage *colorImage =  readColorImage(argv[1]);
        destroyColorImage(&colorImage);
    }


    //Image* image = readImage(argv[1]);
    //ColorImage* image = readColorImage(argv[1]);
    ///FeatureMatrix* matriz = computeFeatureVectors(image, 16);

    //FeatureMatrix* dictionary = computekMeans(matriz, matriz->nFeaturesVectors*0.1, 256);

	
    return 0;
}
