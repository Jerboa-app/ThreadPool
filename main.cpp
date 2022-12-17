/*

    stack overflows 
        https://stackoverflow.com/questions/26516683/reusing-thread-in-loop-c
        https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
        https://stackoverflow.com/questions/3929774/how-much-overhead-is-there-when-creating-a-thread
        https://stackoverflow.com/questions/17348228/code-runs-6-times-slower-with-2-threads-than-with-1

*/

#include <iostream>
#include <random>
#include <chrono>
using namespace std::chrono;

#include "ThreadPool.h"

void work(
  std::vector<float> & a, 
  std::vector<float> & b, 
  std::vector<float> & c, 
  size_t i
  ){

    double x = std::sqrt(a[i]) * std::sqrt(b[i]) * std::cos(a[i]/b[i]);
    double y = std::tan(a[i]) / std::sqrt(b[i]) * std::cos(a[i]/b[i]);
    c[i] = x+y;
}

void threadWork(
  std::vector<float> & a, 
  std::vector<float> & b, 
  std::vector<float> & c, 
  std::vector<size_t> & ids
  ){
    for (int i = 0; i < ids.size(); i++){
      double x = std::sqrt(a[ids[i]]) * std::sqrt(b[ids[i]]) * std::cos(a[ids[i]]/b[ids[i]]);
      double y = std::tan(a[ids[i]]) / std::sqrt(b[ids[i]]) * std::cos(a[ids[i]]/b[ids[i]]);
      c[ids[i]] = x+y;
    }
}

const size_t N = 1000000;
std::default_random_engine e;
std::uniform_real_distribution<float> U;

int main(){
  std::vector<float> a,b,c1,c2;
  for (int i = 0; i < N; i++){
    a.push_back(U(e));
    b.push_back(U(e));
    c1.push_back(0.0);
    c2.push_back(0.0);
  }

  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  for (int i = 0; i < N; i++){
    work(a,b,c1,i);
  }

  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "serial time/per function call " << time_span.count()  << ", " << time_span.count()/float(N) << "\n";

  int n = N/2.0;

  std::vector<size_t> i1,i2;

  // so each thread is not accessing memory right
  // next to one another (false sharing)
  for (int i = 0; i < N; i++){
    if (i < N/2.0){
      i1.push_back(i);
    }
    else{
      i2.push_back(i);
    }
  }

  t1 = high_resolution_clock::now();

  std::thread j1(threadWork,std::ref(a),std::ref(b),std::ref(c2),std::ref(i1));
  std::thread j2(threadWork,std::ref(a),std::ref(b),std::ref(c2),std::ref(i2));

  j1.join(); j2.join();

  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "2 threaded time: " << time_span.count() << "\n";

  for (int i = 0; i < N; i++){
    assert(c2[i] == c1[i]);
  }

  for (int i = 0; i < N; i++){
    c2[i]=0.0;
  }

  ThreadPool pool(2);

  t1 = high_resolution_clock::now();

  pool.queueJob(std::bind(threadWork,std::ref(a),std::ref(b),std::ref(c2),std::ref(i1)));
  pool.queueJob(std::bind(threadWork,std::ref(a),std::ref(b),std::ref(c2),std::ref(i2)));

  pool.wait();

  t2 = high_resolution_clock::now();
  time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "thread pool(2) time: " << time_span.count() << "\n";

  for (int i = 0; i < N; i++){
    assert(c2[i] == c1[i]);
  }

  pool.stop();

}
