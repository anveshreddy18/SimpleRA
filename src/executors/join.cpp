#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");

    int offset =0;
    
    if(tokenizedQuery.size()==13)
    {
        // when JOIN USING NESTED
        if(tokenizedQuery[4]=="NESTED")
        {
            offset=2;
            parsedQuery.joinType="NESTED";
            parsedQuery.buffer_size=stoi(tokenizedQuery[12]);
        }
        else if(tokenizedQuery[4]=="PARTHASH")
        {
            offset=2;
            parsedQuery.joinType="PARTHASH";
            parsedQuery.buffer_size=stoi(tokenizedQuery[12]);
        }
        else 
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }

    if (offset==0 && (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON"))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3+offset];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4+offset];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6+offset];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8+offset];

    string binaryOperator = tokenizedQuery[7+offset];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeJOIN()
{
    if(parsedQuery.joinType=="NESTED")
    {
        logger.log("executeJOIN USING NESTED");
        Table table1 = *(tableCatalogue.getTable(parsedQuery.joinFirstRelationName));
        Table table2 = *(tableCatalogue.getTable(parsedQuery.joinSecondRelationName));

        // This contains the list of all column names when join is performed. Given that both tables don't have any column in common 
        vector<string>columns;

        for (int columnCounter = 0; columnCounter < table1.columnCount; columnCounter++)
        {
            string columnName = table1.columns[columnCounter];
            columns.emplace_back(columnName);
        }

        for (int columnCounter = 0; columnCounter < table2.columnCount; columnCounter++)
        {
            string columnName = table2.columns[columnCounter];
            columns.emplace_back(columnName);
        }

        Table *resultantTable = new Table(parsedQuery.joinResultRelationName, columns);


        vector<int>row2;
        vector<int>resultantRow;
        resultantRow.reserve(resultantTable->columnCount);

        int column1 = table1.getColumnIndex(parsedQuery.joinFirstColumnName);
        int column2 = table2.getColumnIndex(parsedQuery.joinSecondColumnName);


        int nB = parsedQuery.buffer_size;
        vector<Page>buffer_blocks(nB-2);
        int times = ceil((table1.blockCount)/(float)(nB-2));
        int page_index =0;
        while(times)
        {
            int counter=0;
            while(counter<nB-2 && page_index<table1.blockCount)
            {
                buffer_blocks[counter]= Page(parsedQuery.joinFirstRelationName, page_index);
                counter++;
                page_index++;
            }

            Cursor cursor2 = table2.getCursor();
            row2 = cursor2.getNext();
            while(!row2.empty())
            {
                for(int i=0;i<counter;i++)
                {
                    Page cur = buffer_blocks[i];
                    for(int rowCounter=0;rowCounter<table1.maxRowsPerBlock;rowCounter++)
                    {
                        vector<int>row1 = cur.getRow(rowCounter);
                        if(row1.size()==0) break;
                        if(evaluateBinOp(row1[column1], row2[column2], parsedQuery.joinBinaryOperator))
                        {
                            resultantRow = row1;
                            resultantRow.insert(resultantRow.end(), row2.begin(), row2.end());
                            resultantTable->writeRow<int>(resultantRow);
                        }
                    }
                }
                row2 = cursor2.getNext();
            }
            times--;
        }
        // cout<<"main ops safe\n";
        resultantTable->blockify();
        // cout<<"blockify safe\n";
        tableCatalogue.insertTable(resultantTable);

    }
    if(parsedQuery.joinType=="PARTHASH")
    {
        logger.log("executeJOIN USING PARTHASH");
        Table table1 = *(tableCatalogue.getTable(parsedQuery.joinFirstRelationName));
        Table table2 = *(tableCatalogue.getTable(parsedQuery.joinSecondRelationName));

        // This contains the list of all column names when join is performed. Given that both tables don't have any column in common 
        vector<string>columns;

        for (int columnCounter = 0; columnCounter < table1.columnCount; columnCounter++)
        {
            string columnName = table1.columns[columnCounter];
            columns.emplace_back(columnName);
        }

        for (int columnCounter = 0; columnCounter < table2.columnCount; columnCounter++)
        {
            string columnName = table2.columns[columnCounter];
            columns.emplace_back(columnName);
        }

        Table *resultantTable = new Table(parsedQuery.joinResultRelationName, columns);
        vector<int>resultantRow;
        resultantRow.reserve(resultantTable->columnCount);

        int column1 = table1.getColumnIndex(parsedQuery.joinFirstColumnName);
        int column2 = table2.getColumnIndex(parsedQuery.joinSecondColumnName);


        int nB = parsedQuery.buffer_size;
        int M = nB-1;
        int limit = table1.maxRowsPerBlock;

        // PARTITIONING TABLE1 INTO M BLOCKS
        vector<vector<int>>partition1[M];
        vector<int>no_of_pages1(M,0);
        Cursor cursor1 = table1.getCursor();
        vector<int>row1 = cursor1.getNext();
        while(!row1.empty())
        {
            int val = row1[column1];
            int part_no = val%M;
            partition1[part_no].push_back(row1);
            
            // if partition1[part_no] is full, then we write it to disk
            if(partition1[part_no].size()==limit)
            {
                // Create new_page and write it into disk
                string tableName = parsedQuery.joinFirstRelationName+"_partition"+to_string(part_no);
                Page new_page = Page(tableName, no_of_pages1[part_no], partition1[part_no], limit);
                new_page.writePage("Table");

                // updations
                no_of_pages1[part_no]++;
                partition1[part_no].clear();
            }
            row1 = cursor1.getNext();
        }
        // push all the partitions of parition1 to disk
        for(int i=0;i<M;i++)
        {
            if(partition1[i].size())
            {
                // then write this page into the disk
                string tableName = parsedQuery.joinFirstRelationName+"_partition"+to_string(i);
                Page new_page = Page(tableName, no_of_pages1[i], partition1[i], partition1[i].size());
                new_page.writePage("Table");

                // updations
                no_of_pages1[i]++;
                partition1[i].clear();
            }
        }

        // PARTITIONING TABLE2 INTO M BLOCKS
        vector<vector<int>>partition2[M];
        vector<int>no_of_pages2(M,0);
        Cursor cursor2 = table2.getCursor();
        vector<int>row2 = cursor2.getNext();
        while(!row2.empty())
        {
            int val = row2[column2];
            int part_no = val%M;
            partition2[part_no].push_back(row2);
            
            // if partition2[part_no] is full, then we write it to disk
            if(partition2[part_no].size()==limit)
            {
                // Create new_page and write it into disk
                string tableName = parsedQuery.joinSecondRelationName+"_partition"+to_string(part_no);
                Page new_page = Page(tableName, no_of_pages2[part_no], partition2[part_no], limit);
                new_page.writePage("Table");

                // updations
                no_of_pages2[part_no]++;
                partition2[part_no].clear();
            }
            row2 = cursor2.getNext();
        }
        // push all the partitions of parition1 to disk
        for(int i=0;i<M;i++)
        {
            if(partition2[i].size())
            {
                // then write this page into the disk
                string tableName = parsedQuery.joinSecondRelationName+"_partition"+to_string(i);
                Page new_page = Page(tableName, no_of_pages2[i], partition2[i], partition2[i].size());
                new_page.writePage("Table");

                // updations
                no_of_pages2[i]++;
                partition2[i].clear();
            }
        }

        // Partition phase is completed
        // Probing phase begins

        for(int i=0;i<M;i++)
        {
            // we are at a partition i
            unordered_map<int, vector<pair<int,int>>> mp1;
            // traverse all pages of partition i from table2
            for(int j=0;j<no_of_pages2[i];j++)
            {
                // cout<<"coming here\n";
                string pageName = "../data/temp/"+parsedQuery.joinSecondRelationName+"_partition"+to_string(i)+ "_Page" + to_string(j);
                
                BLOCK_ACCESSES++;

                // cout<<pageName<<endl;
                // now extract page using pageName from temp directory
                ifstream fin(pageName, ios::in);
                int number;
                int rowCount =0;
                string line;
                while(getline(fin, line))
                {
                    rowCount++;
                }
                fin.close();

                // cout<<rowCount<<endl;

                ifstream finn(pageName, ios::in);
                vector<vector<int>>rows(rowCount, vector<int>(table2.columnCount));
                for (uint rowCounter = 0; rowCounter < rowCount; rowCounter++)
                {
                    for (int columnCounter = 0; columnCounter < table2.columnCount; columnCounter++)
                    {
                        finn >> number;
                        // cout<<number<<" ";
                        rows[rowCounter][columnCounter] = number;
                    }
                    // cout<<endl;
                }
                finn.close(); 
                
                // Now, we have the contents of the page i.e vector<vector<int>>rows

                for(int rowCounter=0; rowCounter<rowCount; rowCounter++)
                {
                    mp1[rows[rowCounter][column2]].push_back({j, rowCounter});
                }
            }

            // for(auto itr:mp1)
            // {
            //     cout<<itr.first<<" = \n";
            //     for(int z =0;z<itr.second.size();z++)
            //     {
            //         cout<<"{ "<<itr.second[z].first<<" , "<<itr.second[z].second<<" }";
            //     }
            //     cout<<endl;
            // }

            // We have traversed all pages from partition i of table2

            // We should take nB-2 no of blocks every time from the partition i of table1

            vector<vector<int>>buffer_blocks[nB-2];
            int times = ceil((no_of_pages1[i])/(float)(nB-2));
            int page_index =0;
            while(times)
            {
                int counter=0;
                while(counter<nB-2 && page_index<no_of_pages1[i])
                {
                    string pageName = "../data/temp/"+parsedQuery.joinFirstRelationName+"_partition"+to_string(i)+ "_Page" + to_string(page_index);
                    
                    BLOCK_ACCESSES++;

                    // now extract page using pageName from temp directory
                    ifstream fin(pageName, ios::in);
                    int number;
                    int rowCount =0;
                    string line;
                    while(getline(fin, line))
                    {
                        rowCount++;
                    }
                    fin.close();

                    ifstream finn(pageName, ios::in);
                    vector<vector<int>>rows(rowCount, vector<int>(table1.columnCount));
                    for (uint rowCounter = 0; rowCounter < rowCount; rowCounter++)
                    {
                        for (int columnCounter = 0; columnCounter < table1.columnCount; columnCounter++)
                        {
                            finn >> number;
                            rows[rowCounter][columnCounter] = number;
                        }
                    }
                    finn.close(); 

                    buffer_blocks[counter] = rows;
                    counter++;
                    page_index++;
                }

                // Now go through all the pages of the buffer block 
                for(int j=0;j<counter;j++)
                {
                    for(int r=0;r<buffer_blocks[j].size();r++)
                    {
                        vector<pair<int,int>> v = mp1[buffer_blocks[j][r][column1]];
                        for(int l=0;l<v.size();l++)
                        {
                            pair<int,int>p = v[l];
                            // fetch page with name table2_partitioni_page_p.first
                            string pageName = "../data/temp/"+parsedQuery.joinSecondRelationName+"_partition"+to_string(i)+ "_Page" + to_string(p.first);
                            
                            BLOCK_ACCESSES++;

                            ifstream fin(pageName, ios::in);
                            int number;
                            int rowCount =0;
                            string line;
                            while(getline(fin, line))
                            {
                                rowCount++;
                            }
                            fin.close();

                            ifstream finn(pageName, ios::in);
                            vector<vector<int>>rows(rowCount, vector<int>(table2.columnCount));
                            for (uint rowCounter = 0; rowCounter < rowCount; rowCounter++)
                            {
                                for (int columnCounter = 0; columnCounter < table2.columnCount; columnCounter++)
                                {
                                    finn >> number;
                                    rows[rowCounter][columnCounter] = number;
                                }
                            }
                            finn.close(); 

                            // we have fetched the page
                            resultantRow = buffer_blocks[j][r];
                            resultantRow.insert(resultantRow.end(), rows[p.second].begin(), rows[p.second].end());
                            resultantTable->writeRow<int>(resultantRow);
                        }
                    }
                }
                times--;
            }
        }
        // cout<<"Working fine\n";
        // There's nothing after this.. simon go back
        BLOCK_ACCESSES/=2;
         // cout<<"main ops safe\n";
        resultantTable->blockify();
        // cout<<"blockify safe\n";
        tableCatalogue.insertTable(resultantTable);

    }
    else
    {
        logger.log("executeJOIN NORMAL");
    }
    return;
}