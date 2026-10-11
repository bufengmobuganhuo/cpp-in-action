#include <drogon/drogon.h>

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

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k204NoContent);

        const auto origin = req->getHeader("Origin");
        resp->addHeader("Access-Control-Allow-Origin", origin.empty() ? "*" : origin);
        resp->addHeader("Access-Control-Allow-Credentials", "true");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type, Accept, Origin, X-Requested-With");
        resp->addHeader("Access-Control-Max-Age", "86400");
        resp->addHeader("Vary", "Origin");
        callback(resp);
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
