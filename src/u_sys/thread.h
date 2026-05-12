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
#include "invoke.h"
#include "utils.h"
#include "string.h" 
#include "tctb.h"

#include <atomic>

#include "esp_log.h"

namespace ufo
{
    class thread;

    namespace detail
    {

        struct thread_impl_base
        {
            virtual ~thread_impl_base() = default;
            virtual void Run() = 0;
        };
        using thread_ptr = std::unique_ptr<thread_impl_base>;

        // it is unsave but only parent can set value
        enum class state_t : uint8_t
        {
            // RUN -> TERMINATING -> IDLE
            RUN,
            TERMINATE,
            IDLE,    
            JOINED,   
        };

        static std::atomic<uint32_t> u_thread_cnt = 0;

        inline void RTOS_task(void *arg);

        struct thread_context
        {
            TaskHandle_t _parent = nullptr; // no delete    if it is nullptr => task was detached
            TaskHandle_t _self = nullptr;
            std::atomic<TaskHandle_t> _joiner{nullptr};
            
            
            uint32_t _id = 0;
            state_t _state = state_t::IDLE;
        };

        struct thread_task_bridge {
            void* _other = nullptr;     
            std::shared_ptr<thread_context> _context;   // no delete (shared_ptr)
            thread_ptr _ptr;                            // no delete (unique_ptr)
        };

    } // namespace detail

    struct thread_cfg
    {
        const char* _name = nullptr;
        uint32_t _stackSize = 0;
        uint8_t _prio = 0;
        uint8_t _core = 0;
    };

    namespace notify
    {
        constexpr uint32_t START = (1 << 0);  // родитель -> ребёнок: запускайся
        constexpr uint32_t DONE = (1 << 1);   // ребёнок -> родитель: я завершился
        constexpr uint32_t CANCEL = (1 << 2); // родитель -> ребёнок: отмена (ещё до старта)
    }

    class thread
    {
    public:
        using Handle_t = TaskHandle_t;
        using state_t = detail::state_t;
        using Context_t = detail::thread_context;
        
        static constexpr const char* UFO_THREAD_TAG = "U_TH";
    private:
        std::shared_ptr<Context_t> _context;

        Handle_t _handle = nullptr;     // no delete

    private:
        // impl = wrapper about { func, token, args... }
        template <typename funk, typename... _Args>
        struct _Imle : detail::thread_impl_base
        {
            funk f;
            std::tuple<typename std::decay<_Args>::type...> tup;

            _Imle(funk &&f, _Args &&...args)
                : f(std::forward<funk>(f)), tup(std::forward<_Args>(args)...)
            {}

            void Run() { 
                //"Run()-->Call()\n";    
                Call(); 
            }

            template <std::size_t TupSize = std::tuple_size_v<decltype(tup)>>
            void Call()
            {
                _invoke(std::make_index_sequence<TupSize>{});
            }

            template <size_t... ind>
            void _invoke(std::index_sequence<ind...>)
            {
                invoke(f, std::get<ind>(std::move(tup))...);
            }
        };

        class id
        {
        private:
            intptr_t _id = 0;
        public:
            id(TaskHandle_t hand) : _id( reinterpret_cast<intptr_t>(hand)) {}
            uint32_t get() const {
                return static_cast<uint32_t>(_id);
            }
        };
    public:
        thread() {}

        template <typename Call, typename... Args>
        thread(Call && f, Args && ...args)
        {
            thread_cfg cfg;
            cfg._core = 0; 
            cfg._prio = 5; 
            cfg._stackSize = 2048; 
            create(cfg ,std::forward<Call>(f), std::forward<Args>(args)...);
        }

        template <typename Call, typename... Args>
        thread(thread_cfg& cfg, Call && f, Args && ...args)
        {
            create(cfg, std::forward<Call>(f), std::forward<Args>(args)...);
        }

        thread(const thread&) = delete;
        thread(thread&& other) : _context(std::move(other._context)), _handle(other._handle) 
        {
            other._handle = nullptr;
        }

        thread& operator= (const thread&) = delete;
        thread& operator= (thread&& other) {
            if (&other != this)
            {
                if (joinable())
                {
                    ESP_LOGE(UFO_THREAD_TAG, "[assignment] joinable");
                    throw;
                }
                
                _context = std::move(other._context);
                _handle = other._handle;
                other._handle = nullptr;
            }
            return *this;
        }

        ~thread()
        {
            ESP_LOGI(UFO_THREAD_TAG, "[~] call");

            if (joinable())
            {
                // terminate();
                ESP_LOGE(UFO_THREAD_TAG, "[~] destruction of joinable");
            }
            ESP_LOGI(UFO_THREAD_TAG, "[~] finish");
        }

        bool joinable() const noexcept {
            if (!_context) return false;
            // Поток joinable если:
            // 1. Есть родитель (не detached)
            // 2. Никто не ждёт join
            // 3. Задача ещё существует
            return (_context->_parent != nullptr && 
                    _context->_joiner.load(std::memory_order_acquire) == nullptr);
        }

    private:

        template <typename Call, typename... Args>
        void create(thread_cfg& cfg, Call&& f, Args&&... args) {
            ESP_LOGI(UFO_THREAD_TAG, "[create] Call");
            
            if (_context) {
                // if (joinable()) detach();
                // _context.reset();
                ESP_LOGE(UFO_THREAD_TAG, "[create] Cant be reached");
                throw;
            }
            
            _context = std::make_shared<detail::thread_context>();
            _context->_parent = xTaskGetCurrentTaskHandle();
            
            ++detail::u_thread_cnt;
            
            ESP_LOGI(UFO_THREAD_TAG, "[create] Bridge");

            // Создаём мост
            auto* bridge = new detail::thread_task_bridge();
            if (!bridge)
            {
                ESP_LOGE(UFO_THREAD_TAG, "[create] !Bridge");
                return;
            }
            bridge->_ptr = std::make_unique<_Imle<Call, Args...>>(std::forward<Call>(f), std::forward<Args>(args)...);
            bridge->_context = _context;
            
            ESP_LOGI(UFO_THREAD_TAG, "[create] naming");
            // Формируем имя задачи
            char task_name[configMAX_TASK_NAME_LEN] = {};
            if (cfg._name == nullptr) {
                snprintf(task_name, sizeof(task_name), "th%lu", 
                        static_cast<unsigned long>(detail::u_thread_cnt.load()));
            } else {
                snprintf(task_name, sizeof(task_name), "%.*s", 
                        static_cast<int>(sizeof(task_name) - 1), cfg._name);
            }
            
            ESP_LOGI(UFO_THREAD_TAG, "[create] native creation");

            // Создаём задачу FreeRTOS
            BaseType_t result = xTaskCreatePinnedToCore(
                detail::RTOS_task,
                task_name,
                cfg._stackSize > 0 ? cfg._stackSize : configMINIMAL_STACK_SIZE * 2,
                bridge,
                cfg._prio > 0 ? cfg._prio : tskIDLE_PRIORITY + 1,
                &_handle,
                cfg._core
            );
            
            if (result != pdPASS) {
                delete bridge;
                _context.reset();
                ESP_LOGE(UFO_THREAD_TAG, "[create] !native thread");
            }
            
            _context->_self = _handle;
            _context->_id = reinterpret_cast<uint32_t>(_handle);
            
            detail::u_privite_tskTaskControlBlock* cb = reinterpret_cast<detail::u_privite_tskTaskControlBlock*>(_handle);
            ESP_LOGI(UFO_THREAD_TAG, "[create] Info:\n\tid: %lu\n\tname: %s\n\tprio: %u", _context->_id, cb->pcTaskName, cb->uxPriority);

            // Запускаем задачу — отправляем START уведомление
            xTaskNotify(_handle, notify::START, eSetBits);
        }

        // Проверка, завершился ли поток (без блокировки)
        bool is_finished() const noexcept {
            if (!_context || !_handle) return false;
            
            // Быстрая проверка через уведомления
            uint32_t notification;
            if (xTaskNotifyWait(0, notify::DONE, &notification, 0) == pdTRUE) {
                return true;
            }
            
            // Запасной вариант через FreeRTOS API
            eTaskState state = eTaskGetState(_handle);
            return (state == eDeleted);
        }

        // Финализация join (внутренний метод)
        void _finalize_join() {
            if (_context) {
                _context->_parent = nullptr;
                _context->_joiner.store(nullptr, std::memory_order_release);
                --detail::u_thread_cnt;
            }
        }
        
    public:

        void detach() {
            if (!_context) {
                ESP_LOGE(UFO_THREAD_TAG, "[detach] !no such proc");
                // printf("detach:: !context\n");
            }
            
            if (_context->_parent == nullptr) {
                ESP_LOGE(UFO_THREAD_TAG, "[detach] already detached");
            }
            
            // Проверяем, не ждёт ли уже кто-то join
            if (_context->_joiner.load(std::memory_order_acquire) != nullptr) {
                // throw thread_error(thread_errc::already_joined);
                ESP_LOGE(UFO_THREAD_TAG, "[detach] joined");
            }
            
            // Отсоединяем
            _context->_parent = nullptr;
            --detail::u_thread_cnt;
        }
    

        void join() {
            // printf("[join] Enter, _context=%p\n", _context.get());
            
            ESP_LOGI(UFO_THREAD_TAG, "[join] call");

            if (!_context) {
                ESP_LOGE(UFO_THREAD_TAG, "[join] !no such proc");
            }
            
            if (_context->_parent == nullptr) {
                ESP_LOGE(UFO_THREAD_TAG, "[join] !parent (detached)");
            }
            
            if (_context->_self == xTaskGetCurrentTaskHandle()) {
                ESP_LOGE(UFO_THREAD_TAG, "[join] self join");
            }
            
            // Если поток уже завершился, просто финализируем
            if (is_finished()) {
                ESP_LOGI(UFO_THREAD_TAG, "[join] finalizing");
                _finalize_join();
                return;
            }

            TaskHandle_t current = xTaskGetCurrentTaskHandle();
            TaskHandle_t expected = nullptr;
            
            // printf("[join] Attempting to set _joiner from %p to %p\n", expected, current);
            
            if (!_context->_joiner.compare_exchange_strong(expected, current,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                // printf("[join] compare_exchange failed: expected=%p, current=%p\n", expected, current);
                ESP_LOGE(UFO_THREAD_TAG, "[join] already joined or waiting\n");
            }
            
            ESP_LOGI(UFO_THREAD_TAG, "[join] waiting thread");
            
            uint32_t notification;
            xTaskNotifyWait(0, notify::DONE, &notification, portMAX_DELAY);
            ESP_LOGI(UFO_THREAD_TAG, "[join] catch notif");
            
            _finalize_join();
            ESP_LOGI(UFO_THREAD_TAG, "[join] done");
        }

        uint32_t get_id() const {
            if (!_context)
            {
                return 0;
            }
            return _context->_id;
        }

    };

    class thread_guard
    {
    private:
        thread _t;
    public:
        thread_guard(thread&& t) : _t(std::forward<thread>(t)) {}
        // thread_guard(){}
        thread_guard(thread_guard&) = delete;
        thread_guard& operator=(thread_guard&) = delete;

        thread_guard(thread_guard&& other) : _t(std::move(other._t))
        {}
        thread_guard& operator=(thread_guard&& other) {
            if (&other != this)
            {
                _t = std::move(other._t);
            }
            return *this;
        }


        thread* operator->() { 
            return &_t;
        }

        ~thread_guard() {
            if (_t.joinable())
            {
                _t.join();
            }
        }
    };

    // todo move it into .cpp
    namespace detail
    {
        inline void RTOS_task(void *arg)
        {
            utl::sleep_for(5);
            thread_task_bridge* bridge = reinterpret_cast<thread_task_bridge*>(arg);
            
            ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] call");

            // printf("[RTOS_task] Started, bridge=%p\n", bridge);
            // printf("[RTOS_task] _parent=%p, _self=%p\n", 
            //     bridge->_context->_parent, 
            //     bridge->_context->_self);

            uint32_t notification;
            xTaskNotifyWait(0, 0xFFFFFFFF, &notification, portMAX_DELAY);

            ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] catch notif");
            
            if (notification & notify::START) {
                ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] run user");
                bridge->_ptr->Run();
                ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] finish user");
            }

            TaskHandle_t target = bridge->_context->_joiner.load(std::memory_order_acquire);

            ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] ");
            
            if (target == nullptr) {
                target = bridge->_context->_parent;
                ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] notif parent");
            }

            if (target != nullptr) {
                ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] notif joiner");
                xTaskNotify(target, notify::DONE, eSetBits);
            } else {
                ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] no notif targer");
            }
            
            ESP_LOGI(thread::UFO_THREAD_TAG, "[RTOSth] del bridge and task");
            delete bridge;
            vTaskDelete(nullptr);
        }


        
    } // namespace detail

} // namespace ufo
