/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */
/*
 * testThread.cpp
 *
 *  Created on: 2010.11
 *      Author: Marty Kraimer
 */

#include <cstddef>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <cstdio>
#include <cstring>
#include <list>

#include <epicsUnitTest.h>
#include <testMain.h>


#include <pv/event.h>
#include <pv/thread.h>
#include <pv/executor.h>
#include <pv/timeFunction.h>

using namespace epics::pvData;
using std::string;

static string actionName("action");

class Action;
typedef std::tr1::shared_ptr<Action> ActionPtr;

class Action : public Runnable {
public:
    virtual ~Action() {}
    FILE *out;
    bool actuallyRan;
    Event begin, end;
    Action(): actuallyRan(false) {}
    virtual void run() {
        testDiag("Action waiting");
        begin.signal();
        bool waited=end.wait();
        actuallyRan=true;
        testDiag("Action %s", waited?"true":"false");
    }
};

static void testThreadRun() {
    // show that we can control thread start and stop
    ActionPtr ax(new Action());
    {
        ThreadPtr tr(new Thread(actionName,lowPriority,ax.get()));
        bool w=ax->begin.wait();
        testDiag( "main %s", w?"true":"false");
        testDiag( "Action is %s", ax->actuallyRan?"true":"false");
        ax->end.signal();
    }
    testOk1(ax->actuallyRan==true);
    testDiag( "Action is %s", ax->actuallyRan?"true":"false");
    testDiag("testThreadRun PASSED");
}

class Basic;
typedef std::tr1::shared_ptr<Basic> BasicPtr;

class Basic :
     public Command,
     public std::tr1::enable_shared_from_this<Basic>
{
public:
    POINTER_DEFINITIONS(Basic);
    Basic(ExecutorPtr const &executor)
    : executor(executor) {}
    ~Basic()
    {
    }
    void run()
    {
        executor->execute(getPtrSelf());
        bool result = wait.wait();
        testOk1(result==true);
        if(result==false) testDiag("basic::run wait returned false");
    }
    virtual void command()
    {
        wait.signal();
    }
private:
    Basic::shared_pointer getPtrSelf()
    {
        return shared_from_this();
    }
    ExecutorPtr executor;
    Event wait;
};

typedef std::tr1::shared_ptr<Basic> BasicPtr;

static void testBasic() {
    ExecutorPtr executor(new Executor(string("basic"),middlePriority));
    BasicPtr basic( new Basic(executor));
    basic->run();
    testDiag("testBasic PASSED");
}

namespace {
struct fninfo {
    int cnt;
    epicsEvent evnt;
};

static void threadFN(void *raw)
{
    fninfo *arg = (fninfo*)raw;
    arg->evnt.signal();
    arg->cnt++;
}

struct classMeth {
    int cnt;
    epicsEvent evnt;
    classMeth() :cnt(0) {}
    void inc() {
        const char *tname = epicsThreadGetNameSelf();
        testOk(strcmp(tname, "test2")==0, "thread name '%s' == 'test2' ", tname);
        evnt.signal();
        cnt++;
    }
};
}

static void testBinders()
{
    testDiag("Testing thread bindables");

    testDiag("C style function");
    {
        fninfo info;
        info.cnt = 0;
        Thread foo(Thread::Config(&threadFN, (void*)&info)
                   .name("test1")
                   .prio(epicsThreadPriorityMedium)
                   .autostart(true)
                   );

        info.evnt.wait();

        foo.exitWait();

        testOk(info.cnt==1, "cnt (%d) == 1", info.cnt);
    }

    testDiag("class method");
    {
        classMeth inst;

        Thread foo(Thread::Config(&inst, &classMeth::inc)
                   .prio(epicsThreadPriorityMedium)
                   .autostart(false)
                   <<"test"<<2
                   );

        epicsThreadSleep(0.1);

        testOk(inst.cnt==0, "inst.cnt (%d) == 0", inst.cnt);

        foo.start();
        inst.evnt.wait();
        foo.exitWait();

        testOk(inst.cnt==1, "inst.cnt (%d) == 1", inst.cnt);
    }

    testDiag("C++11 style lambda");
#if __cplusplus>=201103L
    {
        int cnt = 0;
        epicsEvent evnt;
        auto fn = [&cnt,&evnt]() mutable {evnt.signal(); cnt++;};
        Thread foo(Thread::Config(fn)
                   .name("test3")
                   .prio(epicsThreadPriorityMedium)
                   .autostart(true)
                   );
        evnt.wait();
        foo.exitWait();

        testOk(cnt==1, "cnt (%d) == 1", cnt);
    }
#else
    testSkip(1, "Not built as C++11");
#endif
}

class MyFunc : public TimeFunctionRequester {
public:
    POINTER_DEFINITIONS(MyFunc);
    MyFunc(BasicPtr const &basic);
    virtual void function();
private:
    BasicPtr basic;
};

MyFunc::MyFunc(BasicPtr const &basic)
    : basic(basic)
    {}
void MyFunc::function()
{
    basic->run();
}


typedef std::tr1::shared_ptr<MyFunc> MyFuncPtr;

#ifdef TESTTHREADCONTEXT

static void testThreadContext() {
    ExecutorPtr executor(new Executor(string("basic"),middlePriority));
    BasicPtr basic(new Basic(executor));
    MyFuncPtr myFunc(new MyFunc(basic));
    TimeFunctionPtr timeFunction(new TimeFunction(myFunc));
    double perCall = timeFunction->timeCall();
    perCall *= 1e6;
    testDiag("time per call %f microseconds",perCall);
    testDiag("testThreadContext PASSED");
}
#endif

MAIN(testThread)
{
    testPlan(7);
    testDiag("Tests thread");
    testThreadRun();
    testBasic();
    testBinders();
#ifdef TESTTHREADCONTEXT
    testThreadContext();
#endif
    return testDone();
}
