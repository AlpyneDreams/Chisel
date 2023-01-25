#pragma once

#include <type_traits>

namespace chisel
{
    namespace detail {
        template <template <typename...> class C, typename... Ts>
        std::true_type derived_from_template_impl(const C<Ts...>*);

        template <template <typename...> class C>
        std::false_type derived_from_template_impl(...);
    }

    // Checks if Derived publicly extends any specialization of Base<...>
    template <typename Derived, template <typename...> class Base>
    concept derived_from_template = std::is_class_v<Derived> && decltype(detail::derived_from_template_impl<Base>(std::declval<Derived*>()))::value;
}