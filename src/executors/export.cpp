#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() != 2 && tokenizedQuery.size()!=3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT;
    if(tokenizedQuery.size()==3)
    {
        if(tokenizedQuery[1] == "MATRIX")
        {
            parsedQuery.exportType = "Matrix";
            parsedQuery.exportRelationName = tokenizedQuery[2];
        }
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    else
    {
        parsedQuery.exportType = "Table";
        parsedQuery.exportRelationName = tokenizedQuery[1];
    }
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    if (matrixCatalogue.isMatrix(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if(parsedQuery.exportType == "Table")
    {
        parsedQuery.printType = "Table";
        Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
        table->makePermanent();
    }
    else
    {
        parsedQuery.printType = "Matrix";
        Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);
        matrix->makePermanent();
    }
    return;
}