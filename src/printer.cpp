#include "printer.hpp"

#include <iostream>

namespace NParallelGrep::NPrinter {
    TPrinter::TPrinter(
        NLib::NThreadPool::TBlockingQueue<NParallelGrep::NResult::TResult>& out,
        NParallelGrep::NCli::TCliOptions& opts
    )
        : CountOnlyFlg_(opts.CountOnly)
        , Worker_([&]{
            NResult::TResult result;
            while (out.Pop(result)) {
                if (CountOnlyFlg_) {
                    Counter_++;
                } else {
                    std::cout << result.FileName << ":" << result.LineNumber << ":" << result.Line << '\n';
                }
            }
        })
    {
    }

    void TPrinter::Close() {
        if (CountOnlyFlg_) {
            std::cout << "Count: " << Counter_ << '\n';
        }
        Worker_.join();
    }
}
