#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace SGF {
    template<typename E>
    concept Enum = std::is_enum_v<E>;

    template<Enum E>
    class Flags
    {
    public:
        using EnumType = E;
        using Underlying = std::underlying_type_t<E>;

        constexpr Flags() {};

        template<std::same_as<E>... Es>
        constexpr Flags(Es... flags)
            : m_Value((Underlying{} | ... | static_cast<Underlying>(flags))) {}

        constexpr explicit Flags(Underlying value)
            : m_Value(value) {}

        constexpr Underlying ToUnderlying() const {
            return m_Value;
        }

        constexpr explicit operator bool() const {
            return m_Value != 0;
        }
        constexpr operator E() const {
            return static_cast<E>(m_Value);
		}
        constexpr operator Underlying() const {
            return m_Value;
		}
        constexpr Flags operator|(E rhs) const {
            return Flags(m_Value | static_cast<Underlying>(rhs));
        }
        constexpr Flags operator|(Flags rhs) const {
            return Flags(m_Value | rhs.m_Value);
        }
        constexpr Flags operator&(E rhs) const {
            return Flags(m_Value & static_cast<Underlying>(rhs));
        }
        constexpr Flags operator&(Flags rhs) const {
            return Flags(m_Value & rhs.m_Value);
        }

        constexpr Flags operator^(Flags rhs) const {
            return Flags(m_Value ^ rhs.m_Value);
        }

        constexpr Flags operator~() const {
            return Flags(~m_Value);
        }

        constexpr Flags operator<<(int shift) const {
            return Flags(m_Value << shift);
        }

        constexpr Flags operator>>(int shift) const {
            return Flags(m_Value >> shift);
        }

        constexpr Flags& operator|=(Flags rhs) {
            m_Value |= rhs.m_Value;
            return *this;
        }

        constexpr Flags& operator&=(Flags rhs) {
            m_Value &= rhs.m_Value;
            return *this;
        }

        constexpr Flags& operator^=(Flags rhs) {
            m_Value ^= rhs.m_Value;
            return *this;
        }

        constexpr Flags& operator<<=(int shift) {
            m_Value <<= shift;
            return *this;
        }

        constexpr Flags& operator>>=(int shift) {
            m_Value >>= shift;
            return *this;
        }

        constexpr bool operator==(Flags rhs) const {
            return m_Value == rhs.m_Value;
        }

        constexpr bool operator==(const Flags& rhs) const {
            return m_Value == rhs.m_Value;
        }

        constexpr bool Has(E flag) const {
            return (m_Value & static_cast<Underlying>(flag)) != 0;
        }

        constexpr bool Has(Flags<E> flags) const {
			return ((*this & flags).ToUnderlying() == flags.ToUnderlying());
        }

        constexpr void Set(E flag) {
            m_Value |= static_cast<Underlying>(flag);
        }

        constexpr void Unset(E flag) {
            m_Value &= ~static_cast<Underlying>(flag);
        }

        constexpr void Toggle(E flag) {
            m_Value ^= static_cast<Underlying>(flag);
        }
    private:
        Underlying m_Value{};
    };

    template<typename E, typename... Es>
    requires (std::is_enum_v<E> && (std::same_as<Es, E> && ...))[[nodiscard]]
    constexpr Flags<E> AsFlags(E first, Es... rest) {
        return { first, rest... };
    }

}