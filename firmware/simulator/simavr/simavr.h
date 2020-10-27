#pragma once

#include <functional>
#include <stdint.h>

#include "simulator/lifetime.h"
#include "simulator/simavr/internal/iom32u4.h"

// TODO: Remove asap!
extern "C" {
struct avr_t;
}

namespace threeboard {
namespace simulator {

enum State {
  LIMBO,
  STOPPED,
  RUNNING,
  SLEEPING,
  STEP,
  STEP_DONE,
  DONE,
  CRASHED,
};

enum Flag {
  UART_FLAG_STDIO = 2,
};

// Define all necessary IOCTL indices here so we don't have to depend on any
// simavr headers. The strange looking constants are an artifact of the very
// weird way that simavr identifies IOCTL indices.
enum Ioctl {
  // The USB ioctls are defined in avr_usb.h.
  USB_WRITE = 1970496119,
  USB_READ = 1970496114,
  USB_SETUP = 1970496115,
  USB_RESET = 1970496082,
  USB_VBUS = 1970496086,

  // The UART ioctls are defined in avr_uart.c.
  UART_GET_FLAGS = 1969317681,
  UART_SET_FLAGS = 1969320753,
};

// A copy of the avr_io_usb, defined in avr_usb.h.
struct UsbPacketBuffer {
  uint8_t endpoint;
  uint32_t size;
  uint8_t *buffer;
};

using UsbAttachCallback = std::function<void(uint32_t)>;
using UartOutputCallback = std::function<void(uint8_t)>;

// A shim to collect the simavr API into one single interface to make all
// classes that interact with simavr testable.
class Simavr {
public:
  virtual ~Simavr() = default;

  virtual void Run() = 0;

  virtual void InitGdb() = 0;
  virtual void DeinitGdb() = 0;
  virtual void Reset() = 0;
  virtual void Terminate() = 0;

  virtual int InvokeIoctl(uint32_t ioctl, void *param) = 0;

  virtual std::unique_ptr<Lifetime>
  RegisterUsbAttachCallback(UsbAttachCallback *callback) = 0;
  virtual std::unique_ptr<Lifetime>
  RegisterUartOutputCallback(UartOutputCallback *callback) = 0;

  virtual void SetData(uint8_t idx, uint8_t val) = 0;
  virtual void SetState(uint8_t val) = 0;
  virtual void SetGdbPort(uint8_t val) = 0;

  virtual uint8_t GetData(uint8_t idx) const = 0;
  virtual uint8_t GetState() const = 0;
  virtual uint8_t GetGdbPort() const = 0;
  virtual uint64_t GetCycle() const = 0;
  virtual uint16_t GetStackPointer() const = 0;
  virtual uint16_t GetBssSectionSize() const = 0;
  virtual uint16_t GetDataSectionSize() const = 0;
  virtual uint16_t GetRamSize() const = 0;

  // TODO: Remove asap!
  virtual avr_t *GetAvr() = 0;
};
} // namespace simulator
} // namespace threeboard
