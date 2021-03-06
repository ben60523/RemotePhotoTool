//
// RemotePhotoTool - remote camera control software
// Copyright (C) 2008-2015 Michael Fink
//
/// \file TestLuaState.cpp Tests for Lua::State class
//

// includes
#include "stdafx.h"
#include "CppUnitTest.h"
#include "Lua.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LuaScriptingUnitTest
{
   /// tests Lua::State class
   TEST_CLASS(TestLuaState)
   {
   public:
      /// tests LoadSourceString()
      TEST_METHOD(TestLoadSourceString)
      {
         Lua::State state;

         state.LoadSourceString(_T("-- comment test"));
         state.LoadSourceString(_T("function run() return 42; end"));

         Assert::IsTrue(state.GetValue(_T("run")).GetType() == Lua::Value::typeFunction,
            _T("run must be of type function"));

         state.CallFunction(_T("run"));
      }

      /// tests error handling
      TEST_METHOD(TestStateErrorHandling)
      {
         Lua::State state;
         state.LoadSourceString(_T("function run() error(); end"));

         try
         {
            state.CallFunction(_T("run"));
         }
         catch (const Lua::Exception&)
         {
            // ok
            return;
         }
         catch (...)
         {
            Assert::Fail(_T("must throw Lua::Exception and nothing else"));
         }

         Assert::Fail(_T("must throw exception"));
      }

      /// tests RequireLib() method to load built-in libraries
      TEST_METHOD(TestStateRequireLib)
      {
         // setup
         Lua::State state;

         // run
         state.RequireLib("coroutine");

         // check
         Lua::Value value = state.GetValue(_T("coroutine"));
         Assert::IsTrue(value.GetType() == Lua::Value::typeTable, _T("type must be table"));

         Lua::Table table = state.GetTable(_T("coroutine"));
         Lua::Value funcYield = table.GetValue(_T("yield"));

         Assert::IsTrue(funcYield.GetType() == Lua::Value::typeFunction, _T("type must be function"));
      }

      /// tests temporary nil value handling
      TEST_METHOD(TestValueNilTemporary)
      {
         // set up
         Lua::State state;

         // run
         for (int i = 0; i < 100; i++)
         {
            Lua::Value value = state.GetValue("not_init");
            Assert::IsTrue(Lua::Value::typeNil == value.GetType(), _T("type must be nil"));
         }

         // check
         // must not overflow stack
      }

      /// tests CallFunction(), without return parameters
      TEST_METHOD(TestCallFunctionNoRet)
      {
         Lua::State state;

         state.LoadSourceString(_T("function run() return; end"));

         std::vector<Lua::Value> vecRetvals = state.CallFunction(_T("run"));
         Assert::AreEqual<size_t>(0, vecRetvals.size(), _T("must have returned zero return values"));
      }

      /// tests CallFunction(), with single return parameter
      TEST_METHOD(TestCallFunctionRetval)
      {
         Lua::State state;

         state.LoadSourceString(_T("function run() return 42; end"));

         std::vector<Lua::Value> vecRetvals = state.CallFunction(_T("run"), 1);
         Assert::AreEqual<size_t>(1, vecRetvals.size(), _T("must have returned one return value"));

         const Lua::Value& value = vecRetvals[0];
         Assert::IsTrue(Lua::Value::typeNumber == value.GetType(), _T("type must be number"));
         Assert::AreEqual(value.Get<double>(), 42.0, 1e-6, _T("value must be 42"));
      }

      /// tests CallFunction(), with multiple return parameters
      TEST_METHOD(TestCallFunctionMultipleRetvals)
      {
         Lua::State state;

         state.LoadSourceString(_T("function test() return 42, 42==6*7, \"123abc\", nil; end"));

         std::vector<Lua::Value> vecRetvals = state.CallFunction(_T("test"), 4);
         Assert::AreEqual<size_t>(4, vecRetvals.size(), _T("must have returned 4 return values"));

         Assert::IsTrue(Lua::Value::typeNumber == vecRetvals[0].GetType(), _T("type must be number"));
         Assert::IsTrue(Lua::Value::typeBoolean == vecRetvals[1].GetType(), _T("type must be boolean"));
         Assert::IsTrue(Lua::Value::typeString == vecRetvals[2].GetType(), _T("type must be string"));
         Assert::IsTrue(Lua::Value::typeNil == vecRetvals[3].GetType(), _T("type must be nil"));
      }

      TEST_METHOD(TestAddTable)
      {
         Lua::State state;

         {
            Lua::Table table = state.AddTable(_T("abc"));
         }

         state.LoadSourceString(_T("function test() return abc; end"));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::IsTrue(Lua::Value::typeTable == vecRetval[0].GetType(), _T("type must be table"));
      }

      TEST_METHOD(TestGetTable)
      {
         Lua::State state;

         {
            Lua::Table table1 = state.AddTable(_T("abc"));
         }

         Lua::Table table2 = state.GetTable(_T("abc"));
      }

      TEST_METHOD(TestTableAddValue)
      {
         Lua::State state;

         {
            Lua::Table table1 = state.AddTable(_T("abc"));
            table1.AddValue(_T("def"), Lua::Value(42.0));
         }

         {
            state.LoadSourceString(_T("function test() return abc.def; end"));

            std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            const Lua::Value& val = vecRetval[0];
            Assert::IsTrue(Lua::Value::typeNumber == vecRetval[0].GetType(), _T("type must be number"));
            Assert::AreEqual(val.Get<double>(), 42.0, 1e-6, _T("value must be 42"));
         }
      }

      TEST_METHOD(TestTableAddFunction)
      {
         Lua::State state;

         // define a function to call back
         Lua::T_fnCFunction fn = [&] (Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value");

            double d = vecParams[0].Get<double>();

            std::vector<Lua::Value> vecRetValues;

            vecRetValues.push_back(Lua::Value(d));
            vecRetValues.push_back(Lua::Value(d+d));
            vecRetValues.push_back(Lua::Value(d*d));

            return vecRetValues;
         };

         {
            Lua::Table table1 = state.AddTable(_T("abc"));
            table1.AddFunction("run", fn);
         }

         {
            state.LoadSourceString(_T("function test() return abc.run(42.0); end"));

            std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 3);

            Assert::AreEqual<size_t>(3, vecRetval.size(), _T("must have returned 3 return value"));

            const Lua::Value& val0 = vecRetval[0];
            Assert::IsTrue(Lua::Value::typeNumber == val0.GetType(), _T("type must be number"));
            Assert::AreEqual(val0.Get<double>(), 42.0, 1e-6, _T("value must be 42"));

            Assert::AreEqual(vecRetval[1].Get<double>(), 42.0+42.0, 1e-6, _T("value must be 42+42"));

            Assert::AreEqual(vecRetval[2].Get<double>(), 42.0*42.0, 1e-6, _T("value must be 42*42"));
         }
      }

      TEST_METHOD(TestTableCallFunction)
      {
         Lua::State state;

         // setup
         state.LoadSourceString(_T("abc = { def = function(self, val) return val, val+val, val*val end }"));

         // run
         Lua::Table table = state.GetTable(_T("abc"));

         std::vector<Lua::Value> vecParam;
         vecParam.push_back(Lua::Value(42.0));

         std::vector<Lua::Value> vecRetval = table.CallFunction(_T("def"), 3, vecParam);

         // check
         Assert::AreEqual<size_t>(3, vecRetval.size(), _T("must have returned 3 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
         Assert::AreEqual(vecRetval[1].Get<double>(), 42.0+42.0, 1e-6, _T("value must be 42+42"));
         Assert::AreEqual(vecRetval[2].Get<double>(), 42.0*42.0, 1e-6, _T("value must be 42*42"));
      }

      /// tests method State::AddUserdata(), and value isn't stored anywhere
      TEST_METHOD(TestStateAddUserdataTemporary)
      {
         // setup
         Lua::State state;

         // run
         {
            Lua::Userdata userdata = state.AddUserdata(4);
         }

         // check
         // must not crash
      }

      /// tests method State::AddUserdata() and copy ctor of Userdata
      TEST_METHOD(TestStateUserdataCopyCtor)
      {
         // setup
         Lua::State state;

         // run
         {
            Lua::Userdata userdata = state.AddUserdata(4);
            Lua::Userdata userdata2 = userdata; // copy ctor

            Lua::Userdata userdata3 = userdata;
            userdata3 = userdata; // assign operator
         }

         // check
         // must not crash
      }

      /// tests method State::AddUserdata() and stores it as global value
      TEST_METHOD(TestStateAddUserdataStoreGlobal)
      {
         // setup
         Lua::State state;

         const unsigned char ucValue = 42;

         // run
         {
            Lua::Userdata userdata = state.AddUserdata(4);

            userdata.Data<unsigned char>()[0] = ucValue;

            state.AddValue(_T("user"), Lua::Value(userdata));
         }

         state.LoadSourceString(_T("function test() return user; end"));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::IsTrue(Lua::Value::typeUserdata == vecRetval[0].GetType(), _T("type must be userdata"));

         Lua::Userdata userdata = vecRetval[0].Get<Lua::Userdata>();

         Assert::IsTrue(userdata.Data<unsigned char>()[0] == ucValue, _T("stored value must match"));
      }

      /// tests method State::AddUserdata() and stores it as value in a table
      TEST_METHOD(TestStateAddUserdataStoreInTable)
      {
         // setup
         Lua::State state;

         const unsigned char ucValue = 42;

         // run
         {
            Lua::Userdata userdata = state.AddUserdata(4);

            userdata.Data<unsigned char>()[0] = ucValue;

            state.AddValue(_T("user"), Lua::Value(userdata));
         }

         {
            Lua::Value value = state.GetValue(_T("user"));
            Lua::Userdata userdata = value.Get<Lua::Userdata>();

            state.AddValue(_T("user2"), Lua::Value(userdata));
         }

         state.LoadSourceString(_T("function test() return user,user2; end"));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 2);

         // check
         Assert::AreEqual<size_t>(2, vecRetval.size(), _T("must have returned 2 return values"));

         Assert::IsTrue(Lua::Value::typeUserdata == vecRetval[0].GetType(), _T("type must be userdata"));
         Assert::IsTrue(Lua::Value::typeUserdata == vecRetval[1].GetType(), _T("type must be userdata"));

         Lua::Userdata userdata1 = vecRetval[0].Get<Lua::Userdata>();
         Lua::Userdata userdata2 = vecRetval[1].Get<Lua::Userdata>();

         Assert::IsTrue(userdata1.Data() == userdata2.Data(), _T("pointer of two userdatas must be equal"));
      }

      /// tests method State::AddUserdata() and stores it as value in a table
      TEST_METHOD(TestStateUserdataCopy)
      {
         // setup
         Lua::State state;

         const unsigned char ucValue = 42;

         // run
         {
            Lua::Table table = state.AddTable(_T("abc"));

            Lua::Userdata userdata = state.AddUserdata(4);

            userdata.Data<unsigned char>()[0] = ucValue;

            table.AddValue(_T("user"), Lua::Value(userdata));
         }

         // check
         // must not crash
      }

      TEST_METHOD(TestStateAddFunction)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fn = [&] (Lua::State&, const std::vector<Lua::Value>&) -> std::vector<Lua::Value>
         {
            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(42.0));

            return vecRetValues;
         };

         state.AddFunction(_T("run"), fn);

         // run
         state.LoadSourceString(_T("function test() return run(); end"));

         // check
         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);

         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
      }

      TEST_METHOD(TestStateAddFunctionWithParam)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fn = [&] (Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value");

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(vecParams[0]);

            return vecRetValues;
         };

         state.AddFunction(_T("run"), fn);

         // run
         state.LoadSourceString(_T("function test(val) return run(val); end"));

         std::vector<Lua::Value> vecParam;
         vecParam.push_back(Lua::Value(42.0));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1, vecParam);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
      }

      TEST_METHOD(TestStateAddValue)
      {
         Lua::State state;

         // setup
         state.AddValue(_T("the_value"), Lua::Value(42.0));

         // run
         state.LoadSourceString(_T("function test() return the_value; end"));

         // check
         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);

         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
      }

      /// Tests function call, calling a global Lua function, with a table as parameter
      TEST_METHOD(TestFunctionCall_LuaFunction_ParamTable)
      {
         Lua::State state;

         // setup
         state.LoadSourceString(_T("function test(val) return val * 42.0; end"));

         // run
         for (int i = 0; i < 1000; i++)
         {
            Lua::StackChecker checker(state.GetState());

            Lua::Value valFunc = state.GetValue(_T("test"));
            Lua::Function func = valFunc.Get<Lua::Function>();

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(42.0));

            std::vector<Lua::Value> vecRetval = func.Call(1, vecParam);

            // check
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            Assert::AreEqual(vecRetval[0].Get<double>(), 42.0 * 42.0, 1e-6, _T("value must be 42 * 42"));
         }
      }

      /// Tests function call, calling a Lua function stored in a table, with a table as parameter
      TEST_METHOD(TestFunctionCall_TableFunction_ParamTable)
      {
         Lua::State state;

         // setup
         state.LoadSourceString(_T("abc = { test = function(self, val) return val * 42.0; end; }"));

         // run
         for (int i = 0; i < 1000; i++)
         {
            Lua::StackChecker checker(state.GetState());

            Lua::Table table = state.GetTable(_T("abc"));

            Lua::Value valFunc = table.GetValue(_T("test"));
            Lua::Function func = valFunc.Get<Lua::Function>();

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(table));
            vecParam.push_back(Lua::Value(42.0));

            std::vector<Lua::Value> vecRetval = func.Call(1, vecParam);

            // check
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            Assert::AreEqual(vecRetval[0].Get<double>(), 42.0 * 42.0, 1e-6, _T("value must be 42 * 42"));
         }
      }

      /// Tests function call, calling a C++ bound function stored in a table, with a table as parameter
      TEST_METHOD(TestFunctionCall_CppBinding_ParamTable)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fn = [](Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(vecParams[0].Get<double>() * 42.0));

            return vecRetValues;
         };

         // run
         for (int i = 0; i < 1000; i++)
         {
            Lua::StackChecker checker(state.GetState());

            Lua::Table table = state.AddTable(_T("abc"));
            table.AddFunction("run", fn);

            Lua::Value valFunc = table.GetValue(_T("run"));
            Lua::Function func = valFunc.Get<Lua::Function>();

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(42.0));

            std::vector<Lua::Value> vecRetval = func.Call(1, vecParam);

            // check
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            Assert::AreEqual(vecRetval[0].Get<double>(), 42.0 * 42.0, 1e-6, _T("value must be 42 * 42"));
         }
      }

      /// Tests function call, calling a global Lua function, with a userdata object as parameter
      TEST_METHOD(TestFunctionCall_ParamUserdata)
      {
         Lua::State state;

         // setup
         state.LoadSourceString(_T("function test(val) return val; end"));

         // run
         for (int i = 0; i < 1000; i++)
         {
            Lua::StackChecker checker(state.GetState());

            Lua::Value valFunc = state.GetValue(_T("test"));
            Lua::Function func = valFunc.Get<Lua::Function>();

            Lua::Userdata userdata = state.AddUserdata(sizeof(int));
            *userdata.Data<int>() = 42;

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(userdata));

            std::vector<Lua::Value> vecRetval = func.Call(1, vecParam);

            // check
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            Assert::IsTrue(vecRetval[0].GetType() == Lua::Value::typeUserdata, _T("value must be of userdata type"));
         }
      }

      /// Tests function call, calling a global Lua function, with a function object as parameter
      TEST_METHOD(TestFunctionCall_ParamFunction)
      {
         Lua::State state;

         // setup
         state.LoadSourceString(_T("function test(func, val) return func(val) + 2 * func(val/2); end"));
         state.LoadSourceString(_T("function calc(val) return val * val; end"));

         // run
         for (int i = 0; i < 1000; i++)
         {
            Lua::StackChecker checker(state.GetState());

            Lua::Value valFuncTest = state.GetValue(_T("test"));
            Lua::Function funcTest = valFuncTest.Get<Lua::Function>();

            Lua::Value valFuncCalc = state.GetValue(_T("calc"));
            Lua::Function funcCalc = valFuncCalc.Get<Lua::Function>();

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(funcCalc));
            vecParam.push_back(Lua::Value(42.0));

            std::vector<Lua::Value> vecRetval = funcTest.Call(1, vecParam);

            // check
            Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

            Assert::IsTrue(vecRetval[0].GetType() == Lua::Value::typeNumber, _T("value must be a number"));
            Assert::AreEqual(vecRetval[0].Get<double>(),
               (42.0 * 42.0) + 2.0 * ((42.0/2.0) * (42.0 / 2.0)),
               1e-6, _T("value must be equal to calculated value"));
         }
      }

      TEST_METHOD(TestFunctionPushTable)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fn = [] (Lua::State& paramState, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value");

            Lua::Table table = paramState.AddTable(_T(""));
            table.AddValue(_T("def"), vecParams[0]);

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(table));

            return vecRetValues;
         };

         state.AddFunction(_T("run"), fn);

         // run
         state.LoadSourceString(_T("function test(val) local abc = run(val); return abc.def; end"));

         std::vector<Lua::Value> vecParam;
         vecParam.push_back(Lua::Value(42.0));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1, vecParam);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
      }

      TEST_METHOD(TestFunctionParamInClosure)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fn = [&] (Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeFunction != vecParams[0].GetType())
               throw std::runtime_error("must pass a function value");

            // call function
            Lua::Function func = vecParams[0].Get<Lua::Function>();

            std::vector<Lua::Value> vecCallParams;
            vecCallParams.push_back(Lua::Value(42.0));
            vecCallParams.push_back(Lua::Value(64.0));

            return func.Call(1, vecCallParams); // tail call
         };

         state.AddFunction(_T("run"), fn);

         // run
         state.LoadSourceString(_T("function calc(a, b) return a+b; end"));
         state.LoadSourceString(_T("function test() return run(calc); end"));

         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("test"), 1);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0+64.0, 1e-6, _T("value must be 42+64"));
      }

      TEST_METHOD(TestFunctionPushOnStack)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fnAsyncWait = [](Lua::State& paramState, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeFunction != vecParams[0].GetType())
               throw std::runtime_error("must pass a function value in param 0");

            Lua::Function func = vecParams[0].Get<Lua::Function>();

            // add function as value
            paramState.AddValue(_T("_the_handler"), vecParams[0]);

            return std::vector<Lua::Value>();
         };

         Lua::T_fnCFunction fnHandler = [](Lua::State&, const std::vector<Lua::Value>& vecParams)-> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value in param 0");

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(42.0));

            return vecRetValues;
         };

         state.AddFunction(_T("async_wait"), fnAsyncWait);
         state.AddFunction(_T("handler"), fnHandler);

         state.LoadSourceString(_T("function wait_completed() return _the_handler(64.0); end"));
         state.LoadSourceString(_T("function test() async_wait(handler); end"));

         // run
         state.CallFunction(_T("test"));
         std::vector<Lua::Value> vecRetval = state.CallFunction(_T("wait_completed"), 1);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42.0"));
      }

      TEST_METHOD(TestFunctionPushOnTable)
      {
         Lua::State state;

         // setup

         // define a function to call back
         Lua::T_fnCFunction fnAsyncWait = [](Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 2)
               throw std::runtime_error("must pass exactly 2 values");

            if (Lua::Value::typeTable != vecParams[0].GetType())
               throw std::runtime_error("must pass a table value in param 0");

            if (Lua::Value::typeFunction != vecParams[1].GetType())
               throw std::runtime_error("must pass a function value in param 1");

            Lua::Table self = vecParams[0].Get<Lua::Table>();
            Lua::Function func = vecParams[1].Get<Lua::Function>();

            // add function as value
            self.AddValue(_T("_the_handler"), vecParams[1]);

            return std::vector<Lua::Value>();
         };

         Lua::T_fnCFunction fnHandler = [](Lua::State&, const std::vector<Lua::Value>& vecParams)-> std::vector<Lua::Value>
         {
            if (vecParams.size() != 2)
               throw std::runtime_error("must pass exactly 2 values");

            if (Lua::Value::typeTable != vecParams[0].GetType())
               throw std::runtime_error("must pass a table value in param 0");

            if (Lua::Value::typeNumber != vecParams[1].GetType())
               throw std::runtime_error("must pass a number value in param 1");

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(42.0));

            return vecRetValues;
         };

         state.LoadSourceString(_T("App = { ")
            _T("wait_completed = function() return App:_the_handler(64.0); end, ")
            _T("test = function() App:async_wait(App.handler); end ")
            _T("}"));

         Lua::Table table = state.GetTable(_T("App"));
         table.AddFunction("async_wait", fnAsyncWait);
         table.AddFunction("handler", fnHandler);

         // run
         table.CallFunction(_T("test"));
         std::vector<Lua::Value> vecRetval = table.CallFunction(_T("wait_completed"), 1);

         // check
         Assert::AreEqual<size_t>(1, vecRetval.size(), _T("must have returned 1 return value"));

         Assert::AreEqual(vecRetval[0].Get<double>(), 42.0, 1e-6, _T("value must be 42.0"));
      }

      TEST_METHOD(TestThreadEmptyCtor)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         // run

         // check
         Assert::IsTrue(thread.Status() == Lua::Thread::statusOK, _T("non-started thread must have status statusOK"));
      }

      TEST_METHOD(TestThreadCallFunc)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         state.LoadSourceString(_T("function run() return; end"));

         // run
         Lua::Function func = thread.GetValue(_T("run")).Get<Lua::Function>();

         std::vector<Lua::Value> vecParam;
         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal = thread.Start(func, vecParam);

         // check
         Assert::IsTrue(retVal.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));
         Assert::IsTrue(retVal.second.empty(), _T("must have returned no return values"));
      }

      TEST_METHOD(TestThreadCallFuncReturnValue)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         state.LoadSourceString(_T("function run() return 42, 42==6*7, \"123abc\", nil; end"));

         // run
         Lua::Function func = thread.GetValue(_T("run")).Get<Lua::Function>();

         std::vector<Lua::Value> vecParam;
         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal = thread.Start(func, vecParam);

         // check
         Assert::IsTrue(retVal.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

         std::vector<Lua::Value>& vecRetvals = retVal.second;

         Assert::AreEqual<size_t>(4, vecRetvals.size(), _T("must have returned 4 return values"));

         Assert::IsTrue(Lua::Value::typeNumber == vecRetvals[0].GetType(), _T("type must be number"));
         Assert::AreEqual(vecRetvals[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));

         Assert::IsTrue(Lua::Value::typeBoolean == vecRetvals[1].GetType(), _T("type must be boolean"));
         Assert::IsTrue(Lua::Value::typeString == vecRetvals[2].GetType(), _T("type must be string"));
         Assert::IsTrue(Lua::Value::typeNil == vecRetvals[3].GetType(), _T("type must be nil"));
      }

      TEST_METHOD(TestThreadCallFunctionInTable)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         state.LoadSourceString(_T("abc = { def = function(self, val) return val, val+val, val*val end }"));

         // run
         Lua::Table table = thread.GetTable(_T("abc"));

         Lua::Function func = table.GetValue(_T("def")).Get<Lua::Function>();

         std::vector<Lua::Value> vecParam;
         vecParam.push_back(Lua::Value(table));
         vecParam.push_back(Lua::Value(42.0));

         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal = thread.Start(func, vecParam);

         // check
         Assert::IsTrue(retVal.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

         std::vector<Lua::Value>& vecRetvals = retVal.second;

         Assert::AreEqual<size_t>(3, vecRetvals.size(), _T("must have returned 3 return values"));

         Assert::AreEqual(vecRetvals[0].Get<double>(), 42.0, 1e-6, _T("value must be 42"));
         Assert::AreEqual(vecRetvals[1].Get<double>(), 42.0 + 42.0, 1e-6, _T("value must be 42+42"));
         Assert::AreEqual(vecRetvals[2].Get<double>(), 42.0*42.0, 1e-6, _T("value must be 42*42"));
      }

      TEST_METHOD(TestThreadCallFunctionClosure)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         // define a function to call back
         Lua::T_fnCFunction fn = [](Lua::State&, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must pass exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value");

            std::vector<Lua::Value> vecRetValues;
            vecRetValues.push_back(Lua::Value(vecParams[0].Get<double>() * 42.0));

            return vecRetValues;
         };

         state.AddFunction(_T("run"), fn);

         // run
         Lua::Function func = thread.GetValue(_T("run")).Get<Lua::Function>();

         std::vector<Lua::Value> vecParam1;
         vecParam1.push_back(Lua::Value(3.0));
         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal = thread.Start(func, vecParam1);

         // check
         Assert::IsTrue(retVal.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

         std::vector<Lua::Value>& vecRetvals = retVal.second;

         Assert::AreEqual<size_t>(1, vecRetvals.size(), _T("must have returned 1 return values"));

         Assert::IsTrue(Lua::Value::typeNumber == vecRetvals[0].GetType(), _T("type must be number"));
         Assert::AreEqual(3.0 * 42.0, vecRetvals[0].Get<double>(), 1e-6, _T("value must match calculated value"));
      }

      TEST_METHOD(TestThreadStartYieldResume)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         state.RequireLib("coroutine");
         state.LoadSourceString(_T("function run(value) return coroutine.yield(value * 42, 64); end"));

         // run 1
         Lua::Function func = thread.GetValue(_T("run")).Get<Lua::Function>();

         std::vector<Lua::Value> vecParam1;
         vecParam1.push_back(Lua::Value(3.0));
         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal1 = thread.Start(func, vecParam1);

         // check 1
         Assert::IsTrue(retVal1.first == Lua::Thread::statusYield, _T("status of finished thread must be statusYield"));

         std::vector<Lua::Value>& vecRetvals1 = retVal1.second;
         Assert::AreEqual<size_t>(2, vecRetvals1.size(), _T("must have returned 2 return values"));

         double dValue2 = vecRetvals1[0].Get<double>() - vecRetvals1[1].Get<double>();

         // run 2
         std::vector<Lua::Value> vecParam2;
         vecParam2.push_back(Lua::Value(dValue2));

         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal2 = thread.Resume(vecParam2);

         // check 2
         Assert::IsTrue(retVal2.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

         std::vector<Lua::Value>& vecRetvals2 = retVal2.second;

         Assert::AreEqual<size_t>(1, vecRetvals2.size(), _T("must have returned 1 return values"));

         Assert::IsTrue(Lua::Value::typeNumber == vecRetvals2[0].GetType(), _T("type must be number"));
         Assert::AreEqual(3.0 * 42.0 - 64.0, 1e-6, vecRetvals2[0].Get<double>(), _T("value must match calculated value"));
      }

      TEST_METHOD(TestThreadYieldInClosure)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         // define a function to call back
         Lua::T_fnCFunction fn = [&thread](Lua::State& paramState, const std::vector<Lua::Value>& vecParams) -> std::vector<Lua::Value>
         {
            if (vecParams.size() != 1)
               throw std::runtime_error("must have passed exactly 1 value");

            if (Lua::Value::typeNumber != vecParams[0].GetType())
               throw std::runtime_error("must pass a number value");

            // yield to caller
            std::vector<Lua::Value> vecYieldParams;
            vecYieldParams.push_back(Lua::Value(vecParams[0].Get<double>() * 42.0));
            vecYieldParams.push_back(Lua::Value(64.0));

            auto continuation = [&thread](Lua::State&, const std::vector<Lua::Value>& vecParams2) -> std::vector<Lua::Value>
            {
               if (vecParams2.size() != 1)
                  throw std::runtime_error("must have passed exactly 1 value");

               std::vector<Lua::Value> vecRetValues;
               vecRetValues.push_back(Lua::Value(vecParams2[0].Get<double>() - 5.0));

               return vecRetValues;
            };

            thread.Yield(paramState, vecYieldParams, continuation);
         };

         state.AddFunction(_T("run"), fn);

         // run 1
         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal1;
         {
            Lua::Function func = thread.GetValue(_T("run")).Get<Lua::Function>();

            std::vector<Lua::Value> vecParam1;
            vecParam1.push_back(Lua::Value(3.0));
            retVal1 = thread.Start(func, vecParam1);
         }

         // check 1
         Assert::IsTrue(retVal1.first == Lua::Thread::statusYield, _T("status of finished thread must be statusYield"));

         std::vector<Lua::Value>& vecRetvals1 = retVal1.second;
         Assert::AreEqual<size_t>(2, vecRetvals1.size(), _T("must have returned 2 return values"));

         double dValue2 = vecRetvals1[0].Get<double>() - vecRetvals1[1].Get<double>();

         // run 2
         std::vector<Lua::Value> vecParam2;
         vecParam2.push_back(Lua::Value(dValue2));

         std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal2 = thread.Resume(vecParam2);

         // check 2
         Assert::IsTrue(retVal2.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

         std::vector<Lua::Value>& vecRetvals2 = retVal2.second;

         Assert::AreEqual<size_t>(1, vecRetvals2.size(), _T("must have returned 1 return values"));

         Assert::IsTrue(Lua::Value::typeNumber == vecRetvals2[0].GetType(), _T("type must be number"));
         Assert::AreEqual(3.0 * 42.0 - 64.0 - 5.0, 1e-6, vecRetvals2[0].Get<double>(), _T("value must match calculated value"));
      }

      /// tests if stack is cleaned up when calling thread methods
      TEST_METHOD(TestThreadCleanupStackMultipleRuns)
      {
         // setup
         Lua::State state;
         Lua::Thread thread(state);

         state.LoadSourceString(_T("abc = { def = function(param1) return param1 + 1; end }"));

         // run
         for (int i = 0; i < 100; i++)
         {
            Lua::StackChecker checker1(state.GetState());
            Lua::StackChecker checker2(thread.GetThreadState());

            Lua::Table table = thread.GetTable(_T("abc"));

            Lua::Function func = table.GetValue(_T("def")).Get<Lua::Function>();

            std::vector<Lua::Value> vecParam;
            vecParam.push_back(Lua::Value(42.0));

            std::pair<Lua::Thread::T_enThreadStatus, std::vector<Lua::Value>> retVal = thread.Start(func, vecParam);

            // check
            Assert::IsTrue(retVal.first == Lua::Thread::statusOK, _T("status of finished thread must be statusOK"));

            std::vector<Lua::Value>& vecRetvals = retVal.second;

            Assert::AreEqual<size_t>(1, vecRetvals.size(), _T("must have returned 1 return values"));

            // stack checker must not report an error
         }
      }
   };
}
