#include "printer.hpp"

#include <iostream>

namespace NParallelGrep::NPrinter {
    TPrinter::TPrinter(
        NLib::NThreadPool::TBlockingQueue<NParallelGrep::NResult::TResult>& out,
        NParallelGrep::NCli::TCliOptions& opts
    )
        : Worker([&]{
            NParallelGrep::NResult::TResult result;
            while (out.Pop(result)) {
                std::cout << result.RelativePath << ":" << result.LineNumber << ":" << result.Line << '\n';
            }
        })
    {
    }
}
