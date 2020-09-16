#include "simulator.h"

#include <iostream>

namespace threeboard {
namespace simulator {
namespace {

using namespace std::placeholders;

// TODO: this can be made configurable later if needed.
constexpr uint16_t kGdbPort = 1234;

// Used to test a single pin in a register.
bool IsEnabled(uint8_t reg, uint8_t pin) { return reg & (1 << pin); }

} // namespace

Simulator::Simulator()
    : gdb_enabled_(false), firmware_(std::make_unique<Firmware>()) {}

void Simulator::Run() {
  if (ui_ != nullptr) {
    std::cout << "Attempted to run a running simulator instance!" << std::endl;
    exit(0);
  }
  firmware_->Reset();
  firmware_->RunAsync();
  ui_ = std::make_unique<UI>(this, firmware_->GetState(),
                             firmware_->GetCycleCount(), gdb_enabled_);
  usb_host_ = std::make_unique<Host>(firmware_->GetAvr(), this);
  ui_->StartRenderLoopAsync();
  std::unique_lock<std::mutex> lock(mutex_);
  sim_run_var_.wait(lock);
  ui_ = nullptr;
}

void Simulator::PrepareRenderState() {
  uint8_t portb = firmware_->GetPortB();
  uint8_t portc = firmware_->GetPortC();
  uint8_t portd = firmware_->GetPortD();
  uint8_t portf = firmware_->GetPortF();

  // Clear the LED state before the next frame is calculated.
  ui_->ClearLedState();

  // The ERR/STATUS LED pair for threeboard v1 is wired as follows:
  // status0: PB6, ERR
  // status1: PC6, STATUS
  if (IsEnabled(portb, 6) && !IsEnabled(portc, 6)) {
    ui_->SetErr(true);
  } else if (IsEnabled(portc, 6) && !IsEnabled(portb, 6)) {
    ui_->SetStatus(true);
  }

  // The wiring of the LED matrix for threeboard v1 is detailed below. Scanning
  // is row-major, meaning only one row is active at a time. The column pins
  // can be effectively considered as active low.
  // row0: PD7, B0_{4,5,6,7}
  // row1: PB4, B0_{0,1,2,3}
  // row2: PD6, B1_{4,5,6,7}
  // row3: PD4, B1_{0,1,2,3}
  // row4: PB5, (R, G, B, PROG)
  // col0: PF0, (R, B0_3, B0_7, B1_3, B1_7)
  // col1: PF1, (G, B0_2, B0_6, B1_2, B1_6)
  // col2: PF4, (B, B0_1, B0_5, B1_1, B1_5)
  // col3: PF5, (PROG, B0_0, B0_4, B1_0, B1_4)
  if (IsEnabled(portd, 7)) {
    ui_->SetBank0(!IsEnabled(portf, 0), 7);
    ui_->SetBank0(!IsEnabled(portf, 1), 6);
    ui_->SetBank0(!IsEnabled(portf, 4), 5);
    ui_->SetBank0(!IsEnabled(portf, 5), 4);
  } else if (IsEnabled(portb, 4)) {
    ui_->SetBank0(!IsEnabled(portf, 0), 3);
    ui_->SetBank0(!IsEnabled(portf, 1), 2);
    ui_->SetBank0(!IsEnabled(portf, 4), 1);
    ui_->SetBank0(!IsEnabled(portf, 5), 0);
  } else if (IsEnabled(portd, 6)) {
    ui_->SetBank1(!IsEnabled(portf, 0), 7);
    ui_->SetBank1(!IsEnabled(portf, 1), 6);
    ui_->SetBank1(!IsEnabled(portf, 4), 5);
    ui_->SetBank1(!IsEnabled(portf, 5), 4);
  } else if (IsEnabled(portd, 4)) {
    ui_->SetBank1(!IsEnabled(portf, 0), 3);
    ui_->SetBank1(!IsEnabled(portf, 1), 2);
    ui_->SetBank1(!IsEnabled(portf, 4), 1);
    ui_->SetBank1(!IsEnabled(portf, 5), 0);
  } else if (IsEnabled(portb, 5)) {
    ui_->SetR(!IsEnabled(portf, 0));
    ui_->SetG(!IsEnabled(portf, 1));
    ui_->SetB(!IsEnabled(portf, 4));
    ui_->SetProg(!IsEnabled(portf, 5));
  }
}

void Simulator::HandlePhysicalKeypress(char key, bool state) {
  // Simulator command keys.
  if (key == 'q') {
    sim_run_var_.notify_all();
  } else if (key == 'g') {
    if (gdb_enabled_) {
      firmware_->DisableGdb();
    } else {
      firmware_->EnableGdb(GetGdbPort());
    }
    gdb_enabled_ = !gdb_enabled_;
  }

  // The key pins are all active low.
  else if (key == 'a') {
    // Switch 1 - maps to PB2.
    firmware_->SetPinB(2, !state);
  } else if (key == 's') {
    // Switch 2 - maps to PB3.
    firmware_->SetPinB(3, !state);
  } else if (key == 'd') {
    // Switch 3 - maps to PB1.
    firmware_->SetPinB(1, !state);
  }
}

void Simulator::HandleVirtualKeypress(uint8_t mod_code, uint8_t key_code) {
  std::cout << "Received virtual keypress: mod=" << (int)mod_code
            << ", key=" << (int)key_code << std::endl;
}

uint16_t Simulator::GetGdbPort() { return kGdbPort; }

} // namespace simulator
} // namespace threeboard
