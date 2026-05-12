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

#include "u_sys/config.h"
#include "driver/gpio.h"

namespace ufo
{
    namespace utl
    {
        esp_err_t gpio_config(gpio_num_t pin, gpio_mode_t mode){

            // no need to reset
            gpio_config_t conf = {
                .pin_bit_mask = 1ULL << pin,
                .mode = mode,           //GPIO_MODE_INPUT | GPIO_MODE_OUTPUT
                .pull_up_en = GPIO_PULLUP_ENABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            esp_err_t e = gpio_config(&conf);
            return e;
        }

        esp_err_t gpio_config(gpio_num_t pin, gpio_mode_t mode, bool pup, bool pdw){
            // no need to reset
            gpio_config_t conf = {
                .pin_bit_mask = 1ULL << pin,
                .mode = mode, // GPIO_MODE_INPUT | GPIO_MODE_OUTPUT
                .pull_up_en = pup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
                .pull_down_en = pdw ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE};
            esp_err_t e = gpio_config(&conf);
            return e;
        }

    } // namespace utl
    

} // namespace ufo
