/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Database/Database.hpp"

#include <functional>
#include <tuple>
#include <utility>
#include <vector>

class CallbackBase
{
public:
    virtual ~CallbackBase() = default;
    virtual void execute() = 0;
};

using QueryResultVector = std::vector<AsyncQueryResult>;

namespace callbackDetail
{
    template <typename Callable, typename Tuple>
    void invokeCallable(Callable&& callable, Tuple& args)
    {
        std::apply(
            [&callable](auto&... storedArgs)
            {
                std::invoke(std::forward<Callable>(callable), storedArgs...);
            },
            args);
    }

    template <typename Object, typename Method, typename Tuple>
    void invokeMember(Object* object, Method method, Tuple& args)
    {
        std::apply(
            [object, method](auto&... storedArgs)
            {
                std::invoke(method, object, storedArgs...);
            },
            args);
    }

    inline QueryResult* getFirstQueryResult(QueryResultVector& result) noexcept
    {
        if (result.empty())
        {
            return nullptr;
        }

        return result.front().result.get();
    }

    template <typename Callable, typename Tuple>
    void invokeSqlFunction(Callable&& callable, QueryResultVector& result, Tuple& args)
    {
        QueryResult* queryResult = getFirstQueryResult(result);

        std::apply(
            [&callable, queryResult](auto&... storedArgs)
            {
                std::invoke(std::forward<Callable>(callable), queryResult, storedArgs...);
            },
            args);
    }
}

class CallBackFunctionP0 : public CallbackBase
{
public:
    using cbFunction = void (*)();

    explicit CallBackFunctionP0(cbFunction function) : m_function(function) {}
    void operator()() { std::invoke(m_function); }
    void execute() override { operator()(); }

private:
    cbFunction m_function;
};

template <typename P1>
class CallBackFunctionP1 : public CallbackBase
{
public:
    using cbFunction = void (*)(P1);

    CallBackFunctionP1(cbFunction function, P1 p1) : m_function(function), m_args(p1) {}
    void operator()() { callbackDetail::invokeCallable(m_function, m_args); }
    void execute() override { operator()(); }

private:
    cbFunction m_function;
    std::tuple<P1> m_args;
};

template <typename P1, typename P2>
class CallBackFunctionP2 : public CallbackBase
{
public:
    using cbFunction = void (*)(P1, P2);

    CallBackFunctionP2(cbFunction function, P1 p1, P2 p2) : m_function(function), m_args(p1, p2) {}
    void operator()() { callbackDetail::invokeCallable(m_function, m_args); }
    void execute() override { operator()(); }

private:
    cbFunction m_function;
    std::tuple<P1, P2> m_args;
};

template <typename P1, typename P2, typename P3>
class CallBackFunctionP3 : public CallbackBase
{
public:
    using cbFunction = void (*)(P1, P2, P3);

    CallBackFunctionP3(cbFunction function, P1 p1, P2 p2, P3 p3) : m_function(function), m_args(p1, p2, p3) {}
    void operator()() { callbackDetail::invokeCallable(m_function, m_args); }
    void execute() override { operator()(); }

private:
    cbFunction m_function;
    std::tuple<P1, P2, P3> m_args;
};

template <typename P1, typename P2, typename P3, typename P4>
class CallBackFunctionP4 : public CallbackBase
{
public:
    using cbFunction = void (*)(P1, P2, P3, P4);

    CallBackFunctionP4(cbFunction function, P1 p1, P2 p2, P3 p3, P4 p4) : m_function(function), m_args(p1, p2, p3, p4) {}
    void operator()() { callbackDetail::invokeCallable(m_function, m_args); }
    void execute() override { operator()(); }

private:
    cbFunction m_function;
    std::tuple<P1, P2, P3, P4> m_args;
};

template <class Class>
class CallbackP0 : public CallbackBase
{
public:
    using Method = void (Class::*)();

    CallbackP0(Class* classInstance, Method method) : m_obj(classInstance), m_func(method) {}
    void operator()() { std::invoke(m_func, m_obj); }
    void execute() override { operator()(); }

private:
    Class* m_obj;
    Method m_func;
};

template <class Class, typename P1>
class CallbackP1 : public CallbackBase
{
public:
    using Method = void (Class::*)(P1);

    CallbackP1(Class* classInstance, Method method, P1 p1) : m_obj(classInstance), m_func(method), m_args(p1) {}
    void operator()() { callbackDetail::invokeMember(m_obj, m_func, m_args); }
    void execute() override { operator()(); }

private:
    Class* m_obj;
    Method m_func;
    std::tuple<P1> m_args;
};

template <class Class, typename P1, typename P2>
class CallbackP2 : public CallbackBase
{
public:
    using Method = void (Class::*)(P1, P2);

    CallbackP2(Class* classInstance, Method method, P1 p1, P2 p2) : m_obj(classInstance), m_func(method), m_args(p1, p2) {}
    void operator()() { callbackDetail::invokeMember(m_obj, m_func, m_args); }
    void execute() override { operator()(); }

private:
    Class* m_obj;
    Method m_func;
    std::tuple<P1, P2> m_args;
};

template <class Class, typename P1, typename P2, typename P3>
class CallbackP3 : public CallbackBase
{
public:
    using Method = void (Class::*)(P1, P2, P3);

    CallbackP3(Class* classInstance, Method method, P1 p1, P2 p2, P3 p3) : m_obj(classInstance), m_func(method), m_args(p1, p2, p3) {}
    void operator()() { callbackDetail::invokeMember(m_obj, m_func, m_args); }
    void execute() override { operator()(); }

private:
    Class* m_obj;
    Method m_func;
    std::tuple<P1, P2, P3> m_args;
};

template <class Class, typename P1, typename P2, typename P3, typename P4>
class CallbackP4 : public CallbackBase
{
public:
    using Method = void (Class::*)(P1, P2, P3, P4);

    CallbackP4(Class* classInstance, Method method, P1 p1, P2 p2, P3 p3, P4 p4) : m_obj(classInstance), m_func(method), m_args(p1, p2, p3, p4) {}
    void operator()() { callbackDetail::invokeMember(m_obj, m_func, m_args); }
    void execute() override { operator()(); }

private:
    Class* m_obj;
    Method m_func;
    std::tuple<P1, P2, P3, P4> m_args;
};

class SQLCallbackBase
{
public:
    virtual ~SQLCallbackBase();
    virtual void run(QueryResultVector& result) = 0;
};

template <class T>
class SQLClassCallbackP0 : public SQLCallbackBase
{
public:
    using scMethod = void (T::*)(QueryResultVector& result);

    SQLClassCallbackP0(T* instance, scMethod method) : m_base(instance), m_method(method) {}
    ~SQLClassCallbackP0() override = default;
    void run(QueryResultVector& data) override { std::invoke(m_method, m_base, data); }

private:
    T* m_base;
    scMethod m_method;
};

template <class T, typename P1>
class SQLClassCallbackP1 : public SQLCallbackBase
{
public:
    using scMethod = void (T::*)(QueryResultVector& result, P1 p1);

    SQLClassCallbackP1(T* instance, scMethod method, P1 p1) : m_base(instance), m_method(method), m_args(p1) {}
    ~SQLClassCallbackP1() override = default;
    void run(QueryResultVector& data) override
    {
        std::apply(
            [this, &data](auto&... storedArgs)
            {
                std::invoke(m_method, m_base, data, storedArgs...);
            },
            m_args);
    }

private:
    T* m_base;
    scMethod m_method;
    std::tuple<P1> m_args;
};

template <class T, typename P1, typename P2>
class SQLClassCallbackP2 : public SQLCallbackBase
{
public:
    using scMethod = void (T::*)(QueryResultVector& result, P1 p1, P2 p2);

    SQLClassCallbackP2(T* instance, scMethod method, P1 p1, P2 p2) : m_base(instance), m_method(method), m_args(p1, p2) {}
    ~SQLClassCallbackP2() override = default;
    void run(QueryResultVector& data) override
    {
        std::apply(
            [this, &data](auto&... storedArgs)
            {
                std::invoke(m_method, m_base, data, storedArgs...);
            },
            m_args);
    }

private:
    T* m_base;
    scMethod m_method;
    std::tuple<P1, P2> m_args;
};

template <class T, typename P1, typename P2, typename P3>
class SQLClassCallbackP3 : public SQLCallbackBase
{
public:
    using scMethod = void (T::*)(QueryResultVector& result, P1 p1, P2 p2, P3 p3);

    SQLClassCallbackP3(T* instance, scMethod method, P1 p1, P2 p2, P3 p3) : m_base(instance), m_method(method), m_args(p1, p2, p3) {}
    ~SQLClassCallbackP3() override = default;
    void run(QueryResultVector& data) override
    {
        std::apply(
            [this, &data](auto&... storedArgs)
            {
                std::invoke(m_method, m_base, data, storedArgs...);
            },
            m_args);
    }

private:
    T* m_base;
    scMethod m_method;
    std::tuple<P1, P2, P3> m_args;
};

template <class T, typename P1, typename P2, typename P3, typename P4>
class SQLClassCallbackP4 : public SQLCallbackBase
{
public:
    using scMethod = void (T::*)(QueryResultVector& result, P1 p1, P2 p2, P3 p3, P4 p4);

    SQLClassCallbackP4(T* instance, scMethod method, P1 p1, P2 p2, P3 p3, P4 p4) : m_base(instance), m_method(method), m_args(p1, p2, p3, p4) {}
    ~SQLClassCallbackP4() override = default;
    void run(QueryResultVector& data) override
    {
        std::apply(
            [this, &data](auto&... storedArgs)
            {
                std::invoke(m_method, m_base, data, storedArgs...);
            },
            m_args);
    }

private:
    T* m_base;
    scMethod m_method;
    std::tuple<P1, P2, P3, P4> m_args;
};

class SQLFunctionCallbackP0 : public SQLCallbackBase
{
public:
    using scMethod = void (*)(QueryResult*);

    explicit SQLFunctionCallbackP0(scMethod method) : m_method(method) {}
    ~SQLFunctionCallbackP0() override = default;
    void run(QueryResultVector& data) override
    {
        QueryResult* queryResult = callbackDetail::getFirstQueryResult(data);
        std::invoke(m_method, queryResult);
    }
    void run(QueryResult* data) { std::invoke(m_method, data); }

private:
    scMethod m_method;
};

template <typename T1>
class SQLFunctionCallbackP1 : public SQLCallbackBase
{
public:
    using scMethod = void (*)(QueryResult*, T1 p1);

    SQLFunctionCallbackP1(scMethod method, T1 p1) : m_method(method), m_args(p1) {}
    ~SQLFunctionCallbackP1() override = default;
    void run(QueryResultVector& data) override { callbackDetail::invokeSqlFunction(m_method, data, m_args); }
    void run(QueryResult* data)
    {
        std::apply(
            [this, data](auto&... storedArgs)
            {
                std::invoke(m_method, data, storedArgs...);
            },
            m_args);
    }

private:
    scMethod m_method;
    std::tuple<T1> m_args;
};
