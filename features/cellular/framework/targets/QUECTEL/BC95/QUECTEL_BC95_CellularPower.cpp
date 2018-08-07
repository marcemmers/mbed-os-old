/*
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "QUECTEL_BC95_CellularPower.h"

using namespace mbed;

QUECTEL_BC95_CellularPower::QUECTEL_BC95_CellularPower(ATHandler &atHandler) : AT_CellularPower(atHandler)
{

}

QUECTEL_BC95_CellularPower::~QUECTEL_BC95_CellularPower()
{

}

nsapi_error_t QUECTEL_BC95_CellularPower::set_at_mode()
{
    _at.lock();
    _at.flush();
    _at.cmd_start("AT+CMEE="); // verbose responses
    _at.write_int(1);
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();
    return _at.unlock_return_error();
}

nsapi_error_t QUECTEL_BC95_CellularPower::reset()
{
    _at.lock();
    _at.cmd_start("AT+NRB"); // reset to full power levels
    _at.cmd_stop();
    _at.resp_start("REBOOTING", true);
    return _at.unlock_return_error();
}

nsapi_error_t QUECTEL_BC95_CellularPower::set_power_level(int func_level, int do_reset)
{
    _at.lock();
    _at.cmd_start("AT+CFUN=");
    _at.write_int(func_level);
    _at.write_int(do_reset);
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();

    _at.cmd_start("AT+CFUN?");
    _at.cmd_stop();
    _at.resp_start("+CFUN:");
    int level = _at.read_int();
    _at.resp_stop();

    if( level != 1 )
    {
        _at.unlock();
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return _at.unlock_return_error();
}

nsapi_error_t QUECTEL_BC95_CellularPower::on()
{
    return set_power_level(1);
}

nsapi_error_t QUECTEL_BC95_CellularPower::is_device_ready()
{
    _at.lock();
    _at.cmd_start("AT+CFUN?");
    _at.cmd_stop();
    _at.resp_start("+CFUN:");
    int level = _at.read_int();
    _at.resp_stop();

    if( level != 1 )
    {
        _at.unlock();
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    if(_at.get_last_error() != NSAPI_ERROR_OK)
    {
        return _at.unlock_return_error();
    }

    _at.lock();
    _at.cmd_start("AT");
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();

    // we need to do this twice because for example after data mode the first 'AT' command will give modem a
    // stimulus that we are back to command mode.
    _at.clear_error();
    _at.cmd_start("AT");
    _at.cmd_stop();
    _at.resp_start();
    _at.resp_stop();

    return _at.unlock_return_error();
}