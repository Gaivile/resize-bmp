#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char* argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    int n = atoi(argv[1]);
    char* infile = argv[2];
    char* outfile = argv[3];
    
    if (n < 1 || n > 100){
        printf("N should be between 1 and 100\n");
        return 1;
    }
 
    // open input file 
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w + r");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // variables for original image
    int oldwidth = bi.biWidth;
    int oldheight = abs(bi.biHeight);
    int oldpadding =  (4 - (oldwidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    // variables for new image's headers
    bi.biWidth = bi.biWidth * n;
    bi.biHeight = bi.biHeight * n;
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    bi.biSizeImage = (bi.biWidth * abs(bi.biHeight)) * sizeof(RGBTRIPLE) + padding * abs(bi.biHeight);
    bf.bfSize = (bi.biSizeImage) + 54;

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);                         

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(oldheight); i < biHeight; i++)
    {
        // make n lines
        for (int l = 0; l < n; l++)
        {
            // indicates file position to read from
            fseek(inptr, (oldwidth * sizeof(RGBTRIPLE) + oldpadding) * i + 54, SEEK_SET);
            
            // iterate over pixels in scanline                                  
            for (int j = 0; j < oldwidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;
    
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
    
                for (int t = 0; t < n; t++)
                {    
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }
    
            // skip over padding, if any
            fseek(inptr, oldpadding, SEEK_CUR);
            
            // then add it back (to demonstrate how)
            for (int k = 0; k < padding; k++)
            {
                fputc(0x00, outptr);
            }
        }
    }
    
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // that's all folks
    return 0;
}
