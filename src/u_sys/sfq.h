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

#include <stdint.h>
#include <stddef.h>
#include <freertos/queue.h>
#include <freertos/projdefs.h>
#include <esp_log.h>
#include <portmacro.h>

namespace ufo
{

    enum class sig_t : uint16_t
    {
        uSIGINT = 2,
        uSIGUSR = 11,
        uSIGSTOP = 19,

        uDEF_MAX_SIGV = 32,         // MAXIMUM SIG VALUE of default signals [0...32] reserved
    };

    /// @brief 
    /// @tparam SigTy integral type 
    template<typename SigTy = uint16_t>
    class sfq_t
    {
    private:
        QueueHandle_t _queue = nullptr;
        
    public:
        sfq_t() {}

        sfq_t(size_t q_sz)
        {
            _queue = xQueueGenericCreate( q_sz, sizeof(SigTy), queueQUEUE_TYPE_BASE);
            if (!_queue)
            {
                ESP_LOGE("sfq", "Creation error");
                throw; //?
            }
        }

        ~sfq_t() 
        {
            if (_queue)
            {
                vQueueDelete(_queue);
            }
        }

        sfq_t(const sfq_t&) = delete;
        sfq_t(sfq_t&& oth) : _queue(oth._queue)
        {
            oth._queue = nullptr;
        }

        sfq_t& operator =(const sfq_t& oth) = delete;
        sfq_t& operator =(sfq_t&& oth) 
        {
            _queue = oth._queue;
            oth._queue = nullptr;

            return *this;
        }

        bool good() { return _queue; }

        bool snd(const SigTy& sig_val) const
        {
            return xQueueGenericSend(_queue, &sig_val, 0, queueSEND_TO_BACK) == 1;
        }

        bool rcv(SigTy& val, TickType_t tm = 10) const
        {
            if (xQueueReceive(_queue, &val, tm) == 1)
            {
                return true;
            }
            return false;
        }
    };


}