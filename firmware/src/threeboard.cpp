#include "src/threeboard.h"

#include "src/logging.h"

#if (defined(AVR) && (!defined(__GNUC__) || __GNUC__ < 9))
static_assert(false, "Unsupported compiler: threeboard requires avr-gcc >=9");
#endif

namespace threeboard {

Threeboard::Threeboard(native::Native *native, usb::Usb *usb,
                       EventBuffer *event_buffer, LedController *led_controller,
                       KeyController *key_controller)
    : native_(native),
      usb_(usb),
      event_buffer_(event_buffer),
      led_controller_(led_controller),
      key_controller_(key_controller),
      layer_controller_(led_controller_->GetLedState(), usb_) {
  native_->SetTimerInterruptHandlerDelegate(this);
  native_->EnableTimer1();
  native_->EnableTimer3();
}

void Threeboard::RunEventLoop() {
  native_->EnableInterrupts();

  // Busy loop until USB setup succeeds.
  WaitForUsbSetup();

  // Busy loop until USB configuration succeeds.
  WaitForUsbConfiguration();

  // Main event loop.
  while (true) {
    RunEventLoopIteration();
  }
}

void Threeboard::HandleTimer1Interrupt() {
  LOG_ONCE("Timer 1 setup complete");
  led_controller_->ScanNextLine();
}

void Threeboard::HandleTimer3Interrupt() {
  LOG_ONCE("Timer 3 setup complete");
  key_controller_->PollKeyState();
  led_controller_->UpdateBlinkStatus();
}

void Threeboard::WaitForUsbSetup() {
  while (!usb_->Setup()) {
    led_controller_->GetLedState()->SetErr(LedState::BLINK_FAST);
    // This is an unrecoverable error. We can either crash here, or delay before
    // retrying USB setup from scratch repeatedly in the hopes that setup
    // eventually succeeds. We choose not to crash.
    native_->DelayMs(50);
  }
  led_controller_->GetLedState()->SetErr(LedState::OFF);
}

void Threeboard::WaitForUsbConfiguration() {
  // Loop until the USB stack configures. If this never happens it will continue
  // to loop infinitely, but also blink the error LED.
  uint16_t iterations = 0;
  while (!usb_->HasConfigured()) {
    iterations += 1;
    if (iterations == UINT16_MAX) {
      LOG_ONCE("Failed to configure USB, continuing to retry");
      led_controller_->GetLedState()->SetErr(LedState::BLINK_FAST);
    }
  }
  led_controller_->GetLedState()->SetErr(LedState::OFF);
}

void Threeboard::RunEventLoopIteration() {
  // Atomically check for new keyboard events, and either handle them or
  // sleep the CPU until the next interrupt.
  native_->DisableInterrupts();
  if (!event_buffer_->HasEvent()) {
    // Sleep the CPU until another interrupt fires.
    native_->EnableCpuSleep();
    native_->EnableInterrupts();
    native_->SleepCpu();
    native_->DisableCpuSleep();
  } else {
    if (event_buffer_->HasKeypressEvent()) {
      layer_controller_.HandleEvent(event_buffer_->GetKeypressEvent());
    }
    // Re-enable interrupts after handling the event.
    native_->EnableInterrupts();
  }
}

}  // namespace threeboard
