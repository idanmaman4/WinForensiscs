#pragma once

#include "DebugMagic.h"
#include "GenericTypeContainer.h"
#include <functional>
#include <memory>

namespace ProcessUtils {

// Callback receives the _EPROCESS container for each live process.
// Return false to stop the walk early.
using EprocessCallback = std::function<bool(std::shared_ptr<GenericTypeContainer> eprocess)>;

inline void for_each_process(DebugMagic& dbg, EprocessCallback cb) {
    auto process_head = dbg.get_symbol_address_as<"nt", "_LIST_ENTRY">("nt", "PsActiveProcessHead");
    if (!process_head.has_value()) return;

    Address current_link = process_head.value()->get<Address>("Flink").value();
    Address head         = process_head.value()->address();

    do {
        auto eprocess = dbg.get_struct_from_field_as<"nt", "_EPROCESS", "ActiveProcessLinks">(current_link);
        if (!eprocess.has_value()) break;

        auto link = eprocess.value()->as_object("ActiveProcessLinks")
                        ->get()->as_pointer("Flink");
        if (!link.has_value()) break;
        Address next_link = link.value();

        if (!cb(eprocess.value())) break;

        current_link = next_link;
    } while (current_link != 0 && current_link != head);
}

} // namespace ProcessUtils
