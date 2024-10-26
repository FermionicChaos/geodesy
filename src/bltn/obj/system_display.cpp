#include <geodesy/bltn/obj/system_display.h>

#include <memory>

#include <GLFW/glfw3.h>

namespace geodesy::bltn::obj {

    void system_display::get_system_displays(engine* aEngine) {
        int MonitorCount = 0;
        GLFWmonitor** MonitorList = glfwGetMonitors(&MonitorCount);
        std::vector<std::shared_ptr<system_display>> SystemDisplayList(MonitorCount);
        for (int i = 0; i < MonitorCount; i++) {
            SystemDisplayList[i] = std::make_shared<system_display>(MonitorList[i]);
        }
        aEngine->Display = SystemDisplayList;
        for (std::shared_ptr<system_display> Display : SystemDisplayList) {
            if (glfwGetPrimaryMonitor() == Display->Monitor) {
                aEngine->PrimaryDisplay = Display;
            }
        }
    }

    system_display::system_display(GLFWmonitor* aMonitor) {
        this->Monitor = aMonitor;
    }

}