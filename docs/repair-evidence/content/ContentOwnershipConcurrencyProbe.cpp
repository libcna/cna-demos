// SPDX-License-Identifier: MS-PL
// Copy distinct wrappers from a stable recorded source. No ContentManager call,
// source mutation, shared-wrapper destruction or owning disposal overlaps workers.
#include <array>
#include <atomic>
#include <barrier>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include "CNA/Internal/ContentOwnership.hpp"

namespace
{
    struct ValueResource final : System::IDisposable
    {
        std::shared_ptr<int> resource = std::make_shared<int>(42);
        bool disposed = false;
        CNA::Internal::ContentOwnershipLink ownership{this};

        ValueResource() = default;
        ValueResource(const ValueResource& other)
            : resource(other.resource), ownership(this, other.ownership) { }
        ValueResource(ValueResource&& other) noexcept
            : resource(std::move(other.resource)), disposed(other.disposed),
              ownership(this, std::move(other.ownership)) { }

        void Dispose() override
        {
            if (ownership.DisposeRepresentative() || disposed) return;
            resource.reset();
            disposed = true;
            ownership.MarkDisposed();
        }
    };
}

int main()
{
    ValueResource original;
    const std::weak_ptr<int> native = original.resource;
    auto representative = std::make_shared<ValueResource>(original);
    auto record = original.ownership.Capture(representative, representative->ownership);
    representative.reset();
    constexpr std::size_t workerCount = 4;
    std::barrier start(static_cast<std::ptrdiff_t>(workerCount));
    std::atomic<int> failures{0};
    std::array<std::thread, workerCount> workers;
    for (auto& worker : workers)
    {
        worker = std::thread([&]
        {
            start.arrive_and_wait();
            for (int i = 0; i < 1000; ++i)
            {
                ValueResource copied(original);
                ValueResource moved(std::move(copied));
                if (!moved.resource || *moved.resource != 42 || moved.disposed)
                    ++failures;
            }
        });
    }
    for (auto& worker : workers) worker.join();
    if (original.disposed || native.expired()) ++failures;
    record->Dispose();
    if (!original.disposed || !native.expired()) ++failures;
    std::cout << "copies=4000 failed=" << failures.load() << '\n';
    return failures.load() == 0 ? 0 : 1;
}
