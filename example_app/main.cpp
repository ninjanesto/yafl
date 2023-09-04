/**
 * \brief       CAPI Clients Factory
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include <fstream>
#include <iostream>
#include "yafl/HOF.h"
#include "yafl/Either.h"
#include "yafl/Maybe.h"

constexpr auto START_IDX=0;
constexpr auto END_IDX=255;

template<typename T>
using Result = yafl::Either<void, T>;
//using Result = yafl::Maybe<T>;

/**
 * Caesar Cypher encode/decode function. Receives the operation and applies to the entire file
 * @tparam F Type of operation (encode or decode)
 * @param operation Encode or decode operation
 * @param seed seed for the CAesar Cypher
 * @param filename file name
 * @return
 */
template<typename F>
Result<void> encode_decode(F&& operation, int seed, const std::string& filename) {
    std::ifstream input(filename);
    std::ofstream output(std::string("acc_") + filename);

    input.seekg(0, std::ios_base::end);
    const std::streamsize size =  input.tellg();
    input.seekg(0);

    std::streamsize counter{0};
    while (counter < size) {
         output.put(char(operation(input.get(),seed % 255)));
         counter++;
    }

    //return Result<void>::Just();
    return Result<void>::Ok();
}

/**
 * Validates if the given seed is valid
 * @param seed seed to validate
 * @return Just seed or Nothing
 */
Result<int> validate_seed(const std::string& seed) {
    int _seed{0};
    try{
        _seed = std::stoi(seed);
    } catch(const std::runtime_error& ex) {
        //return Result<int>::Nothing();
        return Result<int>::Error();
    }

    if (_seed >= START_IDX && _seed <= END_IDX) {
        //return Result<int>::Just(_seed);
        return Result<int>::Ok(_seed);
    }
    //return Result<int>::Nothing();
    return Result<int>::Error();
}


using OperationType = std::function<Result<void>(int, const std::string&)>;
/**
 * Validates if the given operation is valid
 * @param op operation to validate
 * @return Just function to execute or Nothing
 */
Result<OperationType> validate_operation(const std::string& op) {
    static OperationType encode = yafl::partial(encode_decode<decltype(std::plus<>())>, std::plus<>());
    static OperationType decode = yafl::partial(encode_decode<decltype(std::minus<>())>, std::minus<>());

    static const std::unordered_map<std::string, OperationType> operationMap{
            {"-e", encode},
            {"--encode", encode},
            {"-d", decode},
            {"--decode", decode},
    };

    if (const auto result = operationMap.find(op); result != operationMap.end()) {
        //return Result<OperationType>::Just(result->second);
        return Result<OperationType>::Ok(result->second);
    }
    //return Result<OperationType>::Nothing();
    return Result<OperationType>::Error();
}


int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Error invoking tool\nUsage: acc <-e|-d> <seed> <filename>\n";
        return 1;
    }

    const auto operation = argv[1];
    const auto seedString = argv[2];
    const auto filename = argv[3];

    {
        // Method 1
        const auto result = validate_operation(operation)
                (validate_seed(seedString))
                (filename)
                .bind(yafl::id<Result<void>>);
        std::cout << result.isOk() << std::endl;
    }

    {
        // Method 2
        const auto result = validate_seed(seedString)
                .bind(validate_operation(operation))
                .fmap([&filename](auto&& f){ f(filename);});
        std::cout << result.isOk() << std::endl;
    }

    {
        // Method 3
        const auto result = validate_seed(seedString)
                .bind(validate_operation(operation))
                (filename)
                .bind(yafl::id<Result<void>>);
        std::cout << result.isOk() << std::endl;
    }

    {
        // Method 4
        const auto op = validate_operation(operation).value();
        const auto curried_op = yafl::curry(op);
        const auto caesar_cypher = yafl::compose(validate_seed, curried_op);
        const auto result = caesar_cypher(seedString)(filename);
        std::cout << result.isOk() << std::endl;
    }

    {
        // curry / uncurry
        const auto op = validate_operation(operation).value();
        const auto curried_op = yafl::curry(op);
        const auto result = validate_seed(seedString).fmap(curried_op)(filename);
        std::cout << result.isOk() << std::endl;

        const auto uncurried_op = yafl::uncurry(curried_op);
        const auto validatedSeed = validate_seed(seedString);
        if (validatedSeed.isOk()) {
            const auto result2 = uncurried_op(validatedSeed.value(), filename);
            std::cout << result2.isOk() << std::endl;
        }
    }

    {
        // lift
        const auto validate_filename = [](const std::string& fn) { return fn;};
        const auto liftedMaybe = yafl::maybe::lift(validate_filename);
        const auto fn = liftedMaybe(yafl::maybe::Just<std::string>(filename));

        const auto result = validate_seed(seedString)
                .bind(validate_operation(operation))
                (fn.value());
        std::cout << result.isOk() << std::endl;

        const auto liftedEither = yafl::either::lift(validate_filename);
        const auto fn2 = liftedEither(yafl::either::Ok<void, std::string>(filename));

        const auto result2 = validate_seed(seedString)
                .bind(validate_operation(operation))
                        (fn2.value());
        std::cout << result2.isOk() << std::endl;
    }

    {
        const auto l1 = [](int i){ std::cout << "I"<<i << std::endl;};
        const auto l2 = [&l1]() {return l1;};
        const auto l3 = [](auto&& f) { f(42);};
        const auto f = yafl::compose(l2, l3);
        f();

        const auto r = yafl::Either<int, int>::Ok(4);
        const auto r2 = r.fmap(yafl::constf<int>(2));
        std::cout << r2.value() << std::endl;

        const auto curried =  yafl::curry(yafl::fmap<std::function<int(int)>, yafl::Either<int,int>>);
        const auto f2 = yafl::compose(yafl::constf<int>, curried);
        const auto r32 = f2(24)(r);
        std::cout << r32.value() << std::endl;

        const auto ff = yafl::uncurry(f2);
        const auto r3 = ff(31, r);
        std::cout << r3.value() << std::endl;

        const auto f3 = yafl::compose(yafl::constf<int>, yafl::fmap<std::function<int(int)>>);
        const auto r4 = f3(29)(r);
        std::cout << r4.value() << std::endl;
        const auto f34= yafl::uncurry(f3);
        std::cout << f34(28, r).value() << std::endl;
    }

    return 0;
}
