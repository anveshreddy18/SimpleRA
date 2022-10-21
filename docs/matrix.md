# Q2)
>**PAGE LAYOUT**

* Each page corresponding to the matrix have a specified amount of storage defined in the `BLOCK_SIZE` parameter in the `server.cpp` file and also a parameter called `MaxElemsPerBlock` which stores the max number of elements that a block can store. The elements are unsigned integers
* We are storing the elements in a linear fashion till the `BLOCK_SIZE*1000` size of the block is filled. If it overflows, then we open the next block(page) and continue to store the elements in the next page and so on.. 
* The order of inserting elements into block is by traversing every row from top and for each row, we start from the left and constantly `push_back` elements into a `vector`. Whenever the size of the vector overflows the `MaxElemsPerBlock` factor of that `Page`, then we write the contents of the vector into a page and continue to do so for the next pages until all the elements of the matrix are traversed and inserted into blocks.

***

>**IMPLEMENTATION OF CROSS_TRANSPOSE**

* After both the matrices have been loaded, let say the matrices are `A` & `B` both of same size square matrices. Lets say `n = rowCount = colCount` of both A and B. 
* Transposing A & B means, we need to transpose every element in the matrices correspondingly.  
* So we run two loops one for row and another for column of A like below. 
```
for(int i=0;i<n;i++)
{
    for(int j=0;j<n;j++)
    {
        int ind = i*n+j;
    }
}
```
Now what it does is just traverse over the matrix A element by element, then we take index `ind` which is like an indentifier for the element to locate the position of it. Its just plain numbering for matrix elements starting from 0 to n*n-1. 

* The reason this numbering is important is because we can know the position of this element i.e in which block it is present (`Page Index`) and at what position in the page(Remember a page just stores elements in a linear vector). 

* ind/blockSize will give me the pageIndex of this element and ind%blockSize gives the position in that page. So finally, we are able to locate this element.

* Now we need to transpose it / exchange it with the corresponding element of matrix B. The corresponding element of matrix B can also be found in the same manner except that `indB` now will be `j*n+i` instead of i*n+j. Because the corresponding element of (i,j) pair of matrix A is (j,i) pair of matrix B.

* Now that we know both the values let say `valA` and `valB`, and we also know the pages from which these values came from let say `PageA` and `PageB`. We will edit the vector in the PageA by placing valB in the original position of valA and valA in the original position of valB. 

* Now the pages pageA and pageB have been modified but in the main memory of the code only. So to make these changes felt, we have to write them to the blocks in the temp directory. 

* And the transpose is over.

***
***
***

# Q3) 

>**PAGE LAYOUT**

* The page layout is different compared to normal matrix because using normal page layout would result in a lot of wasted space as most of the matrix elements in sparse matrix are anyway zeroes. So we want to use a different page layout so as to save space and time. 

* In a page, we store `triplets` of `{rowNo, colNo, value }` of all the non-zero values in the matrix. We avoid storing zero valued elements as they are not useful. Now again we have a block size parameter `BLOCK_SIZE` which defines the size of the block in terms of no of BYTES. Now since for each triplet, we are storing 3 integer values, we can store say `maxTripletsPerBlock = (BLOCK_SIZE*1000)/12`. 

* The order in which we store these triplets is in the increasing order of rowNo, colNo. If rowNo is same for two triplets, then we use colNo to sort these two. The reason we are storing it in this increasing order of rowNo and colNo is that the number of accessing blocks can be reduced significantly since we now can use binary search to find a element. 

* Suppose we want to find the value at position (i,j), then we do binary search for rowNo = i first, as the rowNo's are sorted, we can reduce our search space by half every time we go to a block to search for that element. Now let say our pointer is at block b, now to search further after we've finalised on the row, we can use colNo to search for it again doing binary search.

* If in case we don't find the element, then the value at that positon (i,j) is `zero`

* So overall, this is a very good method to store the elements in the page. 

>**TRANSPOSE**

* The transpose proceeds similar to that of CROSS_TRANSPOSE for normal matrices. In the way that we traverse each element and the row and column pair should be exchanged with the corresponding transpose pair. for (i,j) it is (j,i).

* Now we find the element as we've described how to search using binary search in the previous part. Once we know what values to change and the locations of them. We change the triplets to the desired values. If one of the element is not found, then the other triplet should be placed value zero in it. and the one with original non-zero value should now be inserted with its corresponding (j,i) pair. For eg. if `value[i][j] is x!=0` and `value[j][i] is 0`. Then after the transpose we need to remove the `(i,j)` pair from the set of triplets because now that it contains the value zero, we no longer need to maintain it in the list of triplets. And on the other hand we need to add the pair `(j,i)` which earlier is not present but now should be inserted because it now contains a non-zero value. To do all these operations we use a `set of vector`. Which maintains the order or rows, cols in the increasing order. We also need to maintain the page index along with the `rowNo, colNo, value, page_ID`. Whenever we need to remove or insert or edit a triplet from the set, we will use `lower_bound` and `upper_bound` STL methods of set to locate the triplets and can do operations on them in logarithmic time. Now after the modification of this, we need to write these changes into the pages.  