#include <drogon/drogon.h>

#include <set>
#include <string_view>

#include "services/SchedulerService.h"

int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0", 8088);
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    //Handle browser CORS preflight requests before Drogon checks route methods.
    drogon::app().registerPreRoutingAdvice([](const drogon::HttpRequestPtr& req,
                                              drogon::AdviceCallback&& callback,
                                              drogon::AdviceChainCallback&& pass)
    {
        if (req->method() != drogon::Options)
        {
            pass();
            return;
        }

        static const std::set<std::string_view> allowed_headers{
            "authorization",
            "content-type",
            "accept",
            "origin",
            "x-requested-with"
        };
        auto allow_all_origins = [](std::string_view)
        {
            return true;
        };
        callback(drogon::HttpResponse::newOptionsResponse(
            req,
            allow_all_origins,
            true,
            true,
            true,
            86400,
            allowed_headers));
    });
    drogon::app().registerPreSendingAdvice([](const drogon::HttpRequestPtr& req,
                                              const drogon::HttpResponsePtr& resp)
    {
        resp->addCorsHeaders(req, {}, true);
    });
    //Run HTTP framework,the method will block in the internal event loop
    auto scheduler = std::make_shared<service::SchedulerService>();
    drogon::app().getLoop()->runEvery(60.0, [scheduler]
    {
        scheduler->scan();
    });
    drogon::app().run();
    return 0;
}
