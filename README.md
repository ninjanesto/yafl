![](doc/logo/yafl.png "Yet Another Functional Library")

# Yet Another Functional Library
Functional Programming concepts implemented in C++17

# Table of Contents
1. [Introduction](#introduction)
2. [Usage](#usage)
3. [Build](#build)

## Introduction
C++ is a multi paradigm programming language and functional programming concepts keep getting added to the C++ standard.

Yafl is a header only library that implements some key FP concepts, such as curring / uncurring, partial application, function composition, kleisli arrows, and also the Maybe and Either monads (Functor, Applicative Functor and Monad).

This library helps you in reducing code noise and boilerplate code. It can improve readability (if you are accustomed to FP) and it is really fun to use and learn functional concepts.

Most of the concepts present in this library were inspired in some Haskell features, which is a really cool functional programming language

## Examples

### High Order Functions (HOF)
Higher order functions are functions that take one or more functions as arguments, and/or return a function as their result.

#### Function Composition
Function composition is a mechanism that combines simple functions to build more complicated functions. 
Is an operation that takes two functions as arguments and returns a new function where th result of each function is passed as the argument of the next, and the result of the last one is the result of the whole.

```c++
struct Xpto { std::string value; };

const auto f1 =[](int i) { return i*42;};
const auto f2 =[](int i) { return std::to_string(i);};
const auto f3 =[](const std::string& s) { return Xpto{ s };};

const auto comp_f1_f2 = yafl::compose(f1, f2);
//Here we need to explicitly use the correct type
const auto threeway = yafl::compose<std::function<std::string(int)>>(comp_f1_f2, f3);
std::cout << threeway(1).value << std::endl;
```

In mathematics, function composition is associative. 

f . (g . h) = (f . g) . h

The above example is implemented following left associativity.
We can implement the above example expressing right associativity. Note that in this example there's no need to explicitly declare the function type
```c++
struct Xpto { std::string value; };

const auto f1 =[](int i) { return i*42;};
const auto f2 =[](int i) { return std::to_string(i);};
const auto f3 =[](const std::string& s) { return Xpto{ s };};

const auto comp_f2_f3 = yafl::compose(f2, f3);
const auto threeway = yafl::compose(f1, comp_f2_f3);
std::cout << threeway(1).value << std::endl;
```

#### Partial Function Application
Partial application (or partial function application) refers to the process of fixing a number of arguments to a function, producing another function of smaller arity (less input arguments).

```c++
struct Xpto { std::string value;};

const auto f = [](int i, int j, float f, const std::string& s, const Xpto& x) {return 42;};

const auto partial_3arg_app = yafl::partial(f, 1, 2, 3.14);
const auto result = partial_3arg_app("s", Xpto{""});
std::cout << result << std::endl;

const auto partial_2arg_app = yafl::partial(f, 1, 2);
// Here we need to explicitly declare the correct type
const auto partial_4arg_app = yafl::partial<std::function<int(float f, const std::string&, const Xpto&)>>(partial_2arg_app, 3.14, "");
const auto result2 = partial_4arg_app(Xpto{""});
std::cout << result2 << std::endl;
```

#### Currying / Uncurrying

Currying consists in the transformation of a function that takes multiple arguments into a sequence of functions, each takes a single argument.

Uncurring is the reverse process. It takes a function that takes one argument and whose return value is another function and yields a new function that takes two arguments (one from each function) and returns as result the application of the first function with using the first argument and using its result with the second argument.

Note: we only referred two arguments , but we support multiple argument functions as shown in the following examples

```c++
int function1(int arg1, float arg2, const std::string& arg3) { return 42;}

const auto curried_function1 = yafl::curry(function1);
const auto curried_function1_with_arg1_and_arg2 = curried_function1(42)(3.14);
const auto result = curried_function1_with_arg1_and_arg2("dummy");

const auto uncurried_function1 = yafl::uncurry(curried_function1);
const auto result2 = uncurried_function1(42, 3.14, "dummy");
```

The same behaviour can be achieved using lambda functions. The next example uses a lambda function that doesn't return anything.

```c++
const auto curried_function1 = yafl::curry([](int, int, int ,int){});
const auto curried_function1_with_arg1 = curried_function1(1);
curried_function1_with_arg1(2)(3)(4);

const auto uncurried_function1 = yafl::uncurry(curried_function1);
uncurried_function1(1, 2, 3, 4);
```

#### Identity and Const functions
The identity function always returns the value that was used as its argument, unchanged.
The const function returns a function that may receive 0 or more arguments but will always evaluate to the value configured when it was invoked
```c++
//v is 42
const auto v = yafl::id(42);

const auto ff = yafl::constf(42);
// all the following invocations will evaluate to 42
ff();
ff("");
ff(1,"",3);
```

## Functor, Applicative Functor and Monad
The following *Functor*, *Applicative Functor* and *Monad* classes are part of YAFL core and are not meant to be used as is but if needed, it is possible to do so.
Each description contains a brief example of a possible usage.

We provide two implementations for all these functional "interfaces", the [Maybe](###maybe) and [Either](###either) classes

#### Functor
In mathematics a functor is a mapping between categories (collection of objects linked by morphisms).

We address Functor as a wrapper class for any value type, that allows a transformation to the underlying values, by relying on the application of a given function.
It is represented in YAFL by the abstract class yafl::Functor, which provides the method `fmap` to apply the given transformation to the wrapped value.

```c++
const std::unique_ptr<yafl::core::Functor<yafl::Maybe, int>> functor =
        std::make_unique<yafl::Maybe<int>>(yafl::maybe::Just(420));
functor->fmap([](int i){ std::cout << i << std::endl;});
```
Functors are required to obey certain laws.
 - Functors must preserve identity morphisms
    
    *fmap id \<functor\> = id \<functor\>*
```c++
const auto m = yafl::maybe::Just(420);

const auto r = yafl::fmap(yafl::id<int>, m);
const auto r2 = yafl::id(m);
std::cout << (r.value() == r2.value()) << std::endl;
```

 - Functors preserve composition of morphisms 
    
    *fmap (f . g) <functor> ==  fmap f . fmap g*
```c++
const auto f1 = [](int i) { return std::to_string(i);};
const auto f2 = [](const std::string& s) { return s + "dummy";};

const auto compose = yafl::compose(f1, f2);
std::cout << (m.fmap(compose).value() == m.fmap(f1).fmap(f2).value()) << std::endl;
```

#### Applicative Functor
Applicative functors are functors on steroids (with extra laws and operations). They are an intermediate class between Functor and Monad.
They allow function application to be chained across multiple instances of the structure.

Functor _fmap_ applies a function to a value inside a functor:

 *fmap :: (a -> b) -> f a -> f b*

Applicative _apply_ applies a function inside a functor to a value also inside the functor

 *apply :: f (a -> b) -> f a -> f b*

We implemented the apply operation using C++ operator(). Our implementation of applicative functors can receive a function with any number of arguments, that can later be partially applied.

Applicatives are required to satisfy four laws:
 - Identity
```c++
const auto v = yafl::maybe::Just(42);
const auto ap = yafl::maybe::Just(&yafl::id<int>);
std::cout << (ap(yafl::Maybe(v)).value() == v.value()) << std::endl;
std::cout << (ap(yafl::maybe::Just(42)).value() == v.value()) << std::endl;

const auto ap2 = yafl::maybe::Just(&yafl::id<int>);
std::cout << (ap2(42).value() == v.value()) << std::endl;
```

 - Composition
```c++

```



#### Monad




### Maybe

### Either 

### Function lift

## Build