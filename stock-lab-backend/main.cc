#include <drogon/drogon.h>

#include "services/SchedulerService.h"

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0", 5555);
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    //Run HTTP framework,the method will block in the internal event loop
    auto scheduler = std::make_shared<service::SchedulerService>();
    drogon::app().getLoop()->runEvery(60.0, [scheduler]
    {
        scheduler->scan();
    });
    drogon::app().run();
    return 0;
}
