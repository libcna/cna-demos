// SPDX-License-Identifier: MS-PL
#include "CnaDemos/DemoContext.hpp"

#include <streambuf>

#include "CnaDemos/Metadata.hpp"

namespace CnaDemos
{
    namespace
    {
        class NullBuffer final : public std::streambuf
        {
        protected:
            int_type overflow(int_type c) override { return traits_type::not_eof(c); }
        };

        std::ostream& NullStream()
        {
            static NullBuffer buffer;
            static std::ostream stream(&buffer);
            return stream;
        }

        thread_local DemoContext* currentContext = nullptr;
    }

    DemoContext::DemoContext(const DemoEntry& entry, RunOptions options, std::ostream& out)
        : entry_(entry),
          options_(std::move(options)),
          out_(out)
    {
    }

    int DemoContext::FrameLimit() const
    {
        if (options_.frames >= 0)
        {
            return options_.frames;
        }
        if (!options_.automated)
        {
            return 0;
        }
        return entry_.info.frames > 0 ? entry_.info.frames : DefaultAutomatedFrames;
    }

    std::ostream& DemoContext::Out()
    {
        return options_.quiet ? NullStream() : out_;
    }

    bool DemoContext::Check(bool condition, std::string_view expectation)
    {
        ++checks_;
        if (!condition)
        {
            failures_.emplace_back("check failed: " + std::string(expectation));
            // Failures are always shown, even with --quiet.
            out_ << "  CHECK FAILED: " << expectation << '\n';
        }
        return condition;
    }

    bool DemoContext::CheckNear(double actual, double expected, double tolerance, std::string_view expectation)
    {
        const double difference = actual > expected ? actual - expected : expected - actual;
        if (difference <= tolerance)
        {
            return Check(true, expectation);
        }
        return Check(false, std::format("{} (got {}, expected {} +/- {})", expectation, actual, expected, tolerance));
    }

    void DemoContext::Fail(std::string_view reason)
    {
        failures_.emplace_back(reason);
        out_ << "  FAILED: " << reason << '\n';
    }

    void DemoContext::Skip(std::string_view reason)
    {
        throw SkipDemo(std::string(reason));
    }

    DemoContext& DemoContext::Current()
    {
        if (currentContext == nullptr)
        {
            throw std::logic_error(
                "CnaDemos::DemoContext::Current() called outside a demo run "
                "(construct demos through the runner, not directly)");
        }
        return *currentContext;
    }

    CurrentContextScope::CurrentContextScope(DemoContext& context)
        : previous_(currentContext)
    {
        currentContext = &context;
    }

    CurrentContextScope::~CurrentContextScope()
    {
        currentContext = previous_;
    }
}
