#ifndef BIT_BRIDGE_APP_HPP
#define BIT_BRIDGE_APP_HPP

#include <wx/wx.h>

class BitBridgeApp : public wxApp {
public:
    BitBridgeApp() = default;

    BitBridgeApp(const BitBridgeApp &) = delete;

    BitBridgeApp(BitBridgeApp &&) = delete;

    BitBridgeApp &operator=(const BitBridgeApp &) = delete;

    BitBridgeApp &operator=(BitBridgeApp &&) = delete;

    ~BitBridgeApp() override = default;

    bool OnInit() override;
};

#endif //BIT_BRIDGE_APP_HPP