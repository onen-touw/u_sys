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
#include "u_sys/thread.h"
#include "u_sys/utils.h" // time

#include "nvs_flash.h"

#include "uffs/ufile.h"

#include "u_cns/u_cns.h"
#include "u_console_lib/u_driver_i2c_console.h"
#include "u_console_lib/u_driver_uart_console.h"
#include "u_console_lib/u_driver_spiffs_console.h"
#include "u_console_lib/u_fs_cat.h"
#include "u_console_lib/u_fs_ls.h"
#include "u_console_lib/u_configurator_console.h"

#include "u_bme_inst/u_bme_tcl.h"
#include "u_bme_inst/u_bme_console.h"
#include "u_bme_inst/u_bme_configurator.h"

#include "u_ads_inst/u_ads_tcl.h"
#include "u_ads_inst/u_ads_console.h"
#include "u_ads_inst/u_ads_configurator.h"

#include "u_ina219_inst/u_ina219_tcl.h"
#include "u_ina219_inst/u_ina219_console.h"
#include "u_ina219_inst/u_ina219_configurator.h"

#include "u_pca9685_inst/u_pca_tcl.h"
#include "u_pca9685_inst/u_pca_console.h"
#include "u_pca9685_inst/u_pca_configurator.h"

#include "u_wdt_inst/u_wdt_tcl.h"
#include "u_wdt_inst/u_wdt_console.h"

// #include "u_esc_inst/u_esc_tcl.h"
// #include "u_esc_inst/u_esc_console.h"

#include <filesystem>

#include "u_configurator/u_configurator.h"


namespace ufo
{
	class sys
	{
	public:
		sys() {}
		sys(sys &&) = default;
		~sys(){}

		void wrapped_task()
		{
			// __u_drivers::__meta_wifi.set_error_callback(clb);
			// using wf_t = __u_drivers::u_driver_wifi_interface_t;
			// wf_t wf(wf_t::mode_t::ap);
			// wf.ip_config("192.168.0.64", "192.168.0.1","255.255.255.0");

			{
				/// call for initialize spiffs driver;
				ufo::u_spiffs_t dev;
			}

			{
				u_configure::u_configurator_t configurator;
				configurator.register_clb(gte::configura_ads1115);
				configurator.register_clb(gte::configura_bme280);
				configurator.register_clb(gte::configura_ina219);
				configurator.register_clb(gte::configura_pca9685);

				auto ack = configurator.configurate();
        		if (ack != u_configure::u_configurator_ack_t::OP_SUCCESS)
				{
					ESP_LOGE("ucfg", "error, %u", static_cast<uint16_t>(ack));
				}
				else
				{
					ESP_LOGI("ucfg", "Success");
				}

				ufo::utl::sleep_for(1000);
			}

			using uart_t = __u_drivers::u_driver_uart_interface_t;
			uart_t uart0(uart_t::uart_port_t::uart0);
			
			ufo::u_console_uart_stream_t uart_io(&uart0);
			using cns_t = ufo::u_console_t;
			
			cns_t cns(&uart_io, 255);  
			
			cns.mk_blank("iic", "driver info and utilities", ufo::i2c_driver_console);
			cns.mk_blank("uart", "driver info", ufo::uart_driver_console);
			cns.mk_blank("fsdrv", "spiffs driver info", ufo::spiffs_driver_console);
			cns.mk_blank("cat", "", ufo::u_fs_cat);
			cns.mk_blank("ls", "", ufo::u_fs_cat);

			cns.mk_blank("bme", "bme280", gte::bme280_console);
			cns.mk_blank("ina", "ina219", gte::ina219_console);
			cns.mk_blank("ads", "ads1115", gte::ads1115_console);
			cns.mk_blank("pca", "pca9685", gte::pca9685_console);
			// cns.mk_blank("esc", "esc control", gte::esc_console);
			cns.mk_blank("wdt", "wdt control", gte::wdt_console);

			cns.mk_blank("cfg", "", ufo::configurator_console);

			ufo::thread_cfg bme_cfg = {"bme", 4096, 5, 0};
			ufo::thread bme_task(bme_cfg, gte::u_bme_task);

			ufo::thread_cfg ina_cfg = {"ina", 4096, 5, 0};
			ufo::thread ina_task(ina_cfg, gte::u_ina219_task);

			ufo::thread_cfg ads_cfg = {"ads", 4096, 5, 0};
			ufo::thread ads_task(ads_cfg, gte::u_ads_task);

			ufo::thread_cfg pca_cfg = {"pca", 4096, 5, 0};
			ufo::thread pca_task(pca_cfg, gte::u_pca_task);

			ufo::thread_cfg wdt_cfg = {"wdt", 4096, 5, 0};
			ufo::thread wdt_task(wdt_cfg, gte::gte_wdt_task);

			// ufo::thread_cfg esc_cfg = {"esc", 4096, 5, 0};
			// ufo::thread esc_task(esc_cfg, gte::u_esc_task);

			ufo::thread_cfg cns_cfg = {"cns", 4096, 5, 0};
			ufo::thread console_task(cns_cfg, &cns_t::ctask, std::move(cns));

			bme_task.join();
			ina_task.join();
			ads_task.join();
			pca_task.join();
			wdt_task.join();
			
			console_task.join(); 
		}

	private:

		static void clb(const __u_drivers::driver_meta_t& meta)
		{
			printf("Wifi Callback\n state: %u\nu_cnt: %u\ne_cnt: %u\nlast e_code: %u\n", 
				meta.state.get(), meta.user_count, meta.error_count, meta.error_code);
		}

	};
} // ufo