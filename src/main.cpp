#include <iostream>
#include "core/Daw.h"
#include "ui/MainUI.h"
#include "libusb.h"
#include "cairomm/cairomm.h"

using namespace tstudio;

int main(int, char **)
{
  auto daw = Daw();
  mainUI(daw.session, daw.playHead);
  std::cout << "Exiting...\n";
}
