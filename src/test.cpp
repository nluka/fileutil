#include <iostream>
#include <mutex>

#include "test.hpp"

using namespace std;
using test::Suite;

static vector<Suite> s_suites{};
#if TEST_THREADSAFE_REGISTRATION
static mutex s_suitesMutex{};
#endif
#if TEST_THREADSAFE_ASSERTS
static mutex s_assertsMutex{};
#endif

static bool s_useStdout = true;
void test::use_stdout(bool const b) {
  s_useStdout = b;
}

static ofstream *s_ofstream = nullptr;
void test::set_ofstream(ofstream *const ofs) {
  s_ofstream = ofs;
}

static char const *s_indent = "\t";
void test::set_indentation(char const *const i) {
  s_indent = i;
}

static bool s_verboseMode = false;
void test::set_verbose_mode(bool const b) {
  s_verboseMode = b;
}

Suite::Assertion::Assertion(char const *const name, bool const expr)
: m_name{name}, m_expr{expr}
{}

string const &Suite::Assertion::name() const noexcept {
  return m_name;
}
bool Suite::Assertion::expr() const noexcept {
  return m_expr;
}

Suite::Suite(char const *const name) : m_name{name} {}
Suite::Suite(string const &name) : m_name{name} {}

void Suite::assert(char const *const name, bool const expr) {
  #if TEST_THREADSAFE_ASSERTS
  scoped_lock const lock{s_assertsMutex};
  #endif
  m_assertions.emplace_back(name, expr);
}

void Suite::print_assertions(ostream *const os) const {
  for (auto const &a : m_assertions) {
    bool const passed = a.expr();
    if (!passed || (passed && s_verboseMode)) {
      *os << s_indent << (passed ? "pass" : "fail") << ": " << a.name() << '\n';
    }
  }
}

string const &Suite::name() const noexcept {
  return m_name;
}

size_t Suite::passes() const noexcept {
  return count_if(
    m_assertions.begin(), m_assertions.end(),
    [](Assertion const &a){
      return a.expr() == true;
    }
  );
}

size_t Suite::fails() const noexcept {
  return count_if(
    m_assertions.begin(), m_assertions.end(),
    [](Assertion const &a){
      return a.expr() == false;
    }
  );
}

void test::register_suite(Suite &&s) {
  #if TEST_THREADSAFE_REGISTRATION
  scoped_lock const lock(s_suitesMutex);
  #endif
  s_suites.push_back(s);
}

void test::evaluate_suites() {
  #if TEST_THREADSAFE_REGISTRATION
  scoped_lock const lock(s_suitesMutex);
  #endif

  for (auto const &s : s_suites) {
    auto const printHeader = [&s](ostream *const os){
      size_t const
        passes = s.passes(),
        cases = passes + s.fails();
      *os << s.name() << " (" << passes << '/' << cases << ")\n";
    };

    if (s_useStdout) {
      printHeader(&cout);
      s.print_assertions(&cout);
    }
    if (s_ofstream != nullptr) {
      printHeader(s_ofstream);
      s.print_assertions(s_ofstream);
    }
  }

  s_suites.clear();
}
