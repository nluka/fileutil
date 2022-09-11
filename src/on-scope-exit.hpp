#ifndef CPPLIB_ONSCOPEEXIT_HPP
#define CPPLIB_ONSCOPEEXIT_HPP

template <typename Func>
class OnScopeExit {
public:
  OnScopeExit() = delete;

  OnScopeExit(Func fn) : m_fn(fn) {}

  ~OnScopeExit() {
    m_fn();
  }

private:
  Func const m_fn;
};

template <typename Func>
OnScopeExit<Func> make_on_scope_exit(Func const &fn) {
  return OnScopeExit<Func>(fn);
}

#endif // CPPLIB_ONSCOPEEXIT_HPP
