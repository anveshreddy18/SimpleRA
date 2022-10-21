#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD() // This function is edited
{
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() != 2 && tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    if(tokenizedQuery.size()==3)
    {
        if(tokenizedQuery[1] == "MATRIX")
        {
            parsedQuery.loadRelationName = tokenizedQuery[2];
            parsedQuery.loadType = "Matrix";
        }
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    else
    {
        parsedQuery.loadRelationName = tokenizedQuery[1];
        parsedQuery.loadType = "Table";
    }
    return true;
}

bool semanticParseLOAD() // Edited.. added matrix functionality
{
    if(parsedQuery.loadType == "Table")
    {
        logger.log("semanticParseLOAD_TABLE");
        if (tableCatalogue.isTable(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Relation already exists" << endl;
            return false;
        }

        if (!isFileExists(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
    }
    else if(parsedQuery.loadType == "Matrix")
    {
        logger.log("semanticParseLOAD_MATRIX");
        if (matrixCatalogue.isMatrix(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Matrix already exists" << endl;
            return false;
        }

        if (!isFileExists(parsedQuery.loadRelationName))
        {
            cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
            return false;
        }
    }
    return true;
}

void executeLOAD()  // added matrix functionality
{
    if(parsedQuery.loadType == "Table")
    {
        logger.log("executeLOAD_TABLE");
        Table *table = new Table(parsedQuery.loadRelationName);
        if (table->load())
        {
            tableCatalogue.insertTable(table);
            cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
        }
    }
    else if(parsedQuery.loadType == "Matrix")
    {
        logger.log("executeLOAD_MATRIX");
        Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
        if (matrix->load())
        {
            matrixCatalogue.insertMatrix(matrix);
            cout << "Loaded Matrix. Column Count: " << matrix->columnCount << " Row Count: " << matrix->rowCount << endl;
        }
    }
    return;
}