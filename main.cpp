#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <x86intrin.h>
#include "PmergeMe.hpp"

#define CACHE_LINE 64

int main(__attribute__((unused))int argc, __attribute__((unused)) char** argv)
{
	// Test about PmergeMpe.hpp
#if 0

	for (int t = 0; t < 1000000; t++)
	{
		std::vector<int> v1;
		v1.reserve(5);

		for (int i = 0; i < 5; i++)
			v1.push_back(rand()%5);

		std::cout << "Before : ";
		for (std::vector<int>::iterator it = v1.begin(); it != v1.end(); it++)
			std::cout << *it << " ";
		std::cout << std::endl;

		PmergeMe<std::vector<int> > p1;
		p1.sort(v1);

		std::cout << "Sorted vector: ";
		for (std::vector<int>::iterator it = v1.begin(); it != v1.end(); it++)
			std::cout << *it << " ";
		std::cout << std::endl;

		// check if the vector is sorted
		std::cout << "Is the vector sorted? " << std::boolalpha << std::is_sorted(v1.begin(), v1.end()) << std::endl;
	}
		__attribute__((unused))
		int n;
		std::cin >> n;
		// // check count of elements
		// std::cout << "Count of elements: " << v1.size() << std::endl;
		// // original count
		// std::cout << "Original count of elements: " << argc - 1 << std::endl;
#endif

	// Mandatory specification performance test
#if 1
	std::srand(std::time(0));

	std::vector<int> v;
	std::deque<int> d;
	std::list<int> l;

	double vectorTimeMicroSec = 0;
	double dequeTimeMicroSec = 0;

	// Print input
	std::cout << "Before : ";
	for (int i = 1; i < argc; i++)
		std::cout << argv[i] << " ";
	std::cout << std::endl;

	// Flush the cache
	__builtin_ia32_mfence();
	{
		for (int i = 0; i < argc; i++)
			for (size_t j = 0; j < std::strlen(argv[i]); j += CACHE_LINE)
				__builtin_ia32_clflush((char*)&argv[i][j]);
		for (int i = 0; i < argc; i += CACHE_LINE / sizeof(char*))
			__builtin_ia32_clflush((char*)&argv[i]);

		enum { N = 1024 * 1024 * 64 };
		volatile char* arr = new char[N];
		for (int i = 0; i < N * 2; i++)
			arr[std::rand() % (N / CACHE_LINE) * CACHE_LINE] = 0;
		delete[] arr;
	}

	// Fill the deque
	__builtin_ia32_mfence();
	d.resize(argc - 1);
	for (int i = 0; i < argc - 1; i++)
	{
		d[i] = std::atoi(argv[i + 1]);
	}

	// Sort deque
	__builtin_ia32_mfence();
	{
		std::clock_t start = std::clock();
		{
			PmergeMe<std::deque<int> > p;
			p.sort(d);
		}
		std::clock_t end = std::clock();
		dequeTimeMicroSec = (end - start) / (CLOCKS_PER_SEC / 1000 / 1000);
	}

	// Flush the cache
	__builtin_ia32_mfence();
	{
		for (int i = 0; i < argc; i++)
			for (size_t j = 0; j < std::strlen(argv[i]); j += CACHE_LINE)
				__builtin_ia32_clflush((char*)&argv[i][j]);
		for (int i = 0; i < argc; i += CACHE_LINE / sizeof(char*))
			__builtin_ia32_clflush((char*)&argv[i]);

		enum { N = 1024 * 1024 * 64 };
		volatile char* arr = new char[N];
		for (int i = 0; i < N * 2; i++)
			arr[std::rand() % (N / CACHE_LINE) * CACHE_LINE] = 0;
		delete[] arr;
	}

	// Fill the vector
	__builtin_ia32_mfence();
	v.resize(argc - 1);
	for (int i = 0; i < argc - 1; i++)
	{
		v[i] = std::atoi(argv[i + 1]);
	}

	// Sort vector
	__builtin_ia32_mfence();
	{
		std::clock_t start = std::clock();
		{
			PmergeMe<std::vector<int> > p;
			p.sort(v);
		}
		std::clock_t end = std::clock();
		vectorTimeMicroSec = (end - start) / (CLOCKS_PER_SEC / 1000 / 1000);
	}

	// Print result
	__builtin_ia32_mfence();
	std::cout << "After  : ";
	for (std::vector<int>::iterator it = v.begin(); it != v.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;

	std::cout << "std::deque  time: " << dequeTimeMicroSec << " us" << std::endl;
	std::cout << "std::vector time: " << vectorTimeMicroSec << " us" << std::endl;

	// check if the vector is sorted
	std::cout << "Is the vector sorted? " << std::boolalpha << std::is_sorted(v.begin(), v.end()) << std::endl;

#endif

	return 0;
}
