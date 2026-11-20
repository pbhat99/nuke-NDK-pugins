// Copyright (c) 2025 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NDK_BASE_SIGNAL_H
#define NDK_BASE_SIGNAL_H

//! A simple signals/slots mechanism.
//!
//! Use this when you want a callback/notification mechanism, and you don't want to inherit from
//! QObject or run "moc".
//!
//! Advantages:
//!  * signals can be static (i.e. not attached to a particular object)
//!  * slots are ordinary member functions of the receiving object or lambda functions
//!  * signals on an object can be individually enabled or disabled
//!  * no need to run a separate build step ("moc")
//!  * don't need to pollute your code with Qt classes
//!  * doesn't require Boost (Boost signals and Qt code don't get along at all)
//!  * you get compile-time errors when trying to connect non-existent signals or slots
//!
//! Disadvantages:
//!  * does not implement sender()
//!  * connection/disconnection is not thread safe
//!  * doesn't do queued connections between threads
//!  * signals can't be connected to other signals
//!
//! Connections are automatically disconnected when either the signal or receiver are destroyed.
//! To make this work for receivers, the receiver must own a SignalEndPoint object. It's safe to
//! not have such an object, but if so, it is your responsibility to make sure that all signals are
//! disconnected before the receiver is destroyed.

//!
//! Example usage:
//!
//! class Signals {
//! public:
//!   Signal<> sig0;
//!   Signal<int> sig1;
//! };
//!
//! class Slots : public SignalEndPoint  {
//! public:
//!   void slot0() {
//!     printf("slot0\n");
//!   }
//!
//!   void slot1( int a1 ) {
//!     printf("slot1 %d\n", a1 );
//!   }
//! };
//!
//!
//! Signals s;
//! Slots r;
//! s.sig0.connect(&r, &Slots::slot0);
//! s.sig1.connect(&r, &Slots::slot1);
//! s.sig1.connect(&r, &Slots::slot0);
//! s.sig0();
//! s.sig1( 1 );
//!
//! s.connect([](int n) { std::cout << "received " << n << std::endl; });
//!
//! If you don't want to inherit from SignalEndPoint but still want automatic disconnection on destruction:
//!
//! class Slots  {
//! public:
//!   ...
//!   SignalEndPoint* signalEndPoint() { return &_endPoint; }
//!   SignalEndPoint _endPoint;
//! };
//!

#include "ndk/api.h"

#include <algorithm>
#include <array>
#include <vector>
#include <functional>

namespace ndk {

  namespace detail {
    class ConnectionBase;
  }  // namespace detail

  //! This class manages one end of a signal connection. Its purpose is to clean
  //! up connections to or from an object when the object is destroyed.
  //! You can inherit from this if you want your class to receive signals. It's safe to use
  //! this as a mix-in class with other classes, including QObject.
  //! If you don't want to inherit, you can instead add a SignalEndPoint member variable and
  //! implement a signalEndPoint method which returns a pointer to it.
  //! Finally, if you want to live dangerously and not have the connections cleaned up on destruction,
  //! you can ignore this class altogether,
  //! An endpoint can be made inactive and will neither send nor receive signals. The class
  //! SignalBlocker provides a scoped way to do this.
  class NDK_API SignalEndPoint {
  public:
    SignalEndPoint() = default;

    //! Copies an end point but does not copy its connections.
    SignalEndPoint(const SignalEndPoint& other);

    virtual ~SignalEndPoint();

    //! Return true if this end point is connected to anything.
    bool isEndPointConnected() const { return !_connections.empty(); }

    //! Disconnect all signal connections to this object
    void disconnectEndPoint() const;

    SignalEndPoint* signalEndPoint() { return this; }

    void setEndPointActive(const bool active) { _active = active; }
    bool isEndPointActive() const { return _active; }

  protected:
    friend class detail::ConnectionBase;
    std::vector<detail::ConnectionBase*> _connections; //! Pointer to a list of connections to this object.
    bool _active = true;
  };

  namespace detail {
    // hasSignalEndPoint and the accompanying helper functions implement
    // compile-time checking for the existence of a member function with
    // a given signature on a class.
    //
    // The approach utilises a feature of C++ known as SFINAE (Substitution
    // Failure Is Not An Error), which allows for a template substitution
    // to fail without erroring as long as a suitable alternative can be
    // matched. In our case the compiler will attempt to match the first
    // version of hasSignalEndPointHelper() in order to resolve the call from
    // hasSignalEndPoint(). This can only succeed if class R implements a
    // valid signalEndPoint() function. If it doesn't, the substitution will
    // fail, and the compiler will go on to match the second version of
    // hasSignalEndPointHelper(), which will always succeed because it expects
    // a variadic argument list (i.e. anything).
    //
    // Some points of interest:
    // 1. The choice of an int argument in the first helper function is arbitrary.
    //    It just needs to be a specific type the compiler can attempt to match
    //    before falling through to the version that takes variadic arguments.
    // 2. "(R*)nullptr->signalEndPoint()" may look odd but its purpose is to
    //    produce a valid function signature for R::signalEndPoint(). As it will
    //    never be called at runtime, it is sufficient to achieve this by casting
    //    a nullptr to an R*.
    template <typename R, bool result = std::is_same_v<decltype(static_cast<R*>(nullptr)->signalEndPoint()), SignalEndPoint*>>
    constexpr bool hasSignalEndPointHelper(int /*unused*/) {
      return result;
    }

    template <typename... Args>
    constexpr bool hasSignalEndPointHelper(Args&&... /*unused*/) { return false; }

    template <typename R>
    constexpr bool hasSignalEndPoint() {
      return hasSignalEndPointHelper<R>(0);
    }

    //! Specialise this template to provide alternative ways of getting the SignalEndPointObject for a type
    // If the type doesn't have a signalEndPoint method, return nullptr
    template<typename T>
    std::enable_if_t<!detail::hasSignalEndPoint<T>(), SignalEndPoint*>signalEndPoint(T* /*t*/) {
      return nullptr;
    }

    // If the type does have a signalEndPoint method, call it and return the result
    template<typename T>
    std::enable_if_t<detail::hasSignalEndPoint<T>(), SignalEndPoint*>signalEndPoint(T* t) {
      return t->signalEndPoint();
    }


    //! A single signal/slot connection: This is a private class
    //! A connection can be destroyed when either its signal or receiver is destroyed or by
    //! a call to disconnect(). Each connection has a pointer to its signal and receiver,
    //! The signal one is here, but the receiver is in the Signal template below for reasons
    //! of having to template pointer-to-member-functions without any casting.
    //! We keep each connection in two linked lists, one hanging off the signal and one off
    //! the receiver. Deleting the connection removes it from both lists.
    class NDK_API ConnectionBase {
    public:
      enum EndPointType { eSignal, eReceiver };

      ConnectionBase() = default;
      ConnectionBase(const ConnectionBase& other) = delete;
      virtual ~ConnectionBase();

      void connect(SignalEndPoint* signal, SignalEndPoint* receiver);

      bool isActive(const EndPointType whichEnd) const { return (_endPoint[whichEnd] == nullptr || _endPoint[whichEnd]->isEndPointActive()); }
      bool isActiveReceiver() const { return isActive(eReceiver); }

    private:
      void link(EndPointType whichEnd, SignalEndPoint* endPoint);

      std::array<SignalEndPoint*, 2> _endPoint {}; //! The two endpoints for this connection
    };

    //! Base class for signals: Do not use this class directly.
    class NDK_API SignalBase : public SignalEndPoint
    {
    public:
      SignalBase() = default;
      SignalBase(const SignalBase& other) = delete;
      ~SignalBase() override = default;

      //! Return true if this end point is connected to anything.
      bool isConnected() const { return isEndPointConnected(); }

      //! Disconnect all signal connections to this object
      void disconnect() const { disconnectEndPoint(); }

      void setActive(const bool active) { setEndPointActive(active); }
      bool isActive() const { return isEndPointActive(); }
    };


    // constexpr template to calculate the number of arguments for various types of function
    template <typename>
    struct num_arguments;

    // Plain function pointer
    template <typename Return, typename ... Args>
    struct num_arguments<Return(*)(Args...)> : std::integral_constant<std::size_t, sizeof...(Args)>
    {
    };

    // std::function
    template <typename Return, typename ... Args>
    struct num_arguments<std::function<Return(Args...)>> : std::integral_constant<std::size_t, sizeof...(Args)>
    {
    };

    // Pointer to member function
    template <typename F, typename Return, typename ... Args>
    struct num_arguments<Return (F::*)(Args...)> : std::integral_constant<std::size_t, sizeof...(Args)>
    {
    };

    // Pointer to member function
    template <typename F, typename Return, typename ... Args>
    struct num_arguments<Return (F::*)(Args...) const> : std::integral_constant<std::size_t, sizeof...(Args)>
    {
    };

    template <typename Lambda>
    struct num_arguments : num_arguments<decltype(&Lambda::operator())>
    {
    };

    //! Base class for signal/slot connections
    template <typename ... CArgs>
    class TConnectionBase : public ConnectionBase {
    public:
      virtual void call(CArgs ...args) const = 0;
    };

    //! A connection to a member function
    template <class Object, class Method, typename ... CArgs>
    class Connection: public TConnectionBase<CArgs...> {
    public:
      Connection(Object* object, Method f)
        : _object(object)
        , _method(f)
      {
        static_assert(num_arguments<Method>::value == sizeof...(CArgs),
                      "Slot must have the same number of parameters as the signal");
      }

      template <std::size_t ... NumArgs>
      void doCall(std::tuple<CArgs...> const& t, std::index_sequence<NumArgs...> const & /*unused*/) const
      {
        if (this->isActiveReceiver()) {
          (_object->*_method)(std::get<NumArgs>(t)...);
        }
      }

      void call(CArgs... args) const override
      {
        doCall(std::make_tuple(args...), std::make_index_sequence<num_arguments<Method>::value>{});
      }

      bool isConnectedTo(Object* object) const { return _object == object; }
      bool isConnectedTo(Object* object, Method method) const { return _object == object && _method == method; }

    private:
      Object* _object;
      Method _method;
    };

    //! A connection to a free function, lambda etc.
    template <typename Callable, typename ... CArgs>
    class FunctionConnection: public TConnectionBase<CArgs...> {
    public:
      explicit FunctionConnection(const Callable& callable) : _callable(callable)
      {
        static_assert(num_arguments<Callable>::value == sizeof...(CArgs),
                      "Callable must have the same number of parameters as the signal");
      }

      template <std::size_t ... NumArgs>
      void doCall(std::tuple<CArgs...> const& t, std::index_sequence<NumArgs...> const& /*unused*/) const
      {
        _callable(std::get<NumArgs>(t)...);
      }

      void call(CArgs... args) const override
      {
        doCall(std::make_tuple(args...), std::make_index_sequence<num_arguments<Callable>::value>{});
      }

    private:
      Callable _callable;
    };
  }  // namespace detail

  //! A signal.
  template <typename ... Args>
  class Signal : public detail::SignalBase
  {
  public:
    Signal() = default;

    //! Connect the signal to a generic callable
    template <class Callable>
    void connect(Callable callable)
    {
      (new detail::FunctionConnection<Callable, Args...>(callable))->connect(this, nullptr);
    }

    //! Connect the signal to a member function
    template <class Object, class Callable>
    void connect(Object* receiver, Callable method)
    {
      (new detail::Connection<Object, Callable, Args...>(receiver, method))->connect(this, detail::signalEndPoint(receiver));
    }

    using SignalBase::disconnect;

    //! Disconnect a connection to a specific slot
    template <class Object>
    void disconnect(Object* receiver, void (Object::*f)(Args... args))
    {
      using Method = void (Object::*)(Args...);
      auto tail = std::remove_if(_connections.begin(), _connections.end(),
        [receiver, f](detail::ConnectionBase* connection) {
          return static_cast<detail::Connection<Object, Method, Args...>*>(connection)->isConnectedTo(receiver, f);
        });

      size_t toDelete = _connections.end() - tail;
      while (toDelete--) {
        // This will cause the element to be removed from the vector via
        // ~ConnectionBase
        delete _connections.back();
      }
    }

    //! Disconnect all connections to a receiver
    template <class Object>
    void disconnect(Object* receiver)
    {
      using Method = void (Object::*)(Args...);
      auto tail = std::remove_if(_connections.begin(), _connections.end(),
        [receiver](detail::ConnectionBase* connection) {
          return static_cast<detail::Connection<Object, Method, Args...>*>(connection)->isConnectedTo(receiver);
        });

      size_t toDelete = _connections.end() - tail;
      while (toDelete--) {
        // This will cause the element to be removed from the vector via
        // ~ConnectionBase
        delete _connections.back();
      }
    }

    //! Emit the signal
    void operator()(Args... args) const
    {
      if (isActive()) {
        for (auto connection : _connections) {
          auto s = static_cast<detail::TConnectionBase<Args...>*>(connection);
          s->call(args...);
        }
      }
    }
  };


  //! An RAII-style class for temporarily disabling a signal
  class NDK_API SignalBlocker
  {
  public:
    explicit SignalBlocker(SignalEndPoint& endPoint)
      : _endPoint(endPoint)
    {
      _active = _endPoint.isEndPointActive();
      _endPoint.setEndPointActive(false);
    }

    template<class T>
    explicit SignalBlocker(T& endPoint)
      : SignalBlocker(*detail::signalEndPoint(&endPoint))
    {
    }

    ~SignalBlocker()
    {
      _endPoint.setEndPointActive(_active);
    }

  private:
    bool _active;
    SignalEndPoint& _endPoint;
  };

}  // namespace ndk

#endif  // NDK_BASE_SIGNAL_H
