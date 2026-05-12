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

#include "config.h"

namespace ufo {

	class mutex_t
	{
		using lock_t = QueueHandle_t;


		
	private:
		lock_t _lock = nullptr;
	public:
		mutex_t() {
			// todo
			// xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
			_lock = xSemaphoreCreateMutex();
			if (!_lock)
			{
				printf("lock == nullptr\n");
			}
		}

		bool lock(TickType_t tick) {
			if (!_lock)
			{
				printf("26lock == nullptr\n");
			}
			
			return xQueueSemaphoreTake(_lock, tick);
		}

		bool unlock(){
			// return xQueueGenericSend(_lock, nullptr, semGIVE_BLOCK_TIME, queueSEND_TO_BACK);
			return xSemaphoreGive(_lock);
		}

        lock_t get(){
            return _lock;
        }

		mutex_t& operator=(mutex_t&& rv) {
			if (this != &rv)
			{
				_lock = rv._lock;
				rv._lock = nullptr;
			}
			return *this;
		}
		
		mutex_t(mutex_t&& rv) : _lock(rv._lock) {
			rv._lock = nullptr;
		}

		const mutex_t& operator=(const mutex_t&) = delete;
		mutex_t(const mutex_t&)=delete;
		
		~mutex_t() {
            if (_lock != nullptr)
            {
                vQueueDelete(_lock);
            }
		}
	};

	class recursive_mutex_t
	{
		using lock_t = QueueHandle_t;
	private:
		lock_t _lock = nullptr;
	public:
		recursive_mutex_t(uint32_t maxCount) {
			_lock = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
		}

		bool lock(TickType_t tick){
			return xQueueTakeMutexRecursive(_lock, tick);
		}

		bool unlock() {
			return xQueueGiveMutexRecursive(_lock);
		}
		
		const recursive_mutex_t& operator=(const recursive_mutex_t&) = delete;
		recursive_mutex_t(const recursive_mutex_t&)=delete;
		
		~recursive_mutex_t() {
            vQueueDelete(_lock);
		}
	};
		
	// RAII
	template <typename _Ty>
	class lock_guard
	{
	public:
		static constexpr uint32_t MutexTickWait = UINT16_MAX;

	private:
		_Ty& _lock;
	public:
		explicit lock_guard(_Ty& lock) : _lock(lock) {
			_lock.lock(MutexTickWait);
		}

		lock_guard(const lock_guard&) = delete;
		lock_guard& operator = (const lock_guard&) = delete;
		
		~lock_guard() {
			_lock.unlock();
		}
	};

} // ufo