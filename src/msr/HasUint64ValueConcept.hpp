//
// Created by pierre on 2025-06-01.
//

#ifndef HASUINT64VALUECONCEPT_H
#define HASUINT64VALUECONCEPT_H


#include <concepts>
#include <cstdint>
template<typename T>
concept HasUint64Value = requires(T t) {
    { t.value } -> std::same_as<uint64_t&>;
};

#endif //HASUINT64VALUECONCEPT_H
