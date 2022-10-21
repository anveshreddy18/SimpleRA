#include "global.h"

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates matrix
 * statistics.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if(getline(fin,line))
    {
        fin.close();
        this->columnStatistics(line);

        if (this->blockify())
            return true;
        return false;
    }
}

/**
 * @brief Function finds the column count and maxElemsPerBlock count from the first line of the .csv data
 * file. 
 */

void Matrix::columnStatistics(string firstLine)
{
    logger.log("Matrix:: columnStatistics");
    string word;
    stringstream s(firstLine);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        this->columnCount++;
    }
    this->maxElemsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int)));;    
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> row(this->columnCount, 0);
    queue<int> temp;
    vector<int>blockFill;
    unordered_set<int> dummy;
    dummy.clear();
    this->distinctValuesInColumns.assign(this->columnCount, dummy);
    this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
            temp.push(row[columnCounter]);
        }
        this->updateStatistics(row);
        while(temp.size()>=this->maxElemsPerBlock)
        {
            // Page is filled
            blockFill.clear();
            while(blockFill.size()<this->maxElemsPerBlock)
            {
                blockFill.push_back(temp.front());
                temp.pop();
            }
            bufferManager.writePage(this->matrixName, this->blockCount, blockFill);
            this->blockCount++;

        }
    }
    if (temp.size())
    {
        // Page is partially filled
        blockFill.clear();
        while(temp.size())
        {
            blockFill.push_back(temp.front());
            temp.pop();
        }
        bufferManager.writePage(this->matrixName, this->blockCount, blockFill);
        this->blockCount++;
    }

    if (this->rowCount == 0)
        return false;
    this->distinctValuesInColumns.clear();
    return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Matrix::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}


/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings
    // this->writeRow(this->columns, cout);

    // logger.log("urke checking => " + this->matrixName);
    Cursor cursor(this->matrixName, 0);

    int n = this->columnCount;
    vector<int> blockFill;
    vector<int>v;
    // fout.open(cout,ios::app);
    for(int i=0;i<min(n,20);i++)
    {
        for(int j=0;j<min(n,20);j++)
        {
            int val = i*n+j;
            int block_no = (val/(this->maxElemsPerBlock));
            int elem_no = val%(this->maxElemsPerBlock);
            cursor.nextPage(block_no);
            Page page = cursor.page;
            blockFill = page.getRow(0);
            v.push_back(blockFill[elem_no]);
        }
        this->writeRow(v, cout);
        v.clear();
    }
    printRowCount(this->rowCount);
}



/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

        if (cursor->pageIndex < this->blockCount - 1)
        {
            cursor->nextPage(cursor->pageIndex+1);
        }
}



/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    // this->writeRow(this->columns, fout);

    Cursor cursor(this->matrixName, 0);
    int n = this->columnCount;
    vector<int>blockFill;
    vector<int>v;
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            int val = i*n+j;
            int block_no = (val/(this->maxElemsPerBlock));
            int elem_no = val%(this->maxElemsPerBlock);
            cursor.nextPage(block_no);
            Page page = cursor.page;
            blockFill = page.getRow(0);
            v.push_back(blockFill[elem_no]);
        }
        this->writeRow(v, fout, "EXPORT");
        v.clear();
    }
    fout.close();
}

/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
    return true;
    return false;
}

/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 * 
 * @return Cursor 
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0);
    return cursor;
}
