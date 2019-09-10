//  Ported from https://github.com/ARMmbed/ATParser//blob/269f14532b98442669c50383782cbce1c67aced5/BufferedSerial/BufferedSerial.cpp
/**
 * @file    BufferedSerial.cpp
 * @brief   Software Buffer - Extends mbed Serial functionallity adding irq driven TX and RX
 * @author  sam grove
 * @version 1.0
 * @see
 *
 * Copyright (c) 2013
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

#include <stdarg.h>
#include <assert.h>
#include <os/os.h>
#include <console/console.h>
#include <hal/hal_uart.h>     //  UART functions.
#include "buffered_serial/buffered_serial.h"

extern "C" int BufferedPrintfC(void *stream, int size, const char* format, va_list arg);
////extern "C" char rx_buf[];
////extern "C" char *rx_ptr;

////  TODO
////char rx_buf[256];        //  Receive buffer.  TODO: Support multiple instances.
////char *rx_ptr = NULL;     //  Pointer to next receive buffer byte to be received.  TODO: Support multiple instances.

static int uart_tx_char(void *arg) {    
    //  UART driver asks for more data to send. Return -1 if no more data is available for TX.
    assert(arg != NULL);
    BufferedSerial *serial = (BufferedSerial *) arg;
    int byte = serial->txIrq();
    return byte;
}

static int uart_rx_char(void *arg, uint8_t byte) {
    //  UART driver reports incoming byte of data. Return -1 if data was dropped.
    ////if (rx_ptr - rx_buf < (int) sizeof(rx_buf)) { *rx_ptr++ = byte; }  //  Save to rx buffer.
    assert(arg != NULL);
    BufferedSerial *serial = (BufferedSerial *) arg;
    int rc = serial->rxIrq(byte);
    return rc;
}

static void uart_tx_done(void *arg) {
    //  UART driver reports that transmission is complete.  Do nothing.
}

int setup_uart(BufferedSerial *serial) {
    //  Configure the UART port HAL settings.
    int rc;
    int uart = serial->_uart;
    uint32_t baud = serial->_baud;

    //  Init rx buffer.
    ////memset(rx_buf, 0, sizeof(rx_buf));
    ////rx_ptr = rx_buf;

    //  Define the UART callbacks.
    rc = hal_uart_init_cbs(uart,
        uart_tx_char, uart_tx_done,
        uart_rx_char, serial);
    if (rc != 0) { return rc; }

    //  Set UART parameters.
    assert(baud != 0);
    rc = hal_uart_config(uart,
        baud,
        8,
        1,
        HAL_UART_PARITY_NONE,
        HAL_UART_FLOW_CTL_NONE
    );
    if (rc != 0) { return rc; }
    return 0;
}

void BufferedSerial::init(char *txbuf, uint32_t txbuf_size, char *rxbuf, uint32_t rxbuf_size, const char* name)
{
    _initialised = 0;
    _uart = 0;
    _baud = 0;
    _txbuf_size = txbuf_size;
    _rxbuf_size = rxbuf_size;
    _txbuf.init(txbuf, txbuf_size);
    _rxbuf.init(rxbuf, rxbuf_size);
    os_error_t rc = os_sem_init(&_rx_sem, 0);  //  Init to 0 tokens, so caller will block until data is available.
    assert(rc == OS_OK);
}

void BufferedSerial::configure(int uart) {
    _uart = uart;
}

int BufferedSerial::readable(void)
{
    return _rxbuf.available();  // note: look if things are in the buffer
}

int BufferedSerial::writeable(void)
{
    return 1;   // buffer allows overwriting by design, always true
}

int BufferedSerial::getc(int timeout)
{
    //  If no data available, wait until the timeout for data.
    os_sem_pend(&_rx_sem, timeout * OS_TICKS_PER_SEC / 1000);
    if (_rxbuf.available()) { return _rxbuf.get(); }
    return -1;
}

int BufferedSerial::putc(int c)
{
    _txbuf.put(c);
    BufferedSerial::prime();
    return c;
}

int BufferedSerial::puts(const char *s)
{
    if (s != NULL) {
        const char* ptr = s;    
        while(*(ptr) != 0) {
            _txbuf = *(ptr++);
        }
        _txbuf = '\n';  // done per puts definition
        BufferedSerial::prime();
        return (ptr - s) + 1;
    }
    return 0;
}

extern "C" size_t BufferedSerialThunk(void *buf_serial, const void *s, size_t length)
{
    BufferedSerial *buffered_serial = (BufferedSerial *)buf_serial;
    return buffered_serial->write(s, length);
}

size_t BufferedSerial::write(const void *s, size_t length)
{
    if (s != NULL && length > 0) {
        const char* ptr = (const char*)s;
        const char* end = ptr + length;    
        while (ptr != end) {
            _txbuf = *(ptr++);
        }
        BufferedSerial::prime();    
        return ptr - (const char*)s;
    }
    return 0;
}

int BufferedSerial::rxIrq(uint8_t byte)
{
    //  UART driver reports incoming byte of data. Return -1 if data was dropped.
    _rxbuf.put(byte);  //  Add to TX buffer.
    os_error_t rc = os_sem_release(&_rx_sem);  //  Signal to semaphore that data is available.
    assert(rc == OS_OK);
    //  Trigger callback if necessary
    if (_cbs[RxIrq]) { _cbs[RxIrq](_cbs_arg[RxIrq]); }
    return 0;
}

int BufferedSerial::txIrq(void)
{
    //  UART driver asks for more data to send. Return -1 if no more data is available for TX.
    if(_txbuf.available()) {
        uint8_t byte = _txbuf.get();  //  Get data from TX buffer.
        return byte;
    }
    //  Trigger callback if no more data to send.
    if (_cbs[TxIrq]) { _cbs[TxIrq](_cbs_arg[TxIrq]); }
    return -1;
}

void BufferedSerial::prime(void)
{
    if (!_initialised) {
        //  Configure the UART port on first use.
        _initialised = 1;
        int rc = setup_uart(this);
        assert(rc == 0);
    }
    ////  TODO: Don't start at every prime.
    hal_uart_start_rx(_uart);  //  Start receiving UART data.
    hal_uart_start_tx(_uart);  //  Start transmitting UART data.
}

void BufferedSerial::attach(void (*func)(void *), void *arg, IrqType type)
{
    _cbs[type] = func;
    _cbs_arg[type] = arg;
}

void BufferedSerial::baud(uint32_t baud0)
{
    _baud = baud0;
}