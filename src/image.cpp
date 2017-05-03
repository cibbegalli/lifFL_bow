#include "image.h"

GrayImage *createGrayImage(int ncols, int nrows)
{
    GrayImage *img=NULL;
    int i;

    img = (GrayImage *) calloc(1,sizeof(GrayImage));
    if (img == NULL){
        Error(MSG1,"CreateImage");
    }

    img->val   = AllocIntArray(nrows*ncols);
    img->tbrow = AllocIntArray(nrows);

    img->tbrow[0]=0;
    for (i=1; i < nrows; i++)
        img->tbrow[i]=img->tbrow[i-1]+ncols;
    img->ncols = ncols;
    img->nrows = nrows;

    return(img);
}

void destroyGrayImage(GrayImage **img)
{
    GrayImage *aux;

    aux = *img;
    if(aux != NULL){
        if (aux->val != NULL)   free(aux->val);
        if (aux->tbrow != NULL) free(aux->tbrow);
        free(aux);
        *img = NULL;
    }
}

GrayImage *readGrayImage(char *filename)
{
    FILE *fp=NULL;
    unsigned char *value=NULL;
    char type[10];
    int  i,ncols,nrows,n;
    GrayImage *img = NULL;
    char z[256];

    fp = fopen(filename,"rb");
    if (fp == NULL){
        fprintf(stderr,"Cannot open %s\n",filename);
        exit(-1);
    }
    fscanf(fp,"%s\n",type);
    if((strcmp(type,"P5")==0)){
        NCFgets(z,255,fp);
        sscanf(z,"%d %d\n",&ncols,&nrows);
        n = ncols*nrows;
        NCFgets(z,255,fp);
        sscanf(z,"%d\n",&i);
        fgetc(fp);
        value = (unsigned char *)calloc(n,sizeof(unsigned char));
        if (value != NULL){
            fread(value,sizeof(unsigned char),n,fp);
        }else{
            fprintf(stderr,"Insufficient memory in ReadImage\n");
            exit(-1);
        }
        fclose(fp);
        img = createGrayImage(ncols,nrows);
        for (i=0; i < n; i++)
            img->val[i]=(int)value[i];
        free(value);
    }else{
        if((strcmp(type,"P2")==0)){
            NCFgets(z,255,fp);
            sscanf(z,"%d %d\n",&ncols,&nrows);
            n = ncols*nrows;
            NCFgets(z,255,fp);
            sscanf(z,"%d\n",&i);
            img = createGrayImage(ncols,nrows);
            for (i=0; i < n; i++)
                fscanf(fp,"%d",&img->val[i]);
            fclose(fp);
        }else{
            fprintf(stderr,"Input image must be P2 or P5\n");
            exit(-1);
        }
    }

    return(img);
}

void writeGrayImage(GrayImage *img,char *filename)
{
    FILE *fp;
    int i, n, Imax,Imin;

    fp = fopen(filename,"wb");
    if (fp == NULL){
        fprintf(stderr,"Cannot open %s\n",filename);
        exit(-1);
    }
    n    = img->ncols*img->nrows;
    Imax = INT_MIN; Imin = INT_MAX;
    for (i=0; i < n; i++) {

        if (img->val[i] > Imax)
            Imax = img->val[i];
        if (img->val[i] < Imin)
            Imin = img->val[i];
    }


    fprintf(fp,"P2\n");
    fprintf(fp,"%d %d\n",img->ncols,img->nrows);
    fprintf(fp,"%d\n",Imax-Imin);

    for (i=0; i < n; i++) {
        fprintf(fp,"%d ",img->val[i]-Imin);
        if (((i+1)%17) == 0)
            fprintf(fp,"\n");
    }

    fclose(fp);
}


int minimumValue(GrayImage *img){
    int i,min;

    min = img->val[0];
    int n = img->nrows*img->ncols;
    for (i = 0; i < n; i++){
        if (img->val[i] < min) {
            min = img->val[i];
        }
    }
    return min;
}
int maximumValue(GrayImage *img){
    int i,max;

    max = img->val[0];
    int n = img->nrows*img->ncols;
    for (i = 0; i < n; i++){
        if (img->val[i] > max) {
            max = img->val[i];
        }
    }
    return max;
}

ColorImage *createColorImage(int nx, int ny)
{
    ColorImage *I=(ColorImage *)calloc(1,sizeof(ColorImage));
    int y;

    I->nx = nx;
    I->ny = ny;
    I->dx = 1.0;
    I->dy = 1.0;
    sprintf(I->unid,"mm");

    I->cor = (Cor **)calloc(ny,sizeof(Cor *));
    if (I->cor == NULL)
        Error(MSG1,"createColorImage");
    else{
        for (y=0; y < I->ny; y++){
            I->cor[y] = (Cor *)calloc(nx,sizeof(Cor));
            if (I->cor[y] == NULL)
                Error(MSG1,"createColorImage");
        }
    }
    return(I);
}

ColorImage *readColorImage(char *filename)
{
    FILE *fp=NULL;
    char type[10];
    int  x,y,nx,ny,Imax;
    ColorImage *I=NULL;
    char remarks[256];
    ushort rgb16[3];

    fp = fopen(filename,"r");
    if (fp != NULL) {
        fscanf(fp,"%s",type);
        NCFgets(remarks,255,fp);

        if (strcmp(type,"P6")==0){
            int c = getc(fp);
            while (c == '#') {
                while (getc(fp) != '\n');
                c = getc(fp);
            }
            ungetc(c, fp);
            if (fscanf(fp, "%d %d", &nx, &ny) != 2) {
                fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
                exit(1);
            }

            I = createColorImage(nx,ny);

            if (fscanf(fp, "%d", &Imax) != 1) {
                fprintf(stderr, "Invalid rgb component (error loading '%s')\n",
                        filename);
                exit(1);
            }

            I->Imax = Imax;
            while (fgetc(fp) != '\n');

            if (Imax <= 255){
                for (y=0; y < ny; y++)
                    for (x=0; x < nx; x++){
                        I->cor[y][x].val[RED]=(int)fgetc(fp);//red
                        I->cor[y][x].val[GREEN]=(int)fgetc(fp);//green
                        I->cor[y][x].val[BLUE]=(int)fgetc(fp);//blue
                    }
            }else if (Imax > 255 && Imax <= 65535){
                int rgbBitDepth = 9;

                while ((1 << rgbBitDepth) <= Imax) {
                    rgbBitDepth++;
                }
                for (y=0; y < ny; y++) {
                    for (x=0; x < nx; x++){
                        if (fread(rgb16, 2, 3, fp) == 3) {
                            I->cor[y][x].val[RED]=((rgb16[0] & 0xff) << 8) | ((ushort) rgb16[0] >> 8);
                            I->cor[y][x].val[GREEN]=((rgb16[1] & 0xff) << 8) | ((ushort) rgb16[1] >> 8);
                            I->cor[y][x].val[BLUE]=((rgb16[2] & 0xff) << 8) | ((ushort) rgb16[2] >> 8);
                        } else{
                            Error("Reading 16-bit error","ReadColorImage");
                        }
                    }
                }
            }
        }else if(strcmp(type,"P3")==0){
            int c = getc(fp);
            while (c == '#') {
                while (getc(fp) != '\n');
                c = getc(fp);
            }
            ungetc(c, fp);
            if (fscanf(fp, "%d %d", &nx, &ny) != 2) {
                fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
                exit(1);
            }

            I = createColorImage(nx,ny);

            if (fscanf(fp, "%d", &Imax) != 1) {
                fprintf(stderr, "Invalid rgb component (error loading '%s')\n",
                        filename);
                exit(1);
            }

            I->Imax = Imax;

            int R=0,G=0,B=0;

            for (y=0; y < ny; y++) {
                for (x=0; x < nx; x++){
                    (void)fscanf(fp, "%d %d %d",&R,&G,&B);
                    I->cor[y][x].val[0] = R;
                    I->cor[y][x].val[1] = G;
                    I->cor[y][x].val[2] = B;
                }
            }
        }

        else{
            Error("ColorImage type invalid","ReadColormage");
        }

        fclose(fp);
    }else{
        Error(MSG2,"ReadColorImage");
    }

    return(I);
}

void writeColorImage(ColorImage *I, char *filename)
{
    FILE *fp=NULL;
    int  x,y;
    ushort rgb16[3];

    fp = fopen(filename,"w");
    if (fp != NULL) {
        fprintf(fp,"P6\n");
        fprintf(fp,"%d %d\n",I->nx,I->ny);

        int max_val = maximumColorValue(I);
        int min_val = minimumColorValue(I);

        if (min_val < 0){
            Error(MSG2,"WriteColorImage");
            return;
        }

        if (max_val < 256){
            fprintf(fp,"255\n");
            for (y=0; y < I->ny; y++)
                for (x=0; x < I->nx; x++){
                    fputc((uchar)I->cor[y][x].val[RED],fp);//red
                    fputc((uchar)I->cor[y][x].val[GREEN],fp);//green
                    fputc((uchar)I->cor[y][x].val[BLUE],fp);//blue
                }
        } else if (max_val < 65536){
            int rgbBitDepth = 9;
            // find the bit depth for the maximum value img_max_val
            while ((1 << rgbBitDepth) <= max_val) {
                rgbBitDepth++;
            }

            fprintf(fp, "%d\n", (1 << rgbBitDepth) - 1);
            for (y=0; y < I->ny; y++)
                for (x=0; x < I->nx; x++){
                    rgb16[RED] = ((I->cor[y][x].val[RED] & 0xff) << 8) | ((ushort) I->cor[y][x].val[RED] >> 8);//I->cor[y][x].val[RED];
                    rgb16[GREEN] = ((I->cor[y][x].val[GREEN] & 0xff) << 8) | ((ushort) I->cor[y][x].val[GREEN] >> 8);//I->cor[y][x].val[GREEN];
                    rgb16[BLUE] = ((I->cor[y][x].val[BLUE] & 0xff) << 8) | ((ushort) I->cor[y][x].val[BLUE] >> 8);//I->cor[y][x].val[BLUE];
                    // write 6 bytes for each image pixel
                    if (fwrite(rgb16, 2, 3, fp) != 3) {
                        Error("Cannot write 16-bit image as P6", "iftWriteImageP6");
                    }
                }
        }
        fclose(fp);
    }else{
        Error(MSG2,"WriteColorImage");
    }

}

void destroyColorImage(ColorImage **I)
{
    int y;

    if ((*I) != NULL) {
        for (y=0; y < (*I)->ny; y++)
            free((*I)->cor[y]);
        free((*I)->cor);
        free(*I);
        *I = NULL;
    }
}


int minimumColorValue(ColorImage *img){

    int img_min_val[3];
    for (int i=0; i<3; i++){
        img_min_val[i] = minimumIntensityColor(img,i);
    }

    return MIN(MIN(img_min_val[RED],img_min_val[GREEN]),img_min_val[BLUE]);
}

int maximumColorValue(ColorImage *img){

    int img_max_val[3];
    for (int i=0; i<3; i++){
        img_max_val[i] = maximumIntensityColor(img,i);
    }

    return MAX(MAX(img_max_val[RED],img_max_val[GREEN]),img_max_val[BLUE]);
}



int minimumIntensityColor(ColorImage *img, int c){
    int i,j,min;

    min = INT_MAX;
    for (i = 0; i<img->nx; i++)
        for (j=0; j<img->ny; j++)
            if (img->cor[j][i].val[c] < min)
                min = img->cor[j][i].val[c];

    return min;
}

int maximumIntensityColor(ColorImage *img, int c){
    int i,j,max;

    max = -1;
    for (i = 0; i<img->nx; i++)
        for (j=0; j<img->ny; j++)
            if (img->cor[j][i].val[c] > max)
                max = img->cor[j][i].val[c];

    return max;
}

bool isValidPixelCoordinate(GrayImage *image,int pixelCoordinateX,int pixelCoordinateY){
    if(pixelCoordinateX < 0 || pixelCoordinateY < 0){
        return false;
    }
    if(pixelCoordinateX >= image->ncols || pixelCoordinateY >= image->ncols){
        return false;
    }
    return true;
}

bool isValidPixelCoordinate(Image *image,int pixelCoordinateX,int pixelCoordinateY){
    if(pixelCoordinateX < 0 || pixelCoordinateY < 0){
        return false;
    }
    if(pixelCoordinateX >= image->nx || pixelCoordinateY >= image->ny){
        return false;
    }
    return true;
}

bool isImagesSameDomain(GrayImage *image1,GrayImage *image2){

    if(image1->nrows == image2->nrows && image1->ncols == image2->ncols){
        return true;
    }
    return false;
}

bool isImagesSameDomain(Image *image1,Image *image2){

    if(image1->nx == image2->nx && image1->nx == image2->nx){
        if(image1->nchannels == image2->nchannels){
            return true;
        }
    }
    return false;
}

void copyGrayImage(GrayImage *image1,GrayImage **image2){
    if((*image2) == NULL){
        *image2 = createGrayImage(image1->ncols,image1->nrows);
    }

    if(isImagesSameDomain(image1,*image2)){
        for (int n = 0; n < image1->nrows*image1->ncols; ++n) {
            (*image2)->val[n] = image1->val[n];
        }
    }
    else{
        printf("different domains");
    }
}

GrayImage *imageSubtraction(GrayImage *image1, GrayImage *image2, bool saturation){
    GrayImage *outputImage = NULL;
    if(isImagesSameDomain(image1,image2)){
        outputImage = createGrayImage(image1->ncols,image1->nrows);
        int n = image1->ncols*image1->nrows;
        int result;
        for (int i = 0; i < n; ++i) {
            result = image1->val[i] - image2->val[i];
            if(saturation){
                result = (result<0)?0:result;
            }
            outputImage->val[i] = result;
        }
    }
    return outputImage;
}



int sumUpAllPixelsValues(GrayImage *image){
    int sum = 0;
    for (int n = 0; n < image->nrows*image->ncols; ++n) {
        sum += image->val[n];
    }
    return sum;
}


void writeImageAsASCII(GrayImage* image,const char *filename,unsigned int maxValueOnChanel){
    int i, j;
    FILE *fp = fopen(filename, "w");
    (void) fprintf(fp, "P2\n%d %d\n%d\n", image->ncols,image->nrows, (int) maxValueOnChanel);
    int k = 0;
    for (i = 0; i < image->nrows; i++){
        for (j = 0; j < image->ncols; j++){
            (void) fprintf(fp,"%d ",image->val[k]);
            k++;
        }
        (void) fprintf(fp,"\n");
    }
    (void) fclose(fp);
}

void writeImageAsASCII(ColorImage* image,const char *filename, unsigned int maxValueOnChanel){
    int i, j;
    FILE *fp = fopen(filename, "w");
    (void) fprintf(fp, "P3\n%d %d\n%d\n", image->nx,image->ny, (int) maxValueOnChanel);
    for (i = 0; i < image->ny; i++){
        for (j = 0; j < image->nx; j++){
            (void) fprintf(fp,"%d %d %d ",image->cor[i][j].val[0],image->cor[i][j].val[1],image->cor[i][j].val[2]);
        }
        (void) fprintf(fp,"\n");
    }
    (void) fclose(fp);
}

void writeImageAsBinary(GrayImage* image,const char *filename,unsigned int maxValueOnChanel){
    int i, j;
    FILE *fp = fopen(filename, "wb");
    (void) fprintf(fp, "P5\n%d %d\n%d\n", image->ncols,image->nrows, (int) maxValueOnChanel);
    int k = 0;
    for (i = 0; i < image->nrows; i++){
        for (j = 0; j < image->ncols; j++){
            static unsigned char gray[1];
            gray[0] = image->val[k];
            (void) fwrite(gray, 1, 1, fp);
            k++;
        }
        //(void) fprintf(fp,"\n");
    }
    (void) fclose(fp);
}

void writeImageAsBinary(ColorImage* image,const char *filename, unsigned int maxValueOnChanel){
    int i, j;
    FILE *fp = fopen(filename, "wb");
    (void) fprintf(fp, "P6\n%d %d\n%d\n", image->nx,image->ny, (int) maxValueOnChanel);
    for (i = 0; i < image->ny; i++){
        for (j = 0; j < image->nx; j++){
            static unsigned char color[3];
            color[0] = image->cor[i][j].val[0];
            color[1] = image->cor[i][j].val[1];
            color[2] = image->cor[i][j].val[2];
            (void) fwrite(color, 1, 3, fp);
        }
        //(void) fprintf(fp,"\n");
    }
    (void) fclose(fp);
}

void writeImage(ColorImage* image,const char *filename, const char *magicNumber,unsigned int maxValueOnChanel){
    //ASCII
    if (strcmp(magicNumber,"P3")==0){
        writeImageAsASCII(image,filename,maxValueOnChanel);
    }//Binary
    else if(strcmp(magicNumber,"P6")==0){
        writeImageAsBinary(image,filename,maxValueOnChanel);
    }else{
        fprintf(stderr,"magic number desconhecido. Por favor, acesse https://en.wikipedia.org/wiki/Netpbm_format "
                "para saber mais sobre");
    }
}

void writeImage(GrayImage* image,const char *filename, const char *magicNumber,unsigned int maxValueOnChanel){
    //ASCII
    if (strcmp(magicNumber,"P2")==0){
        writeImageAsASCII(image,filename,maxValueOnChanel);
    }//Binary
    else if(strcmp(magicNumber,"P5")==0){
        writeImageAsBinary(image,filename,maxValueOnChanel);
    }else{
        fprintf(stderr,"magic number desconhecido. Por favor, acesse https://en.wikipedia.org/wiki/Netpbm_format "
                "para saber mais sobre");
    }
}

void writeImage(GrayImage* image,const char *filename){
    writeImage(image,filename, "P5",255);
}

void writeImage(ColorImage* image,const char *filename){
    writeImage(image,filename, "P6",255);
}

ColorImage *RGBtoYCbCr(ColorImage *rgb)
{
    ColorImage *ycbcr;
    float a = 16.0;
    float b = 128.0;

    ycbcr = createColorImage(rgb->nx,rgb->ny);

    for (int y=0; y < rgb->ny; y++)
        for (int x=0; x < rgb->nx; x++){
            float lum  = (0.257*(float)rgb->cor[y][x].val[0]+
                          0.504*(float)rgb->cor[y][x].val[1]+
                          0.098*(float)rgb->cor[y][x].val[2]+a);
            float Cb = (-0.148*(float)rgb->cor[y][x].val[0]+
                        -0.291*(float)rgb->cor[y][x].val[1]+
                        0.439*(float)rgb->cor[y][x].val[2]+b);
            float Cr = (0.439*(float)rgb->cor[y][x].val[0]+
                        -0.368*(float)rgb->cor[y][x].val[1]+
                        -0.071*(float)rgb->cor[y][x].val[2]+b);

            if (lum < 0)   lum = 0.0;
            if (lum > 255) lum = 255;
            if (Cb < 0)   Cb = 0.0;
            if (Cb > 255) Cb = 255;
            if (Cr < 0)   Cr = 0.0;
            if (Cr > 255) Cr = 255;

            ycbcr->cor[y][x].val[0] = lum;
            ycbcr->cor[y][x].val[1] = Cb;
            ycbcr->cor[y][x].val[2] = Cr;

        }

    return(ycbcr);
}

GrayImage* extractColorChannelAsGrayImage(ColorImage* colorImage,int channel){
    GrayImage* grayImage= createGrayImage(colorImage->nx,colorImage->ny);
    int k=0;
    for (int i = 0; i < colorImage->ny; ++i) {
        for (int j = 0; j < colorImage->nx; ++j) {
            grayImage->val[k] = colorImage->cor[i][j].val[channel];
            k++;
        }
    }
    return grayImage;
}

Image* createImage(int nx, int ny,int nchannels){
    Image* image = (Image*)calloc(1,sizeof(Image));
    image->nx = nx;
    image->ny = ny;
    image->nz = 1;
    image->numberPixels = image->nx*image->ny*image->nz;
    image->dx = 0;
    image->dy = 0;
    image->dz = 0;
    image->scalingFactor = 255;
    image->unid[0] = 'm';
    image->unid[1] = 'm';
    image->channel = (float**)calloc(nchannels,sizeof(float*));
    image->nchannels = nchannels;
    image->colorSpace = UNKNOWN;
    image->dataTypeId = FLOAT;
    for (int i = 0; i < image->nchannels; ++i) {
        image->channel[i] = (float*)calloc(image->numberPixels,sizeof(float));
        //image->channel[i] = (float*)calloc(image->numberPixels,sizeof(float*));
        //image->channel->floatChannel[i] = (float*)calloc(image->numberPixels,sizeof(float));
    }
    return image;
}

Image* createImage(int nx, int ny,int nz, int nchannels){
    Image* image = (Image*)calloc(1,sizeof(Image));
    image->nx = nx;
    image->ny = ny;
    image->nz = nz;
    image->numberPixels = image->nx*image->ny*image->nz;
    image->dx = 0;
    image->dy = 0;
    image->dz = 0;
    image->scalingFactor = 255;
    image->unid[0] = 'm';
    image->unid[1] = 'm';
    image->channel = (float**)calloc(nchannels,sizeof(float*));
    image->nchannels = nchannels;
    image->colorSpace = UNKNOWN;
    image->dataTypeId = FLOAT;
    for (int i = 0; i < image->nchannels; ++i) {
        image->channel[i] = (float*)calloc(image->numberPixels,sizeof(float));
        //image->channel[i] = (float*)calloc(image->numberPixels,sizeof(float*));
        //image->channel->floatChannel[i] = (float*)calloc(image->numberPixels,sizeof(float));
    }
    return image;
}



Image* createImage(int nx, int ny){
    Image* image = (Image*)calloc(1,sizeof(Image));
    image->nx = nx;
    image->ny = ny;
    image->numberPixels = nx*ny;
    image->dx = 0;
    image->dy = 0;
    image->scalingFactor = 255;
    image->unid[0] = 'm';
    image->unid[1] = 'm';
    image->channel = (float**)calloc(1,sizeof(float*));
    image->nchannels = 1;
    image->colorSpace = UNKNOWN;
    image->dataTypeId = FLOAT;
    for (int i = 0; i < image->nchannels; ++i) {
        image->channel[i] = (float*)calloc(image->numberPixels,sizeof(float*));
    }
    return image;
}

void destroyImage(Image**image ){
    Image* auxPointer = *image;
    if(auxPointer == NULL){
        return;
    }
    for (int i = 0; i < auxPointer->nchannels; ++i) {
        free(auxPointer->channel[i]);
    }
    free(auxPointer);
    auxPointer = NULL;
}

Image* readImage(char *filename){
    char ext[10],*pos;
    pos = strrchr(filename,'.') + 1;
    sscanf(pos,"%s",ext);
    Image* image = NULL;
    //gray image
    if (strcmp(ext,"pgm")==0){
        image = readImagePGM(filename);
    }else if(strcmp(ext,"ppm")==0){
        image = readImagePPM(filename);
    }else{
        printf("image format unknown\n");
    }
    return image;
}

Image *readImagePGM(char *filename)
{
    FILE *fp=NULL;
    unsigned char *value=NULL;
    char type[10];
    int  maxValue,ncols,nrows,n;
    Image *img = NULL;
    char z[256];

    fp = fopen(filename,"rb");
    if (fp == NULL){
        fprintf(stderr,"Cannot open %s\n",filename);
        exit(-1);
    }
    fscanf(fp,"%s\n",type);
    NCFgets(z,255,fp);
    if((strcmp(type,"P5")==0)){
        int c = getc(fp);
        while (c == '#') {
            while (getc(fp) != '\n');
            c = getc(fp);
        }
        ungetc(c, fp);
        sscanf(z,"%d %d\n",&ncols,&nrows);
        n = ncols*nrows;
        NCFgets(z,255,fp);
        sscanf(z,"%d\n",&maxValue);
        fgetc(fp);
        value = (unsigned char *)calloc(n,sizeof(unsigned char));
        if (value != NULL){
            fread(value,sizeof(unsigned char),n,fp);
        }else{
            fprintf(stderr,"Insufficient memory in ReadImage\n");
            exit(-1);
        }
        fclose(fp);
        img = createImage(ncols,nrows,1);
        img->scalingFactor = maxValue;
        img->colorSpace = GRAYSCALE;
        for (int i=0; i < n; i++)
            img->channel[0][i]=(float)value[i];
        free(value);
    }else{
        if((strcmp(type,"P2")==0)){
            int c = getc(fp);
            while (c == '#') {
                while (getc(fp) != '\n');
                c = getc(fp);
            }
            ungetc(c, fp);
            sscanf(z,"%d %d\n",&ncols,&nrows);
            n = ncols*nrows;
            NCFgets(z,255,fp);
            sscanf(z,"%d\n",&maxValue);
            img = createImage(ncols,nrows,1);
            img->scalingFactor = maxValue;
            img->colorSpace = GRAYSCALE;

            for (int i=0; i < n; i++)
                fscanf(fp,"%f",&img->channel[0][i]);
            fclose(fp);
        }else{
            fprintf(stderr,"Input image must be P2 or P5\n");
            exit(-1);
        }
    }
    return(img);
}

Image *readImagePPM(char *filename){
    FILE *fp=NULL;
    char type[10];
    int  x,y,nx,ny,Imax;
    Image *I=NULL;
    ushort rgb16[3];
    char z[256];

    fp = fopen(filename,"r");
    if (fp != NULL) {
        fscanf(fp,"%s",type);
        NCFgets(z,255,fp);

        if (strcmp(type,"P6")==0){
            int c = getc(fp);
            while (c == '#') {
                while (getc(fp) != '\n');
                c = getc(fp);
            }
            ungetc(c, fp);

            if (fscanf(fp, "%d %d", &nx, &ny) != 2) {
                fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
                exit(1);
            }
            I = createImage(nx,ny,3);

            if (fscanf(fp, "%d", &Imax) != 1) {
                fprintf(stderr, "Invalid rgb component (error loading '%s')\n",
                        filename);
                exit(1);
            }

            I->scalingFactor = Imax;
            I->colorSpace = RGB;
            while (fgetc(fp) != '\n');

            if (Imax <= 255){
                int iter = 0;
                for (y=0; y < ny; y++) {
                    for (x = 0; x < nx; x++) {
                        I->channel[0][iter] =  (float)fgetc(fp);//red
                        I->channel[1][iter] =  (float)fgetc(fp);//green
                        I->channel[2][iter] =  (float)fgetc(fp);//blue
                        iter++;
                    }
                }
            }else if (Imax > 255 && Imax <= 65535){
                int rgbBitDepth = 256;
                int nbits = 9;

                while ((1 << rgbBitDepth) <= Imax) {
                    nbits++;
                }
                int iter = 0;
                for (y=0; y < ny; y++) {
                    for (x=0; x < nx; x++){
                        if (fread(rgb16, 2, 3, fp) == 3) {
                            I->channel[0][iter]=(((rgb16[0] & 0xff) << 8) | ((ushort) rgb16[0] >> 8));
                            I->channel[1][iter]=(((rgb16[1] & 0xff) << 8) | ((ushort) rgb16[1] >> 8));
                            I->channel[2][iter]=(((rgb16[2] & 0xff) << 8) | ((ushort) rgb16[2] >> 8));
                        } else{
                            Error("Reading 16-bit error","ReadColorImage");
                        }
                        iter++;
                    }
                }
            }
        }else if(strcmp(type,"P3")==0){
            int c = getc(fp);
            while (c == '#') {
                while (getc(fp) != '\n');
                c = getc(fp);
            }
            ungetc(c, fp);
            if (fscanf(fp, "%d %d", &nx, &ny) != 2) {
                fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
                exit(1);
            }

            I = createImage(nx,ny,3);
            I->colorSpace = RGB;

            if (fscanf(fp, "%d", &Imax) != 1) {
                fprintf(stderr, "Invalid rgb component (error loading '%s')\n",
                        filename);
                exit(1);
            }

            I->scalingFactor = Imax;
            int R=0,G=0,B=0;
            int iter=0;
            for (y=0; y < ny; y++) {
                for (x=0; x < nx; x++){
                    (void)fscanf(fp, "%d %d %d",&R,&G,&B);
                    I->channel[0][iter] = R;
                    I->channel[1][iter] = G;
                    I->channel[2][iter] = B;
                    iter++;
                }
            }
        }

        else{
            Error("ColorImage type invalid","ReadColormage");
        }

        fclose(fp);
    }else{
        Error(MSG2,"ReadColorImage");
    }

    return(I);

}

void writeImagePGM(Image *image,char *filename, char *magicNumber){

    if (strcmp(magicNumber,"P2")==0){
        writeImageP2(image,filename);
    }
    else if(strcmp(magicNumber,"P5")==0){
        writeImageP5(image,filename);
    }else{
        printf("unknown magic number");
    }
}

void writeImagePPM(Image *image,char *filename, char *magicNumber){

    if (strcmp(magicNumber,"P3")==0){
        writeImageP3(image,filename);
    }
    else if(strcmp(magicNumber,"P6")==0){
        writeImageP6(image,filename);
    }else{
        printf("unknown magic number");
    }
}


void writeImageP2(Image *image,char *filename){
    int i, j;
    FILE *fp = fopen(filename, "w");
    (void) fprintf(fp, "P2\n%d %d\n%d\n", image->nx,image->ny, image->scalingFactor);
    int k = 0;


    float* channel0 = image->channel[0];
    for (i = 0; i < image->ny; i++){
        for (j = 0; j < image->nx; j++){
            (void) fprintf(fp,"%d ",(int)channel0[k]);
            k++;
        }
        (void) fprintf(fp,"\n");
    }


    (void) fclose(fp);
}

void writeImageP3(Image *image,char *filename){
    int i, j;
    FILE *fp = fopen(filename, "w");
    (void) fprintf(fp, "P3\n%d %d\n%d\n", image->nx,image->ny, image->scalingFactor);
    int k=0;

    if(image->dataTypeId == FLOAT){
        float* channel0 = (float*)image->channel[0];
        float* channel1 = (float*)image->channel[1];
        float* channel2 = (float*)image->channel[2];
        for (i = 0; i < image->ny; i++){
            for (j = 0; j < image->nx; j++){
                (void) fprintf(fp,"%d %d %d ", (int)(channel0[k]),(int)channel1[k],(int)channel2[k]);
                k++;
            }
            (void) fprintf(fp,"\n");
        }
    }
    if(image->dataTypeId == DOUBLE){
        double* channel0 = (double*)image->channel[0];
        double* channel1 = (double*)image->channel[1];
        double* channel2 = (double*)image->channel[2];
        for (i = 0; i < image->ny; i++){
            for (j = 0; j < image->nx; j++){
                (void) fprintf(fp,"%d %d %d ", (int)(channel0[k]),(int)channel1[k],(int)channel2[k]);
                k++;
            }
            (void) fprintf(fp,"\n");
        }
    }
    (void) fclose(fp);

}

void writeImageP5(Image *image,char *filename){
    FILE *fp = fopen(filename, "wb");
    (void) fprintf(fp, "P5\n%d %d\n%d\n", image->nx,image->ny, image->scalingFactor);
    int k = 0;

    float* channel0 = image->channel[0];
    for (int i = 0; i < image->ny; i++){
        for (int j = 0; j < image->nx; j++){
            static unsigned char gray[1];
            gray[0] = (int)channel0[k];
            (void) fwrite(gray, 1, 1, fp);
            k++;
        }
    }

    (void) fclose(fp);
}


void writeImageP6(Image *image,char *filename){
    int i, j;
    FILE *fp = fopen(filename, "wb");
    (void) fprintf(fp, "P6\n%d %d\n%d\n", image->nx,image->ny, image->scalingFactor);
    int k=0;

    float* channel0 = image->channel[0];
    float* channel1 = image->channel[1];
    float* channel2 = image->channel[2];
    for (i = 0; i < image->ny; i++){
        for (j = 0; j < image->nx; j++){
            static unsigned char color[3];
            color[0] = channel0[k];
            color[1] = channel1[k];
            color[2] = channel2[k];
            (void) fwrite(color, 1, 3, fp);
            k++;
        }
    }

    (void) fclose(fp);
}

void writeImage(Image* image,char *filename){
    char ext[10],*pos;
    pos = strrchr(filename,'.') + 1;
    sscanf(pos,"%s",ext);

    if (strcmp(ext,"pgm")==0){
        writeImagePGM(image,filename,"P5");
    }
    else if (strcmp(ext,"ppm")==0){
        writeImagePPM(image,filename,"P6");
    }else{
        printf("unsuported format\n");
    }
}



Image *convertRGBtoYCbCr(Image *rgbImage)
{
    Image *ycbcrImage;
    float a = 16.0;
    float b = 128.0;

    ycbcrImage = createImage(rgbImage->nx,rgbImage->ny,3);
    ycbcrImage->scalingFactor = rgbImage->scalingFactor;
    for (int k=0; k < rgbImage->numberPixels; k++){
        float lum  = (0.257*rgbImage->channel[0][k])+
                     (0.504*rgbImage->channel[1][k])+
                     (0.098*rgbImage->channel[2][k])+a;
        float Cb = (-0.148*rgbImage->channel[0][k]) +
                   (-0.291*rgbImage->channel[1][k]) +
                   (0.439*rgbImage->channel[2][k])+b;
        float Cr = (0.439*rgbImage->channel[0][k]) +
                   (-0.368*rgbImage->channel[1][k]) +
                   (-0.071*rgbImage->channel[2][k]) +b;

        if (lum < 0)   lum = 0.0;
        if (lum > ycbcrImage->scalingFactor) lum = ycbcrImage->scalingFactor;
        if (Cb < 0)   Cb = 0.0;
        if (Cb > ycbcrImage->scalingFactor) Cb = ycbcrImage->scalingFactor;
        if (Cr < 0)   Cr = 0.0;
        if (Cr > ycbcrImage->scalingFactor) Cr = ycbcrImage->scalingFactor;

        ycbcrImage->channel[0][k] = lum;
        ycbcrImage->channel[1][k] = Cb;
        ycbcrImage->channel[2][k] = Cr;
    }

    return(ycbcrImage);
}

GrayImage* convertImage2GrayImage(Image* image){
    GrayImage* grayImage = createGrayImage(image->nx,image->ny);
    for (int k = 0; k < image->numberPixels; ++k) {
        grayImage->val[k] = image->channel[0][k];
    }
    return grayImage;
}

Image* convertGrayImage2Image(GrayImage* grayImage){
    Image* image = createImage(grayImage->ncols,grayImage->nrows,1);
    int maxValue = maximumValue(grayImage);
    if(maxValue < 256){
        image->scalingFactor = 255;
    }else{
        int value = 256;
        while(value-1 < maxValue){
            value = value << 1;
        }
        image->scalingFactor = value-1;
    }

    for (int k = 0; k < image->numberPixels; ++k) {
        image->channel[0][k] = grayImage->val[k];
    }
    return image;
}

ColorImage* convertImage2ColorImage(Image* image){
    ColorImage* colorImage = createColorImage(image->nx,image->ny);
    int k = 0;
    for (int y = 0; y < colorImage->ny; ++y) {
        for (int x = 0; x < colorImage->nx; ++x) {
            colorImage->cor[y][x].val[0] = image->channel[0][k];
            colorImage->cor[y][x].val[1] = image->channel[1][k];
            colorImage->cor[y][x].val[2] = image->channel[2][k];
            k++;
        }
    }
    return colorImage;
}

Image* convertColorImage2Image(ColorImage* colorImage){
    Image* image = createImage(colorImage->nx,colorImage->ny,3);
    int k = 0;
    int maxValue = maximumColorValue(colorImage);
    if(maxValue < 256){
        image->scalingFactor = 255;
    }else{
        int value = 256;
        while(value-1 < maxValue){
            value = value << 1;
        }
        image->scalingFactor = value-1;
    }
    for (int y = 0; y < colorImage->ny; ++y) {
        for (int x = 0; x < colorImage->nx; ++x) {
            image->channel[0][k] = colorImage->cor[y][x].val[0];
            image->channel[1][k] = colorImage->cor[y][x].val[1];
            image->channel[2][k] = colorImage->cor[y][x].val[2];
            k++;
        }
    }
    return image;
}

float sumUpAllPixelsValues(Image *image, bool normalize){
    double sum = 0;
    for (int k = 0; k < image->numberPixels; ++k) {
        for (int c = 0; c < image->nchannels; ++c) {
            sum += image->channel[c][k];
        }
    }
    if(normalize){
        sum /= ( image->numberPixels * image->nchannels);
    }
    return ((float)sum);
}

Image* extractImageChannel(Image* image, int channel){
    Image* channelImage = createImage(image->nx,image->ny,1);
    channelImage->scalingFactor = image->scalingFactor;
    for (int k = 0; k < image->numberPixels; ++k) {
        channelImage->channel[0][k] = image->channel[channel][k];
    }
    return channelImage;
}

void printImage(Image* image){
    int index=0;
    for (int y = 0; y < image->ny; ++y) {
        for (int x = 0; x < image->nx; ++x) {
            for (int c = 0; c < image->nchannels; ++c) {
                index = (y*image->nx)+x;
                printf("%.2f ",image->channel[c][index]);
            }
        }
        printf("\n");
    }
}

Image* createAlphaChannel(Image* image,float alpha){
    Image *output = createImage(image->nx,image->ny,image->nchannels+1);
    output->scalingFactor = image->scalingFactor;
    for (int k = 0; k < image->numberPixels; ++k) {
        for (int c = 0; c < image->nchannels; ++c) {
            output->channel[c][k] = image->channel[c][k];
        }
        output->channel[image->nchannels][k] = alpha;
    }
    return output;
}

void addUniformNoise(Image* image, float uniformValue, double probability){
    for (int k = 0; k < image->numberPixels; ++k) {
        if(randomNormalized() <=  probability){
            for (int c = 0; c < image->nchannels; ++c) {
                image->channel[c][k] += uniformValue;
                if(image->channel[c][k] > image->scalingFactor){
                    image->channel[c][k] = image->scalingFactor;
                }
            }
        }
    }
}

void addSaltAndPepperNoise(Image* image, double probability){
    for (int k = 0; k < image->numberPixels; ++k) {
        if(randomNormalized() <=  probability){
            for (int c = 0; c < image->nchannels; ++c) {
                //salt
                if(randomNormalized() >= 0.5){
                    image->channel[c][k] = image->scalingFactor;
                }else{
                    image->channel[c][k] = 0;
                }
            }
        }

    }
}

void addAdditiveGaussianNoise(Image* image, double mean, double variance){


    double standardDeviation = sqrt(variance);
    for (int y = 0; y < image->ny; ++y) {
        for (int x = 0; x < image->nx; ++x) {
            double value = generateGaussianNoise(0,standardDeviation);
            value = value*image->scalingFactor;
            imageVal(image,x,y) += value;
            if(imageVal(image,x,y) < 0){
                imageVal(image,x,y) = 0;
            }
            if(imageVal(image,x,y) > image->scalingFactor){
                imageVal(image,x,y) = image->scalingFactor;
            }
        }
    }
}

uint8_t* convertImage2IntergerArray8bits(Image* imageRGBA){
    uint8_t* output = (uint8_t*)calloc(imageRGBA->nchannels*imageRGBA->nx*imageRGBA->ny ,sizeof(uint8_t));
    long index = 0;
    for (int k = 0; k < imageRGBA->numberPixels; ++k) {
        for (int c = 0; c < imageRGBA->nchannels; ++c) {
            output[index] = (uint8_t)imageRGBA->channel[c][k];
            index++;
        }
    }
    return output;
}

Image* copyImage(Image* image, bool copyChannels){
    Image* imageCopy = createImage(image->nx,image->ny,image->nz,image->nchannels);
    imageCopy->colorSpace = image->colorSpace;
    imageCopy->dx = image->dx;
    imageCopy->dy = image->dy;
    imageCopy->dz = image->dz;
    imageCopy->dataTypeId = image->dataTypeId;
    imageCopy->scalingFactor = image->scalingFactor;
    for (int i = 0; i < 10; ++i) {
        imageCopy->unid[i] = image->unid[i];
    }
    if(copyChannels) {
        for (int k = 0; k < image->numberPixels; ++k) {
            for (int c = 0; c < image->nchannels; ++c) {
                imageCopy->channel[c][k] = image->channel[c][k];
            }
        }
    }
    return imageCopy;
}

Image *imageSubtraction(Image *image1, Image *image2, bool saturation){
    Image *outputImage = NULL;
    if(isImagesSameDomain(image1,image2)){
        outputImage = createImage(image1->nx,image1->ny,image1->nchannels);
        int n = image1->nx*image1->ny;
        int result;
        for (int i = 0; i < n; ++i) {
            for(int c=0; c<image1->nchannels; c++){
                result = image1->channel[c][i] - image2->channel[c][i];
                if(saturation){
                    result = (result<0)?0:result;
                }
                outputImage->channel[c][i] = result;
            }

        }
    }
    return outputImage;
}

Image* readImagesFromDirectory(DirectoryManager* directoryManager){
    Image* imagePack = NULL;
    Image* currentImage = NULL;
    Image* merged = NULL;
    for (int i = 0; i < (int)directoryManager->nfiles; ++i) {
        currentImage = readImage(directoryManager->files[i]->path);
        merged = mergeImages(imagePack,currentImage);
        destroyImage(&currentImage);
        destroyImage(&imagePack);
        imagePack = copyImage(merged,true);
        destroyImage(&merged);
    }
    return imagePack;
}

Image* convertGrayImage2RGBImage(Image* image_ppm){ 
    Image* image_rgb = createImage(image_ppm->nx,image_ppm->ny,3);
    for (int k = 0; k < image_rgb->numberPixels; ++k) {
        image_rgb->channel[0][k] = image_ppm->channel[0][k];
        image_rgb->channel[1][k] = image_ppm->channel[0][k];
        image_rgb->channel[2][k] = image_ppm->channel[0][k];
    }
    return image_rgb;
}

Image* mergeImages(Image* image1, Image* image2){
    Image* imageMerged = NULL;
    if(image1 == NULL && image2 == NULL){
        printf("NULL images\n");
        return NULL;
    }
    if(image1 == NULL){
        imageMerged = copyImage(image2,true);
        return imageMerged;
    }
    if(image2 == NULL){
        imageMerged = copyImage(image1,true);
        return imageMerged;
    }

    if(image1->nx != image2->nx || image1->ny != image2->ny){
        printf("images dimensions mismatch\n");
        return NULL;
    }
    if(image1->nchannels != image2->nchannels){
        printf("number of channels mismatch\n");
        return NULL;
    }
    int totalSlices = image1->nz+image2->nz;
    imageMerged = createImage(image1->nx,image1->ny,totalSlices,image1->nchannels);

#pragma omp parallel for
    for (int k = 0; k < image1->numberPixels; ++k) {
        for (int c = 0; c < image1->nchannels; ++c) {
            imageMerged->channel[c][k] = image1->channel[c][k];
        }
    }

    for (int k = 0,kShift = image1->numberPixels; k < image2->numberPixels; ++k,++kShift) {
        for (int c = 0; c < image2->nchannels; ++c) {
            imageMerged->channel[c][kShift] = image2->channel[c][k];
        }
    }
    return imageMerged;
}

Image* getSlice(Image* image, int sliceIndex){
    if(sliceIndex >= image->nz || sliceIndex < 0){
        printf("invalid sliceIndex (z-coordinate) value\n");
        return NULL;
    }
    Image* slice = createImage(image->nx,image->ny,image->nchannels);
    for (int y = 0; y < image->ny; ++y) {
        for (int x = 0; x < image->nx; ++x) {
            for (int c = 0; c < image->nchannels; ++c) {
                imageValCh(slice,x,y,c) = imageVolumeCh(image,x,y,sliceIndex,c);
            }
        }
    }
    return slice;
}

Image* extractSubImage(Image*image, int xCoord,int yCoord, int xsize, int ysize, bool zeroPadding){
    Image *subImage = NULL;
    int imageX = xCoord+xsize;
    int imageY = yCoord+ysize;
    int subImageSizeX = 0;
    int subImageSizeY = 0;
    
    subImageSizeX = xsize;
    subImageSizeY = ysize;
    
    if(image->nx <= xCoord && image->ny <= yCoord){
        printf("Warning: xCoord and yCoord are outbound.\n");
    }
    
    if(image->nx < imageX){
        if(!zeroPadding){
            subImageSizeX = image->nx-xCoord;
            if(subImageSizeX <= 0){
                printf("Impossible create a patch without zero padding\n");
                return NULL;
            }
        }
    }
    if(image->ny < imageY){
        if(!zeroPadding){
            subImageSizeY = image->ny-yCoord;
            if(subImageSizeY <= 0){
                printf("Impossible create a patch without zero padding\n");
                return NULL;
            }
        }
    }
    
    subImage = createImage(subImageSizeX,subImageSizeY,image->nchannels);
    for (int y = 0; y < subImage->ny; ++y) {
        for (int x = 0; x < subImage->nx; ++x) {
            for (int c = 0; c < subImage->nchannels; ++c) {
                if(isValidPixelCoordinate(image,xCoord+x,yCoord+y)){
                    imageValCh(subImage,x,y,c) = imageValCh(image,(xCoord+x),(yCoord+y),c);
                }
            }
        }
    }
    return subImage;
}
