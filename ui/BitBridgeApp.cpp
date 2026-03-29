#include "BitBridgeApp.hpp"
#include "MainFrame.hpp"
#include "AppSettings.hpp"

bool BitBridgeApp::OnInit() {
    auto settings = std::make_unique<AppSettings>();
    settings->LoadFromFile("bitbridge-settings.toml");

    auto *frame = new MainFrame("Bit Bridge - Load Balancer Configuration",
                                wxPoint(50, 50),
                                wxSize(580, 420),
                                std::move(settings));
    frame->Show(true);
    return true;
}