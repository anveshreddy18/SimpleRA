# SimpleRA

## Compilation Instructions

We use ```make``` to compile all the files and creste the server executable. ```make``` is used primarily in Linux systems.

To compile

```cd``` into the SimpleRA directory
```
cd SimpleRA
```
```cd``` into the soure directory (called ```src```)
```
cd src
```
To compile
```
make clean
make
```

## To run

Post compilation, an executable names ```server``` will be created in the ```src``` directory
```
./server
```

## Branch Info

- ```master``` branch supports transposing matrices and join and group by commands.

- ```BPTree``` branch supports B+ tree indices.

## Commands / Queries

- Look at the [Overview.html](docs/Overview.html) to understand the syntax and working of the table related queries.

- ```LOAD MATRIX <matrix_name>```:
The LOAD MATRIX command loads contents of the .csv (stored in ```data``` folder) and stores it as blocks in the ```data/temp``` directory.

- ```PRINT MATRIX <matrix_name>```:
PRINT MATRIX command prints the first 20 rows of the matrix on the terminal.

- ```TRANSPOSE <matrix_name>```:
TRANSPOSE command transposes the matrix IN PLACE (without using any additional disk blocks) and writes it back into the same blocks the matrix was stored in.

- ```EXPORT MATRIX <matrix_name>```:
EXPORT command writes the contents of the matrix named
<matrix_name> into a file called <matrix_name>.csv in ```data``` folder.
