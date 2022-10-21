#include "global.h"
/**
 * @brief 
 * SYNTAX: PRINT relation_name
 */
bool syntacticParsePRINT()
{
    logger.log("syntacticParsePRINT");
    if ((tokenizedQuery.size() != 2) && (tokenizedQuery.size() != 3))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = PRINT;
    if(tokenizedQuery.size() == 3)
    {
        if(tokenizedQuery[1] == "MATRIX")
        {
            parsedQuery.printType = "Matrix";
            parsedQuery.printRelationName = tokenizedQuery[2];   
        }
        else 
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    else
    {
        parsedQuery.printType = "Table";
        parsedQuery.printRelationName = tokenizedQuery[1];
    }
    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");
    if(matrixCatalogue.isMatrix(parsedQuery.printRelationName))
    {
        parsedQuery.printType = "Matrix";
        return true;
    }
    else if (tableCatalogue.isTable(parsedQuery.printRelationName))
    {
        parsedQuery.printType = "Table";
        return true;
    }
    else 
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

void executePRINT()
{
    if(parsedQuery.printType == "Table")
    {
        logger.log("executePRINT_Table");
        Table* table = tableCatalogue.getTable(parsedQuery.printRelationName);
        table->print();
    }
    else
    {
        logger.log("executePRINT_Matrix");
        Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
        matrix->print();
    }
    return;
}
