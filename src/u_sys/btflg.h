/*
* u_sys is © 2026, Anton Granitov (onen-touw), BSTU Voenmeh
*
* u_sys is published and distributed under 
* the Academic Software License v1.0 (ASL).
*
* u_sys is distributed in the hope that it will be useful 
* for non-commercial academic research, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the ASL for more details.
*
* You should have received a copy of the ASL along with this program; 
* if not, write to anton.granitov123@gmail.com or https://github.com/onen-touw.  
* It is also published at LICENSE.md in root folder of this repository.
*
* You may contact the original licensor at anton.granitov123@gmail.com or https://github.com/onen-touw.
*/

#pragma once

#include <type_traits>
#include <stdint.h>
#include <bits/move.h>
#include "mutex.h"
namespace ufo
{
    template<typename Ty, typename lTy = void, typename lTy2 = void>
    class bit_flag_t;

    template <typename Ty>
    class bit_flag_t<Ty>
    {
    private:
        Ty _flags = 0;

    public:
        bit_flag_t() {
            static_assert(std::is_integral_v<Ty>);
        }
        bit_flag_t(Ty flags) : _flags(flags) {}

        template<typename... Poses>
        bit_flag_t(Poses... poses)
        {
            set(poses...);
        }

        bit_flag_t(const bit_flag_t& oth) : _flags(oth._flags) {}
        bit_flag_t& operator= (const bit_flag_t& oth) {
            if (&oth != this)
            {
                _flags = oth._flags;
            }
            return *this;            
        }
        
        size_t size() const 
        {
            return sizeof(Ty) * 8;
        }

        bool operator== (const bit_flag_t<Ty>& o) const {
            return _flags == o._flags;
        }

        bit_flag_t(bit_flag_t&& oth) : _flags(oth._flags) {
            oth._flags = 0;
        }
        bit_flag_t& operator= (bit_flag_t&& oth) {
            if (&oth != this)
            {
                _flags = oth._flags;
                oth._flags = 0;
            }
            return *this;    
        }

        template<typename Pos, typename... Poses>
        void set(Pos pos, Poses... lst)
        {
            set(pos);
            set(lst...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void set(enum_Ty pos)
        {
            set(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void set(int_t pos)
        {
            if (pos < sizeof(Ty) * 8)
            {
                _flags |= (1 << pos);
            }
        }

        template <typename Pos, typename... Poses>
        void unset(Pos pos, Poses... lst)
        {
            unset(pos);
            unset(lst...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void unset(enum_Ty pos)
        {
            unset(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void unset(int_t pos)
        {
            if (get(pos))
            {
                togle(pos);
            }
        }

        template <typename Pos, typename... Poses>
        void togle(Pos pos, Poses... lst)
        {
            togle(pos);
            togle(lst...);
        }

        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void togle(enum_Ty pos)
        {
            togle(static_cast<uint8_t>(pos));
        }

        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void togle(int_t pos)
        {
            if (pos < sizeof(Ty) * 8)
            {
                _flags ^= (1 << pos);
            }
        }

        template <typename Pos, typename... Poses>
        bool get(Pos pos, Poses... poses) const
        {
            return get(pos) && get(poses...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        bool get(enum_Ty pos) const
        {
            return get(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        bool get(int_t pos) const
        {
            if (pos < sizeof(Ty) * 8)
            {
                return static_cast<bool>((_flags >> pos) & 0b1);
            }
            return false;
        }

        void rst() { _flags = 0; }
        Ty get() const { return _flags; }

        void upd(Ty val) { _flags = val; }
    };
} // namespace ufo
