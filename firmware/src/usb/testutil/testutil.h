#pragma once

#include "src/native/native_mock.h"
#include "src/usb/shared/protocol.h"

namespace threeboard {
namespace usb {
namespace testutil {

void MockSendingSetupPacket(native::NativeMock *, const SetupPacket &);
}
}  // namespace usb
}  // namespace threeboard
