#pragma once

#pragma once

#include <vector>
#include <assert.h>
//using namespace std;
#include <thread>
#include <mutex>

//  Simple matrix class that wraps a vector,
//  See chapters 1 and 2

template <class T>
class matrix
{
public:
	size_t      myRows;
	size_t      myCols;
	std::vector<T>   myVector;



	//  Constructors
	matrix() : myRows(0), myCols(0) {}
	matrix(const size_t rows, const size_t cols) : myRows(rows), myCols(cols), myVector(rows*cols) {}

	//  Copy, assign
	matrix(const matrix& rhs) : myRows(rhs.myRows), myCols(rhs.myCols), myVector(rhs.myVector) {}
	matrix& operator=(const matrix& rhs)
	{
		if (this == &rhs) return *this;
		matrix<T> temp(rhs);
		swap(temp);
		return *this;
	}

	//  Copy, assign from different (convertible) type
	template <class U>
	matrix(const matrix<U>& rhs)
		: myRows(rhs.rows()), myCols(rhs.cols())
	{
		myVector.resize(rhs.rows() * rhs.cols());
		copy(rhs.begin(), rhs.end(), myVector.begin());
	}
	template <class U>
	matrix& operator=(const matrix<U>& rhs)
	{
		if (this == &rhs) return *this;
		matrix<T> temp(rhs);
		swap(temp);
		return *this;
	}

	//  Move, move assign
	matrix(matrix&& rhs) : myRows(rhs.myRows), myCols(rhs.myCols), myVector(move(rhs.myVector)) {}
	matrix& operator=(matrix&& rhs)
	{
		if (this == &rhs) return *this;
		matrix<T> temp(move(rhs));
		swap(temp);
		return *this;
	}

	//  Swapper
	void swap(matrix& rhs)
	{
		myVector.swap(rhs.myVector);
		::swap(myRows, rhs.myRows);
		::swap(myCols, rhs.myCols);
	}

	//  Resizer
	void resize(const size_t rows, const size_t cols)
	{
		myRows = rows;
		myCols = cols;
		if (myVector.size() < rows*cols) myVector = std::vector<T>(rows*cols);
	}

	//  Access
	size_t rows() const { return myRows; }
	size_t cols() const { return myCols; }
	//  So we can call matrix [i][j]
	T* operator[] (const size_t row) { return &myVector[row*myCols]; }
	const T* operator[] (const size_t row) const { return &myVector[row*myCols]; }
	bool empty() const { return myVector.empty(); }

	//  Iterators
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	iterator begin() { return myVector.begin(); }
	iterator end() { return myVector.end(); }
	const_iterator begin() const { return myVector.begin(); }
	const_iterator end() const { return myVector.end(); }
};

template <class T>
inline matrix<T> transpose(const matrix<T>& mat)
{
	matrix<T> res(mat.cols(), mat.rows());
	for (size_t i = 0; i < res.rows(); ++i)
	{
		for (size_t j = 0; j < res.cols(); ++j)
		{
			res[i][j] = mat[j][i];
		}
	}

	return res;
}

template <class T>
matrix<T> matrixProduct(const matrix<T>& mat1, const matrix<T>& mat2)
{
	assert(mat1.cols() == mat2.rows());
	matrix<T> res(mat1.rows(), mat2.cols());
	for (size_t i = 0; i < mat1.rows(); ++i)
	{
		//const T* ai = mat1[i];
		for (size_t j = 0; j < mat2.cols(); ++j)
		{
			//now iterate ovet the cols and add to the res
			double x = 0;
			//now another loop
			for (size_t k = 0; k < mat1.cols(); ++k)
			{
				x += mat1[i][k] * mat2[k][j];
				//mat1[i][k] is local , successive aik are local in memory
				//successive mat2[k][j] are not local, but some 1000 doubles (or the size of the matrix2)
			}
			res[i][j] = x;
		}

	}

	return res;//std::move 
}

template <class T>
matrix<T> matrixProduct2(const matrix<T>& mat1, const matrix<T>& mat2)
{
	assert(mat1.cols() == mat2.rows());
	matrix<T> res(mat1.rows(), mat2.cols());

	for (size_t i = 0; i < mat1.rows(); ++i)
	{		
		for (size_t j = 0; j < mat2.cols(); ++j)
			res[i][j] = 0;//init
	}	

	for (size_t i = 0; i < mat1.rows(); ++i)
	{
		for (size_t k = 0; k < mat1.cols(); ++k)
		{
			//#pragma loop(no_vector) - cancel vectorization
			for (size_t j = 0; j < mat2.cols(); ++j)
				res[i][j] += mat1[i][k] * mat2[k][j];
		}
	}

	return res;//std::move 
}


template <class T>
matrix<T> matrixProductMT(const matrix<T>& mat1, const matrix<T>& mat2)
{
	assert(mat1.cols() == mat2.rows());
	matrix<T> res(mat1.rows(), mat2.cols());

	for (size_t i = 0; i < mat1.rows(); ++i)
	{
		for (size_t j = 0; j < mat2.cols(); ++j)
			res[i][j] = 0;//init
	}

	std::mutex mut;
	int num_threads = 4;

	int i_step = 0;
	auto f_ =[&i_step, &mut, &res, mat1, mat2, num_threads]()
	{
		mut.lock();
		int i = i_step++; //post increment
		mut.unlock();
		int i_start = mat1.rows() / num_threads * i;
		int i_end = mat1.rows() / num_threads * (i+1);
		for (i = i_start; i < i_end; ++i)
		{
			for (size_t k = 0; k < mat1.cols(); ++k)
			{
				//#pragma loop(no_vector) - cancel vectorization
				for (size_t j = 0; j < mat2.cols(); ++j)
					res[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
		
	};

	std::vector<std::thread> myThreads(num_threads);
	for (int i = 0; i < num_threads; ++i)
		myThreads[i] = std::thread(f_); //we pass reference

	for (int i = 0; i < num_threads; ++i)
		myThreads[i].join();

	return res;//std::move 
}
