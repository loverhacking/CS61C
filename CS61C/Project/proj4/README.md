# numc

Here's what I did in project 4:

# Prev Work
Since I do the proj4 in my local laptop, there's some extra work to do.
* create a virtual environment
``` shell
python3.6 -m venv .venv # create a virtual environment
source .venv/bin/activate # activate the virtual environment
pip3 install -r requirements.txt # install all python packages for running custom python tests
deactivate # exit out of the virtual environment
```

* install CUnit library (for Task 1 test)
```shell
# Installing compilation dependencies
sudo apt install build-essential autoconf libtool

# Download source code
wget https://sourceforge.net/projects/cunit/files/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
tar -xjf CUnit-2.1-3.tar.bz2
cd CUnit-2.1-3

# Compile and install
autoreconf -f -i
./configure
make
sudo make install

# Update dynamic link library cache
sudo ldconfig
```
* change `Makefile` (for Task 1 test)
  * `CUNIT = -lcunit `
  * `PYTHON = -I/usr/include/python3.8 -lpython3.8`
    * I use python 3.8 version

# Task 1: Matrix functions in C
* The most difficult part is `allocate_matrix` and `deallocate_matrix` function implementation
  * for `allocate_matrix`
    * note the memory allocation for `matrix->data` and initialize to zero
    * note how to deal with memory allocation fail and free memory to avoid memory leak
  * for `deallocate_matrix`
    * implement it in a iterative way 
    * If the matrix has a parent, it's a slice. 
      * The function frees the slice structure and moves to the parent, 
  decrementing its reference count.
      * If at any point a parent's reference count remains positive after decrementing,
        the loop terminates early since other references still exist.
    * If the matrix has no parent, it's a root. 
      * The function decrements its reference count.
        If the count reaches zero, it frees the data and the matrix structure.
* When implementing the `pow_matrix` function, I create a temporary matrix to store the intermediate result

# Task2: Writing the setup file
* This part is not hard as long as you read the documents carefully and find the relevant argument you need.
And can read the example for reference.

# Task3: Writing the Python-C interface

## Number Methods
* This part includes implementing add, sub, mul, neg, abs and pow operation.
  * It basically calls the function written in `matrix.c`.
  * Notice the error checking
  * Use `Matrix61c_new` to create new `numc.Matrix` objects, set `shape` and `mul` attribute. 
    * Use `allocate_matrix` function to allocate space to hold result.
  * Finally, can install new `numc` module and try the method above to test.

## Instance Methods
* This part is the same as above
  * Notice look for API (e.g. `PyLong_AsLong`)for the translation between `Pyobject*` and basic data type(e.g. `int`)
* When implementing `PyMethodDef` structs `Matrix61c_methods`, 
   `{NULL, NULL, 0, NULL}` from starter code must be last element of `Matrix61c_methods`
* Finally, can install new `numc` module and try the method above to test.


## Indexing
* This part is the most difficult for me, since there exist so many details and cases to handle.
 I write more than 200 lines in one function. Here I summarize it as follows.
* For `Matrix61c_subscript` function:
  * If is 1d matrix, the arg is a int or a slice
    * If is an int, find the corresponding element.
    * If is a slice, create a new matrix and copy the value from `self->mat`.
      * Notice the length of slice can be one, which degenerate to the int case.
  * If is 2d matrix, the arg is a int, a slice or a tuple of two slices/ints.
    * If is an int,  it's an int index of row.
    * If is a slice, it's a slice index  for rows.
    * If is a tuple of two slices/ints:
      * Deal with row and col index separately to find each it's a int or a slice.
      * I write a helper function `creatMatrixFromSlice` to generalize the matrix creation.
      * Notice the length of row slice and col slice can be 1, or both the row and col is a int, so in that situation 
      return the element directly and no need to create a new matrix.
* For `Matrix61c_set_subscript` function:
  * It basically does the same thing as above, but add additional error check for arg `v` and copy value from it.
  * Notice if the slice is 2d matrix, the `v` should also be 2d matrix, and have to iterate every element to check it is an int or a float.
  * Notice if the slice is 1 by n, the length of length should be equal to the length of col slice and the `v` should be a list.

## Testing
* I add more tests in `unittests.py` and write a script `test.sh` to do basic correctness check.

# Task 4: Speeding up matrix operations
* Can find lab9 for reference, and speed up by SIMD and OpenMP
* Write a script `testSpeed.sh` to do speed up test.
* Since can't access `dumbpy` module, which is on hive machine. I optimize the code to speed up, and compare the speedup version of `numc` with my base
version, which is also my `dumbpy` version.
  * I write `setup_dumbpy.py` and add `install_dumb` in `Makefile` to install `dumbby` module 
    