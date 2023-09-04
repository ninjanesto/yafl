//#include <iostream>
//#include "Maybe.h"
//#include "Either.h"
//#include "Compose.h"
//
////Retry monad
//
////Continuation monad
//
////Try monad
//
//int main(int argc, char** argv) {
//
//    Maybe<> voidmaybe = Just<>().fmap([](){});
//    Maybe<> voidmaybe2 = Just(42).fmap([](const auto& x){std::cout << x << "void lambda\n";});
//    Maybe<> voidmaybe3 = Nothing<>().fmap([](){});
//    Maybe<> voidmaybe4 = Nothing<int>().fmap([](const auto& x){std::cout << x << "void lambda\n";});
//
//    std::cout << voidmaybe << std::endl;
//    std::cout << voidmaybe2 << std::endl;
//    std::cout << voidmaybe3 << std::endl;
//    std::cout << voidmaybe4 << std::endl;
//
//    Maybe<int> valuemaybe = Just(42).fmap([](const auto&){return 1;});
//    Maybe<int> valuemaybe2 = Nothing<int>().fmap([](const auto&){return 1;});
//
//    std::cout << valuemaybe << std::endl;
//    std::cout << valuemaybe2 << std::endl;
//
//    auto flambda = [](int) {return 3.4f;};
//    auto dlambda = [](double) {return std::string("bammm");};
//    auto slambda = [](const std::string& s) {std::cout << s << std::endl;};
//    auto gggg = fmap(flambda, fmap(flambda, Maybe<int>::Just(2)));
//    auto hhhh = fmap(flambda | flambda,  Maybe<int>::Just(2));
//
//    //apply::A(int -> int) -> A (int) -> A (int)
//    auto lambdaJust = Just([](const int& x){ return x*42;});
//    auto lambdaJustRes = lambdaJust(Maybe<int>::Just(2));
//    std::cout << "lambdaJustRes: " << lambdaJustRes << "\n";
//    std::cout << "lambdaJustRes: " << lambdaJust(Maybe<int>::Just(2)) << "\n";
//    std::cout << "lambdaJustRes: " << lambdaJust(Nothing<int>()) << "\n";
//
//    //apply::A(() -> int) -> A () -> A (int)
//    auto lambdaJust3 = Just([](){ return 42;});
//    auto lambdaJustRes3 = lambdaJust3();
//    std::cout << "lambdaJustRes3: " << lambdaJustRes3 << "\n";
//
//    //apply::A(a -> ())) -> A (a) -> A ()
//    auto lambdaJust2 = Just([](int x){ std::cout << x << std::endl;});
//    auto lambdaJustRes2 = lambdaJust2(Nothing<int>());
//    std::cout << "lambdaJustRes2: " << lambdaJustRes2 << "\n";
//    std::cout << "lambdaJustRes2: " << lambdaJust2(Just(2)) << "\n";
//
//    //apply::A(int, float -> int) -> A (int) -> A (float -> int)
//    auto lambdaJust4 = Just([](int x, float f, const std::string& s){ return std::to_string(x*f*42) + s;});
//    auto lambdaJustRes4 = lambdaJust4(Just(2))(Nothing<float>())(Just(std::string("Yes")));
//    std::cout << "lambdaJustRes4: " << lambdaJustRes4 << "\n";
//    std::cout << "lambdaJustRes4: " << lambdaJust4(Just(2), Just(2.0))(Just(std::string("Yes"))) << "\n";
//    std::cout << "lambdaJustRes4: " << lambdaJust4(Just(2))(Nothing<int>(), Just(std::string("Yes"))) << "\n";
//
//    auto lambdaJust5 = Nothing<std::function<void(int)>>();
//    auto lambdaJust5Res = lambdaJust5(Just(2));
//    std::cout << "lambdaJust5Res: " << lambdaJust5Res << "\n";
//    std::cout << "lambdaJust5Res: " << lambdaJust5(Nothing<int>()) << "\n";
//
//    auto comp = flambda | dlambda | slambda;
//    auto liftedComp = lift<Maybe<int>>(comp);
//    std::cout << "Lifted " << liftedComp(Maybe<int>::Just(3));
//
//    std::cout << gggg.value() << std::endl;
//    std::cout << hhhh.value() << std::endl;
//
//    const auto nothing = Maybe<int>::Nothing();
//    auto just42 = Maybe<int>::Just(42);
//
//    const auto fp = [](const auto& x){ return x * 0.1;};
//    const auto f = [](const auto& x){ return Nothing<float>();};
//    const auto g = [](const auto& g){ return Just("ToString: " + std::to_string(g));};
//
//    auto nothingstring = (nothing.fmap(fp)).bind(g);
//    auto result = (Just(42).bind(f)).bind(g);
//
//    auto juststring2 = Just(29)
//            .bind(f)
//            .bind([](const auto& x){ std::cout << "XPTO\n"; return Nothing<float>();})
//            .bind([](const auto& f){ std::cout << "XPTO2\n"; return Just("Lambda: " + std::to_string(f));});
//
//    auto juststring3 = Just(29)
//            .bind(f)
//            | [](const auto& x){ std::cout << "XPTO\n"; return Nothing<float>();};
//
//    const std::function<Maybe<std::string>(const char&)> h = [](const auto& c){ return Just(std::to_string(c)+c);};
//
//    std::cout << nothing << "\n"
//              << just42 << "\n"
//              << nothing.valueOr(29) << "\n"
//              << nothingstring.valueOr("34") << "\n"
//              << result << "\n"
//              << "JUSTSTRING" <<juststring2.valueOr("asd") << "\n"
//              << "JUSTSTRING3" <<juststring3.valueOr("asd") << "\n";
//
//    auto x = Just<int>(42).bind([](int) { return Just<>();})
//                          .bind([](){ return Just<float>(3.4);})
//                          .bind([](const auto& v){ return Just<std::string>(std::to_string(v*2));})
//                          .fmap([](const auto& x ){std::cout << "asdqwezxc\n" << x << "asd\n";});
//    std::cout << x << "XXXXX\n";
//    auto fmapxfunct  = lift<Maybe>([](int i){return 42 * i;});
//    std::cout << fmapxfunct(Maybe<int>::Just(2));
//    auto fmapxfunct2 = Maybe<int>::lift([](int i){return std::to_string(42 * i)+"asd";});
//    std::cout << fmapxfunct2(Maybe<int>::Just(2));
//    //-----------------------------------------------------------------------------------------------------------------
//}
//
////
////
////int main(int argc, char** argv) {
////    Either<std::string, int> resultEither = Right<std::string, int>(42);
////    auto res = resultEither.fmap([](const int i){ return i*0.3;});
////    std::cout << res.right() << "\n";
////
////    Either<std::string, int> resultEither2 = Left<std::string, int>("42");
////    auto res2 = resultEither2.fmap([](const int i){ return i*0.3;});
////    std::cout << res2.left() << "\n";
////
////    Either<std::string, int> resultEither3 = Right<int>(42);
////    auto res3 = resultEither3.fmap([](const int i){ return i*0.3;});
////    std::cout << res.right() << "\n";
//////    auto ef = [](int x) { return x*2;};
////    auto eef = [](int x) { return Right<std::string>(std::string("value: ") + std::to_string(x));};
////
////    std::cout << resultEither.fmap(ef).right();
////    std::cout << resultEither.bind(eef).right();
////    std::cout << resultEither.fmap(ef).bind(eef).rightOr("Bammm");
////
////    Either<std::string, int> resultEither2 = Right<std::string, int>(30);
////
////    auto eef2 = [](int x) {
////        return (x < 29)
////               ? Right<std::string, std::string>(std::string("value: ") + std::to_string(x))
////               : Left<std::string, std::string>("Error coiso");
////    };
////    auto eef3 = [](const std::string& s) {
////        return Right<std::string, std::string>(std::string("value: ") + s);
////    };
////
////    std::cout << "is left?" << resultEither2.bind(eef2).bind(eef3).isLeft() << "\n";
//}
//
