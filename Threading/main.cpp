#include <iostream>

#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "matrix.h"
#include <chrono>
#include <numeric>
#include "TemplateTest.h"

class BankAccount
{
private:
	double balance;
public:
	BankAccount() : balance(0) {};

	void deposit(const double amount)
	{
		balance += amount;
	}

	void withdraw(const double amount)
	{
		std::cout << "Entering: " << std::this_thread::get_id() << "\n";
		if (balance > amount)
		{
			std::chrono::milliseconds span(2000);
			std::this_thread::sleep_for(span);
			balance -= amount;
			std::cout << "Withdraw " << std::this_thread::get_id() << "\n";
			std::cout << "Amount " << amount << " withdrawn" << "\n";
			std::cout << "New Balance " << balance << "\n";
			
		}

		
	}

	double getBalance() const
	{
		return balance;
	}
};


class BankAccountLocked
{
private:
	double balance;
	std::mutex myMutex;
public:
	BankAccountLocked() : balance(0) {};

	void deposit(const double amount)
	{
		//we need to protect here because two threads can deposit but read same balance
		myMutex.lock();
		balance += amount;
		myMutex.unlock();
	}

	void withdraw(const double amount)
	{
		myMutex.lock();
		std::cout << "Entering: " << std::this_thread::get_id() << "\n";
		if (balance > amount)
		{
			std::chrono::milliseconds span(2000);
			std::this_thread::sleep_for(span);
			balance -= amount;
			std::cout << "Withdraw " << std::this_thread::get_id() << "\n";
			std::cout << "Amount " << amount << " withdrawn" << "\n";
			std::cout << "New Balance " << balance << "\n";
			myMutex.unlock();//we have to unlock in all possible outcomes and before return
			return;//here we have to return so that we dont unlock twice!

		}

		myMutex.unlock();//we have to unlock in all possible outcomes and before return

	}

	double getBalance() const
	{
		return balance;
	}
};


class BankAccountAutoLocked
{
private:
	double balance;
	std::mutex myMutex;
public:
	BankAccountAutoLocked() : balance(0) {};

	void deposit(const double amount)
	{
		//we need to protect here because two threads can deposit but read some balance
		std::lock_guard<std::mutex> lg(myMutex); //automatic release
		//myMutex.lock();
		balance += amount;
		//myMutex.unlock();
	}

	void withdraw(const double amount)
	{
		//myMutex.lock();
		std::lock_guard<std::mutex> lg(myMutex); //automatic release
		std::cout << "Entering: " << std::this_thread::get_id() << "\n";
		if (balance > amount)
		{
			std::chrono::milliseconds span(2000);
			std::this_thread::sleep_for(span);
			balance -= amount;
			std::cout << "Withdraw " << std::this_thread::get_id() << "\n";
			std::cout << "Amount " << amount << " withdrawn" << "\n";
			std::cout << "New Balance " << balance << "\n";
			//myMutex.unlock();//we have to unlock in all possible outcomes and before return
			return;//here we have to return so that we dont unlock twice!

		}

		//myMutex.unlock();//we have to unlock in all possible outcomes and before return

	}

	double getBalance() const
	{
		return balance;
	}
};

void threadFunc()
{
	std::cout << "Hello World " << std::this_thread::get_id() << "\n";
}

void testHelloWorldThreading()
{
	const int t_size = std::thread::hardware_concurrency();
	std::vector<std::thread> myThreads(t_size);
	for (int i = 0; i < t_size; ++i)
		myThreads[i] = std::thread(threadFunc);
	std::cout << "Hello World from main thread " << "\n";
	for (int i = 0; i < t_size; ++i)
		myThreads[i].join();

	std::cout << "Completed";
}

void testBankAcc()
{
	auto start = std::chrono::high_resolution_clock::now();
	BankAccount bankAcc;
	bankAcc.deposit(800);
	int t_size = 2;
	std::vector<std::thread> myThreads(t_size);
	for (int i = 0; i < t_size; ++i)
		myThreads[i] = std::thread(&BankAccount::withdraw, &bankAcc, 500); //we pass reference
	bankAcc.withdraw(500);
	for (int i = 0; i < t_size; ++i)
		myThreads[i].join();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	const long seconds = duration.count();

	std::cout << "Final Balace " << bankAcc.getBalance() << "\n";

	std::cout << "Completed" << "\n";
	std::cout << "Time Elapsed" << seconds << "\n";
}

void testBankAcc_copy()
{
	auto start = std::chrono::high_resolution_clock::now();
	BankAccount bankAcc;
	bankAcc.deposit(800);
	int t_size = 2;
	std::vector<std::thread> myThreads(t_size);
	for (int i = 0; i < t_size; ++i)
		myThreads[i] = std::thread(&BankAccount::withdraw, bankAcc, 500); //we pass copy
	bankAcc.withdraw(500);
	for (int i = 0; i < t_size; ++i)
		myThreads[i].join();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	const long seconds = duration.count();

	std::cout << "Final Balace " << bankAcc.getBalance() << "\n";

	std::cout << "Completed" << "\n";
	std::cout << "Time Elapsed" << seconds << "\n";
}

void testBankAccLocked()
{
	auto start = std::chrono::high_resolution_clock::now();
	BankAccountLocked bankAcc;
	bankAcc.deposit(800);
	int t_size = 2;
	std::vector<std::thread> myThreads(t_size);
	for (int i = 0; i < t_size; ++i)
		myThreads[i] = std::thread(&BankAccountLocked::withdraw, &bankAcc, 500); //we pass reference
	bankAcc.withdraw(500);
	for (int i = 0; i < t_size; ++i)
		myThreads[i].join();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	const long seconds = duration.count();
	const double seconds_d = static_cast<double>(seconds);

	std::cout << "Final Balace " << bankAcc.getBalance() << "\n";

	std::cout << "Completed" << "\n";
	std::cout << "Time Elapsed" << seconds << "\n";

}

void testBankAccLockedAuto()
{
	auto start = std::chrono::high_resolution_clock::now();
	BankAccountAutoLocked bankAcc;
	bankAcc.deposit(800);
	int t_size = 2;
	std::vector<std::thread> myThreads(t_size);
	for (int i = 0; i < t_size; ++i)
		myThreads[i] = std::thread(&BankAccountAutoLocked::withdraw, &bankAcc, 500); //we pass reference
	bankAcc.withdraw(500);
	for (int i = 0; i < t_size; ++i)
		myThreads[i].join();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	const long seconds = duration.count();

	std::cout << "Final Balace " << bankAcc.getBalance() << "\n";

	std::cout << "Completed" << "\n";
	std::cout << "Time Elapsed" << seconds << "\n";
}

void LinearSearchThreads()
{
	int n = 10;
	std::vector<double> v= { 1,2,3,4,5,6,7,8,9,10 };

	int i_thread = 0; //thread counter
	int num_threads = 2;

	bool result = false;

	//define func for each thread
	auto f_ = [&i_thread, &result, v, num_threads](const double key)
	{
		std::mutex myMutex; //we need to lock the global counter
		myMutex.lock();
		int i_t = ++i_thread;
		myMutex.unlock();
		
		//now we search in the region
		int num_t = v.size() / num_threads;
		int i_start = 0 + (i_t-1) * num_t;
		int i_end = 0 + i_t * num_t;

		std::cout << "Thread ID " << std::this_thread::get_id() << " i_t " << i_t << "\n";

		for (int i = i_start; i < i_end; ++i)
		{
			std::cout << "Thread ID " << std::this_thread::get_id() << " vec el " << i  << "\n";
			if (v[i] == key)
				result = true; //no need to lock here
		}
	};

	//int t_size = 2;
	double our_key = 10;
	std::vector<std::thread> myThreads(num_threads);
	for (int i = 0; i < num_threads; ++i)
		myThreads[i] = std::thread(f_, our_key); //we pass reference
	
	for (int i = 0; i < num_threads; ++i)
		myThreads[i].join();

	int num_t = v.size() / num_threads;
	std::cout << "num_t " << num_t << "\n";

	std::cout << "Linear search res " << result << "\n";

}

void NumberInSequence()
{
	int i_thread = 0; //thread counter
	std::mutex myMutex;
	auto f_ = [&i_thread, &myMutex]()
	{
		std::lock_guard<std::mutex> lg(myMutex);
		while(i_thread<20)
		{
			//we need to lock the global counter
			
			int i_t = ++i_thread;
			//now print
			std::cout << "Thread ID " << std::this_thread::get_id() << " i_t " << i_t << "\n";
			/*if (i_t > 50)
				return;*/
			//myMutex.unlock();
		}
		
		
	};

	std::cout << "Start of integer sequence "  << "\n";
	int num_threads = 3;
	std::vector<std::thread> myThreads(num_threads);
	for (int i = 0; i < num_threads; ++i)
		myThreads[i] = std::thread(f_); //we pass reference

	for (int i = 0; i < num_threads; ++i)
		myThreads[i].join();

}

void NumberInSequence2()
{
	int i_thread = 0; //thread counter
	std::condition_variable cv1;/*, cv2;*/
	std::mutex myMutex;
	int i_max = 20;
	auto f_odd = [&i_thread, &myMutex, i_max, &cv1 /*&cv2*/]()
	{
		//std::lock_guard<std::mutex> lg(myMutex);
		while (i_thread < i_max)
		{
			//we need to lock the global counter
			std::unique_lock<std::mutex> lock(myMutex);
			//cv1.wait(lock);
			if (i_thread % 2 != 0)
			{
				std::cout << "Thread ID " << std::this_thread::get_id() << " i_t - odd " << i_thread << "\n";
				++i_thread;				
			}
			else
			{
				cv1.wait(lock);
			}
			//int i_t = ++i_thread;
			//now print
			
			lock.unlock();
			cv1.notify_one();
			
		}


	};

	auto f_even = [&i_thread, &myMutex, i_max, &cv1/*, &cv2*/]()
	{
		//std::lock_guard<std::mutex> lg(myMutex);
		while (i_thread < i_max)
		{
			//we need to lock the global counter
			std::unique_lock<std::mutex> lock(myMutex);
			//cv2.wait(lock);
			if (i_thread % 2 == 0)
			{
				std::cout << "Thread ID " << std::this_thread::get_id() << " i_t - even " << i_thread << "\n";
				++i_thread;
			}
			else
			{
				cv1.wait(lock);
			}
			//now print
			//std::cout << "Thread ID- even " << std::this_thread::get_id() << " i_t " << i_t << "\n";
			lock.unlock();
			cv1.notify_one();

		}


	};

	std::cout << "Start of integer sequence " << "\n";
	int num_threads = 2;
	//std::vector<std::thread> myThreads(num_threads);
	//for (int i = 0; i < num_threads; ++i)
	//	myThreads[i] = std::thread(f_); //we pass reference
	std::thread thead1(f_odd);
	std::thread thead2(f_even);

	//cv1.notify_one();

	//for (int i = 0; i < num_threads; ++i)
	//	myThreads[i].join();
	thead1.join();
	thead2.join();

}

void testMoveOper()
{
	class A
	{
	public:
		A(const std::vector<double>& v)
			:data(v)
		{}
		//copy
		A(const A& other)
		{
			std::cout << "A copy ctor" << "\n";
			data = other.data;
			std::cout << "data " << *data.data() << "\n";
			std::cout << "other data " << other.data.size() << "\n";
		}
		//move
		A(A&& other)
		{
			std::cout << "A move ctor" << "\n";
			std::swap(data, other.data);
			std::cout << "data " << *data.data() << "\n";
			std::cout << "other data " << other.data.size() << "\n";
		}

		A& operator=(const A& other)
		{
			std::cout << "A copy assignment" << "\n";
			data = other.data;
			std::cout << "data " << *data.data() << "\n";
			std::cout << "other data " << other.data.size() << "\n";
			return *this;
		}

		A& operator=(A&& other)
		{
			std::cout << "A move assignment " << "\n";
			data = std::move(other.data);
			std::cout << "data " << *data.data() << "\n";
			std::cout << "other data " << other.data.size() << "\n";
			return *this;
		}

		double* operator[](const int x)
		{
			return &data[x];
		}
		

		std::vector<double> data;

	};

	std::vector<double> at = { 3.5, 6, 7, 8 };
	A a(at);
	std::vector<double> bt = { 5.5, 9, 10, 11 };
	A b(bt);
	std::vector<double> ct = { 7.5, 13, 15, 17 };
	A c(ct);
	std::vector<double> dt = { 9.5, 16, 21, 23 };
	A d(dt);

	A aa(a); //copy ctor
	A ab(std::move(b));//move ctor
	a = c; //copy assign
	a = std::move(d); //move assign
	A ac = aa; //copy assign
	A ad = std::move(ab); //move assign

	double* x = ad[0];
	std::cout<< *x << "\n";
	++x;
	std::cout << *x << "\n";

	int arr[5] = { 1,2,3,4,5 };
	int* arr_ptr = arr;
	//int* arr_ptr2 = { 1,2,3,4,5 };
	int* arr2[10]; //this is array of pointers

	
}

void matrixMultiply()
{
	int rows = 1000;
	int cols = 1000;

	int vec_size = rows * cols;
	//create 1000 x 1000
	std::vector<double> v(vec_size, 1.5);
	matrix<double> m(rows, cols);
	m.myVector = v;

	matrix<double> m2(rows, cols);
	m2.myVector = v;

	auto start = std::chrono::system_clock::now();
	//do multiplication
	matrix<double> res = matrixProduct(m, m2);
	
	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << "Time for addition " << dur.count() << " seconds" << std::endl;
	double res1 = std::accumulate(res.myVector.begin(), res.myVector.end(), 0.0);
	std::cout << "Matrix result " << res1 << std::endl;

	start = std::chrono::system_clock::now();
	//do multiplication
	matrix<double> res2 = matrixProduct2(m, m2);

	dur = std::chrono::system_clock::now() - start;
	std::cout << "Time for addition " << dur.count() << " seconds" << std::endl;
	double res22 = std::accumulate(res2.myVector.begin(), res2.myVector.end(), 0.0);
	std::cout << "Matrix result " << res22 << std::endl;


	start = std::chrono::system_clock::now();
	//do multiplication
	matrix<double> res3 = matrixProductMT(m, m2);

	dur = std::chrono::system_clock::now() - start;
	std::cout << "Time for addition " << dur.count() << " seconds" << std::endl;
	double res33 = std::accumulate(res3.myVector.begin(), res3.myVector.end(), 0.0);
	std::cout << "Matrix result " << res33 << std::endl;

}

void inheritanceTest()
{
	class Base {
	private:
		int pvt = 1;

	protected:
		int prot = 2;

	public:
		int pub = 3;

		// function to access private member
		int getPVT() {
			return pvt;
		}

		void setPVT(const int x)
		{
			pvt = x;
		}
	};

	class PublicDerived : public Base {
	public:
		// function to access protected member from Base
		int getProt() {
			return prot;
		}
	};

	PublicDerived d;
	d.pub;
	d.getPVT(); //is ok
	d.getProt();
	//all is same
	//PublicDerived* p = new Base;//this is illegal
	Base* b = new Base;
	PublicDerived* p = new PublicDerived;
	Base* bp = static_cast<Base*>(p);

	class ProtectedDerived : protected Base {
	public:

		//here pvt and prot are protected(from the viewpoint of outsiders) but are accessible from Derived class
		//pvt remains private and accessible from derived class

		// function to access protected member from Base
		int getProt() {
			return prot; //still accessible from the derived class
		}

		// function to access public member from Base
		int getPub() {
			return pub; //still accessible from the derived class
		}

		/*void setPVT(const int x)
		{
			pvt = x;
		}*/
	};

	ProtectedDerived P;
	//int a = P.getPVT(); //getPVT becomes protected in Derived so cannot be used from user
	int a = P.getProt(); //ok
	int bb = P.getPub(); //ok
	//int c = P.pub; //not ok
	//P.setPVT(3);


	class PrivateDerived : private Base {
	public:

		//here pvt and prot are private(from the viewpoint of outsiders) but are accessible from Derived class

		// function to access protected member from Base
		int getProt() {
			return prot; //still accessible from the derived class
		}

		// function to access private member
		int getPub() {
			return pub; //still accessible from the derived class
		}

		/*int getPVT() {
			return pvt;
		}*/

	};

	PrivateDerived PD;
	int ad = PD.getProt();
	int ac = PD.getPub();
}

void testInheritance()
{
	class A
	{
	public:
		A()
		{
			std::cout << "A construct" << std::endl;
		};
		int x;
	
		void setX(int i) { x = i; }
		//void print() { cout << x; }
	};

	class B : virtual public A
	{
	public:
		B():A() { 
			std::cout << "B construct" << std::endl;
			
			setX(10);
		};
	};

	class C : virtual public A
	{
	public:
		C():A() { 
			std::cout << "C construct" << std::endl;
			setX(20); }
	};

	class D : public B, public C {
	public:
		D() :
			B(), C(), A()
		{
			std::cout << "D construct" << std::endl;
		};
	};

	D d;
	std::cout << d.x << std::endl;
}

void templatesFnc()
{
	double a = 4;
	double b = 5;
	double c = func(a, b);
	std::cout << c << "\n";
}

int main() {

	//std::cout << "Hello World " << "\n";
	//std::cin.get();

	
	//testHelloWorldThreading();

	/*BankAccount bankAcc;
	bankAcc.deposit(800);
	bankAcc.withdraw(500);
	bankAcc.withdraw(500);
	std::cout << "Final Balace " << bankAcc.getBalance() << "\n";*/

	std::cout << "Test Threading " << "\n";
	//testBankAcc(); //here we have race condition as we pass reference to bank acc
	//testBankAcc_copy(); //here we dont have as we pass copy of bank acc
	//testBankAccLocked();// with mutex
	//testBankAccLockedAuto();// auto release
	//LinearSearchThreads();
	//NumberInSequence();
	//NumberInSequence2();
	//testMoveOper();
	//matrixMultiply();
	//testInheritance();
	templatesFnc();


	return 0;
}