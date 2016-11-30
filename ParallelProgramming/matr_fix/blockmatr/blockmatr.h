
#include <iostream>
#include <cstring>

#define BLOCKSIZE 8
#define MIN(x,y) (x<y ? x: y)

template<class T> class blockmatrix {
public:
    blockmatrix(size_t lines, size_t columns): lines(lines), columns(columns) { // subject to change
        blocksPerLine = lines / BLOCKSIZE + 1;
        blocksPerColumn = columns / BLOCKSIZE + 1;

        blocks = new T*[blocksPerColumn*blocksPerLine];
        for (size_t i = 0; i < blocksPerColumn*blocksPerLine; i++){
            blocks[i] = new T[BLOCKSIZE*BLOCKSIZE];
            memset(blocks[i], 0, BLOCKSIZE*BLOCKSIZE*sizeof(T));
        }
    }

    ~blockmatrix() { // subhect to change
        for (size_t i = 0; i < blocksPerLine*blocksPerColumn; i++)
            delete[] blocks[i];
        delete [] blocks;
    }

    T * operator()(size_t line, size_t col) { // *acc(0,0) = 2.34; *acc(i,j) = *acc(i,j+1);
        size_t blockLine = line / BLOCKSIZE;
        size_t blockCol = col / BLOCKSIZE;

        size_t relativeLine = line % BLOCKSIZE;
        size_t relativeCol = col % BLOCKSIZE;

        T *block = blocks[blockLine*blocksPerColumn + blockCol];
        //printf("%p ", block);

        //printf("Access to: %d %d-%d %d(%p)\n", blockLine, blockCol, relativeLine, relativeCol, block + relativeLine*BLOCKSIZE + relativeCol);
        return block + relativeLine*BLOCKSIZE + relativeCol;
    }

    // subject to change
    bool mul(blockmatrix const &left, blockmatrix const &right) {
        if (left.columns != right.lines) return false;
        if (lines != left.lines) return false;
        if (columns != right.columns) return false;

        size_t block_i, block_j, block_k;
        size_t i, j, k;
        size_t size_i, size_j, size_k;
        size_t sum;

        //AX=left.col, AY=left.lin, BX=right.col, BY=right.lin

        int counter = 0;

        for (block_j = 0; block_j < right.columns; block_j += BLOCKSIZE) { //X-dimension of B
            for (block_k = 0; block_k < right.lines; block_k += BLOCKSIZE) { //X-dimension of A or Y-dimension of B
                for (block_i = 0; block_i < left.lines; block_i += BLOCKSIZE) {
                    size_i = MIN(block_i + BLOCKSIZE, left.lines) - block_i;
                    size_j = MIN(block_j + BLOCKSIZE, right.columns) - block_j;
                    size_k = MIN(block_k + BLOCKSIZE, right.lines) - block_k;

                    T *leftBlock = left.blocks[block_i/BLOCKSIZE*left.blocksPerColumn + block_k/BLOCKSIZE];
                    T *rightBlock = right.blocks[block_k/BLOCKSIZE*right.blocksPerColumn + block_j/BLOCKSIZE];

                    //printf("Left block : %d %d(%p)\n", block_i/BLOCKSIZE, block_k/BLOCKSIZE, leftBlock);
                    //printf("Right block: %d %d(%p)\n", block_k/BLOCKSIZE, block_j/BLOCKSIZE, rightBlock);

                    for (i = 0; i < size_i; i++) { //X-dimension of C
                        for (j = 0; j < size_j; j++) { //Y-dimension of C
                            sum = 0;
                            for (k = 0; k < size_k; k++) { //Mult dimension of block
                                counter++;
                                //printf("\tLeft  access from: %d %d->%lg(%p)\n", i, k, leftBlock[i*BLOCKSIZE + k], &leftBlock[i*BLOCKSIZE + k]);
                                //printf("\tRight access from: %d %d->%lg(%p)\n", k, j, rightBlock[i*BLOCKSIZE + k], &rightBlock[i*BLOCKSIZE + k]);

                                sum += leftBlock[i*BLOCKSIZE + k] * rightBlock[k*BLOCKSIZE + j];
                            }
                            *this->operator()(block_i+i, block_j+j) += sum;
                        }
                    }
                }
            }
        }

        /*for (size_t i = 0; i < lines; i++) {
            for (size_t j = 0; j < columns; j++) {
                T acc = (T)0;
                for (size_t k = 0; k < left.columns; k++) {
                    counter++;
                    acc += left.body[k][i] * right.body[k][j];
                }
                body[i][j] = acc;
            }
        }*/
        return true;
    }

    bool add(blockmatrix const &left, blockmatrix const &right);
    static void tune(size_t arg) { // subject for change
        ;
    }
private:
    T ** blocks;
    size_t lines, columns;
    size_t blocksPerLine, blocksPerColumn;
};
