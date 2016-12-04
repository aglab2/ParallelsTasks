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

        for (block_j = 0; block_j < right.columns; block_j += BLOCKSIZE) { //X-dimension of B
            for (block_k = 0; block_k < right.lines; block_k += BLOCKSIZE) { //X-dimension of A or Y-dimension of B
                for (block_i = 0; block_i < left.lines; block_i += BLOCKSIZE) {
                    size_i = MIN(block_i + BLOCKSIZE, left.lines) - block_i;
                    size_j = MIN(block_j + BLOCKSIZE, right.columns) - block_j;
                    size_k = MIN(block_k + BLOCKSIZE, right.lines) - block_k;

                    T *leftBlock = left.blocks[block_i/BLOCKSIZE*left.blocksPerColumn + block_k/BLOCKSIZE];
                    T *rightBlock = right.blocks[block_k/BLOCKSIZE*right.blocksPerColumn + block_j/BLOCKSIZE];

                    for (i = 0; i < size_i; i++) { //X-dimension of C
                        for (j = 0; j < size_j; j++) { //Y-dimension of C
                            sum = 0;
                            for (k = 0; k < size_k; k++) { //Mult dimension of block
                                sum += leftBlock[i*BLOCKSIZE + k] * rightBlock[k*BLOCKSIZE + j];
                            }
                            *this->operator()(block_i+i, block_j+j) += sum;
                        }
                    }
                }
            }
        }

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
