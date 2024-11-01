#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include "heatdis/SZp_heatdis_entry.hpp"
#include "utils.hpp"

int main(int argc, char **argv)
{
    int argv_id = 1;
    size_t dim1 = atoi(argv[argv_id++]);
    size_t dim2 = atoi(argv[argv_id++]);
    int blockSize = dim2;
    double errorBound = atof(argv[argv_id++]);
    int max_iter = atoi(argv[argv_id++]);
    size_t nbEle = dim1 * dim2;

    float * oriData = (float *)calloc(nbEle, sizeof(float));
    initData(dim1, dim2, oriData);
    unsigned char *cmpData = (unsigned char *)calloc(4 * nbEle, sizeof(unsigned char));
    size_t cmpSize = 0;
    SZp_compress_rowwise_1d_block(oriData, cmpData, dim1, dim2, blockSize, errorBound, &cmpSize);
    float * h = (float *)calloc(nbEle, sizeof(float));
    doWork(dim1, dim2, max_iter, oriData, h);

    // DOC
    float * decData = (float *)malloc(nbEle * sizeof(float));
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    for(int i=0; i<max_iter; i++){
        SZp_decompress_rowwise_1d_block(decData, cmpData, dim1, dim2, blockSize, errorBound);
        doWork(dim1, dim2, decData, h);
        SZp_compress_rowwise_1d_block(decData, cmpData, dim1, dim2, blockSize, errorBound, &cmpSize);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    writefile("decdata.doc.dat", cmpData, cmpSize);
    free(cmpData);
    // read compressed file
    size_t num;
    std::vector<unsigned char> compressed = readfile<unsigned char>("decdata.doc.dat", num);
    SZp_decompress_rowwise_1d_block(decData, compressed.data(), dim1, dim2, blockSize, errorBound);
    double max_err = verify(oriData, decData, dim1, dim2);
    double elapsed_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000;
    printf("elapsed_time = %.6f\n", elapsed_time);
    printf("cr = %f, max_err = %.14f\n", 1.0 * sizeof(float) * nbEle / cmpSize, max_err);

    free(h);
    free(oriData);
    free(decData);

    return 0;
}
