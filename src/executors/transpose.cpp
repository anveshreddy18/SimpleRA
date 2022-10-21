#include "global.h"


bool syntacticParseCROSS_TRANSPOSE()
{
    logger.log("syntacticParseCROSS_TRANSPOSE");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.printType = "Matrix";
    parsedQuery.queryType = CROSS_TRANSPOSE;
    parsedQuery.crossTransposeFirstRelationName = tokenizedQuery[1];
    parsedQuery.crossTransposeSecondRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseCROSS_TRANSPOSE()
{
    logger.log("semanticParseCROSS_TRANSPOSE");
    string A = parsedQuery.crossTransposeFirstRelationName;
    string B = parsedQuery.crossTransposeSecondRelationName;
    // if either first or second or both doesn't exist in the data, an error is thrown
    if (!isFileExists(A) || !isFileExists(B))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    // Check whether they both are in the Matrix catalogue or not
    if(!matrixCatalogue.isMatrix(A) || !matrixCatalogue.isMatrix(B))
    {
        cout << "SEMANTIC ERROR: One or both Matrices are not loaded" << endl;
        return false;
    }
    if(A == B)
    {
        parsedQuery.sameMatrices = true;
    }
    else
        parsedQuery.sameMatrices = false;
    return true;
}

void executeCROSS_TRANSPOSE()
{
    logger.log("executeCROSS_TRANSPOSE");
    string A = parsedQuery.crossTransposeFirstRelationName;
    string B = parsedQuery.crossTransposeSecondRelationName;
    Matrix *matrixA = matrixCatalogue.getMatrix(A);
    Matrix *matrixB = matrixCatalogue.getMatrix(B);
    int n = matrixA->columnCount;
    Cursor cursorA(A,0);
    Cursor cursorB(B,0);
    int j_limit = n;
    for(int i=0;i<n;i++)
    {
        if(parsedQuery.sameMatrices)
        {
            j_limit = i;
        }
        for(int j=0;j<j_limit;j++)
        {
            int indA = i*n+j;
            int indB = j*n+i;
            
            int block_no_A = (indA/(matrixA->maxElemsPerBlock));
            int elem_no_A = indA%(matrixA->maxElemsPerBlock);

            int block_no_B = (indB/(matrixB->maxElemsPerBlock));
            int elem_no_B = indB%(matrixB->maxElemsPerBlock);
            
            cursorA.nextPage(block_no_A);
            Page pageA = cursorA.page;

            cursorB.nextPage(block_no_B);
            Page pageB = cursorB.page;
           
            int elemA = pageA.getRow(0)[elem_no_A];
            int elemB = pageB.getRow(0)[elem_no_B];

            pageA.editPage(elem_no_A, elemB);
            if(parsedQuery.sameMatrices)
                pageA.editPage(elem_no_B,elemA);
            else
                pageB.editPage(elem_no_B, elemA);

            bufferManager.clear();

        }
    }
}
