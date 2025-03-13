#include <cairomm/surface.h>
#include <iostream>
#include "core/Daw.h"
#include "ui/MainUI.h"
#include "hardware/push2/Push2Gui.h"
#include <string>

using namespace tstudio;
using namespace Cairo;

static void run_ui (Push2Gui &push2Gui){
  std::cout << "Running UI\n";
  push2Gui.run();
}

int main(int, char **)
{
  auto daw = Daw();
  // mainUI(daw.session, daw.playHead);

  auto push2Gui = Push2Gui(daw);
  push2Gui.set_frames_per_second(60);
  std::thread guiThread(run_ui, std::ref(push2Gui));

  std::cin.get();
  std::cout << "Exiting...\n";
  push2Gui.running = false;
  guiThread.join();
  return 0;
}
