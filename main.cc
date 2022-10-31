#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "scheduler.hh"

int func(int a) {
  std::cout << std::this_thread::get_id() << '\n';
  return a;
}

int func2() {
  std::cout << std::this_thread::get_id() << '\n';
  return 20;
}

int main() {
  thread::Scheduler s(8);

  auto f = s.schedule(func, 10);

  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);

  std::this_thread::sleep_for(std::chrono::microseconds(100));

  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);
  s.schedule(func2);

  std::this_thread::sleep_for(std::chrono::microseconds(100));

  std::cout << f.get() << '\n';

  return 0;
}