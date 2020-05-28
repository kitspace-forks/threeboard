#include "bootstrap.h"
#include "src/native/i2c.h"
#include "src/native/native_impl.h"
#include "src/threeboard.h"
#include "src/usb/usb_impl.h"

using namespace threeboard;

void RunThreeboard() {
  // The native interface is used to abstract away all "native" code (e.g.
  // interrupt setup code, setting various pin port values). This is the only
  // place that NativeImpl is injected. To keep all other components testable,
  // they all use the Native interface.
  auto native_impl = native::NativeImpl::Get();
  native_impl->EnableInterrupts();

  // Similar to how we construct native_impl above, this is the only place where
  // UsbImpl is injected, so as to enable other components to be testable with a
  // mocked USB implementation.
  auto usb_impl = usb::UsbImpl(native_impl);

  // Set up the remaining objects to inject into the Threeboard instance. These
  // could be constructed within the instance, but injecting them makes testing
  // easier since we can inject mocks for tets.
  EventHandler event_handler(native_impl);
  LedController led_controller(native_impl);
  KeyController key_controller(native_impl, &event_handler);

  // The threeboard obejct composes in all of these controllers, handlers and
  // implementation objects, and synchronises them in an infinite runloop.
  Threeboard threeboard(native_impl, &usb_impl, &event_handler, &led_controller,
                        &key_controller);

  // Run the firmware event loop. This will run forever.
  threeboard.Run();
}