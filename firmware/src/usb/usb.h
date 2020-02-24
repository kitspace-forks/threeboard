#pragma once

#include <stdint.h>

namespace threeboard {
namespace usb {

class Usb {
public:
  virtual ~Usb() {}

  // Setup the USB hardware to begin connection to the host. This method blocks
  // until the device hardware is correctly setup. Returning from Setup does not
  // guarantee that the device has enumerated.
  // TODO: return error handling status if necessary.
  virtual void Setup() = 0;
  virtual bool HasConfigured() = 0;

  virtual void SendKeypress(const uint8_t key, const uint8_t mod) = 0;
};
} // namespace usb
} // namespace threeboard
