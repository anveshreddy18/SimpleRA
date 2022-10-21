#include "global.h"

/**
 * @brief 
 * SYNTAX: R <- GROUP BY column_name FROM relation_name RETURN Operation(column_name)
 */

bool syntacticParseGROUP()
{
    logger.log("syntacticParseGROUP");
    if (tokenizedQuery.size() != 9)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    string lastword = tokenizedQuery[8];
    parsedQuery.queryType = GROUP;
    parsedQuery.groupResultantRelationName = tokenizedQuery[0];
    parsedQuery.groupFirstColumnName = tokenizedQuery[4];
    parsedQuery.groupRelationName = tokenizedQuery[6];
    parsedQuery.groupOperation = lastword.substr(0,3);
    parsedQuery.groupSecondColumnName = lastword.substr(4, lastword.size()-5);
}

bool semanticParseGROUP()
{
    logger.log("semanticParseGROUP");
    // ResultantRelationName should not prior to this
    if(tableCatalogue.isTable(parsedQuery.groupResultantRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    // RelationName should exist in the tableCatelogue
    if(!tableCatalogue.isTable(parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    // Also both first column & second column should exist in the table
    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupFirstColumnName, parsedQuery.groupRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.groupSecondColumnName, parsedQuery.groupRelationName))
    {
        cout << "SEMANTIC ERROR: One or more of the columns doesn't exist in relation" << endl;
        return false;
    }
    // All is well
    return true;

}

void executeGROUP()
{
    logger.log("executeGROUP");
    
    Table table = *(tableCatalogue.getTable(parsedQuery.groupRelationName));

    vector<string> columns;
    columns.emplace_back(parsedQuery.groupFirstColumnName);
    columns.emplace_back(parsedQuery.groupOperation+parsedQuery.groupSecondColumnName);

    Table *resultantTable = new Table(parsedQuery.groupResultantRelationName, columns);
    vector<int>resultantRow;
    resultantRow.reserve(resultantTable->columnCount);

    unordered_map<int, vector<int>>mp;

    int column1 = table.getColumnIndex(parsedQuery.groupFirstColumnName);
    int column2 = table.getColumnIndex(parsedQuery.groupSecondColumnName);

    Cursor cursor = table.getCursor();

    vector<int>row=cursor.getNext();

    while(!row.empty())
    {
        int val1 = row[column1];
        int val2 = row[column2];
        if(mp.find(val1)==mp.end())
        {
            // val1 is not in the map
            mp[val1] = {INT_MIN, INT_MAX, 0, 0};
        }
        mp[val1][0]=max(mp[val1][0], val2);
        mp[val1][1]=min(mp[val1][1], val2);
        mp[val1][2]+=val2;
        mp[val1][3]++;
        row = cursor.getNext();
    }

    for(auto itr:mp)
    {
        string op = parsedQuery.groupOperation;
        resultantRow.clear();
        resultantRow.push_back(itr.first);
        if(op=="MAX")
        {
            resultantRow.push_back((itr.second)[0]);
        }
        else if(op=="MIN")
        {
            resultantRow.push_back((itr.second)[1]);
        }
        else if(op=="SUM")
        {
            resultantRow.push_back((itr.second)[2]);
        }
        else if(op=="AVG")
        {
            resultantRow.push_back((itr.second)[2]/((itr.second)[3]));
        }
        resultantTable->writeRow<int>(resultantRow);
        
    }
    resultantTable->blockify();
    tableCatalogue.insertTable(resultantTable);
}